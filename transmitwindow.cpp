#include "mainwindow.h"
#include "transmitwindow.h"
// #include "morse_table.h"

QMutex buffMutex;

CBuffer::CBuffer() {
    head = 0;
    tail = 0;
    size = CBUFF_SIZE;
    // For debug only
    // for (int i=0; i< (int) sizeof(cbuff); i++) cbuff[i] = '\0';
    memset(cbuff, 0, size);
}

bool CBuffer::put(char c) {

    // Check for buffer full
    if ( isFull() ) {
        // Buffer full
        return false;
    }

    // We write to "tail" and read from "head"
    cbuff[tail] = c;
    tail = (tail + 1) % size;
    return true;
}

char CBuffer::get() {

    if ( isEmpty() ) {
        // Buffer empty
        return -1;
    }

    // We write to "tail" and read from "head"
    char c = cbuff[head];
    head = (head + 1) % size;
    return c;
}

void CBuffer::clear() {
    head = 0;
    tail = 0;
}

bool CBuffer::isEmpty() {
    return (head == tail);
}

bool CBuffer::isFull() {
    return ((tail + 1) % CBUFF_SIZE == head);
}

int CBuffer::getNumCharsQueued() {

    int num;
    //  Excel: if(tail<head,((tail%size))-head) else (tail%size)-head
    num = (tail < head) ? (tail % CBUFF_SIZE) - head + CBUFF_SIZE : (tail % CBUFF_SIZE) - head;
    return num;
}

bool CBuffer::deleteLast() {
    if ( isEmpty() )
        return false;

    tail = (tail - 1) % CBUFF_SIZE;
    qDebug() << "CBuffer::deleteLast(): head:" << head << "Head char" << cbuff[head] << "Tail index" << tail << "tail char" << cbuff[tail-1];
    return true;
}

TransmitWindow::TransmitWindow(QWidget *parent)
                : QTextEdit(parent)
{

    // pSerial = p;    // Store serial port object pointer
    setObjectName("TransmitWindow");
    setGeometry(QRect(125, 180, 625, 100));
    show();
    setPlaceholderText("Transmit here");

    // Create a cursor object and connect it to our window
    cursor = textCursor();
    setTextCursor(cursor);
    qDebug() << "TransmitWindow::TransmitWindow(): cursor now at posotion:" << cursor.position();

    // Create a format object for "normal" text and capture it's properties
    normal_f.setFontStrikeOut(false);
    normal_f.setForeground(QBrush(Qt::blue));
    normal_f.setFontPointSize(16);
    setCurrentCharFormat(normal_f);

    // Create a Format Object to highlight characters that have been transmitted already
    strikethrough_f.setFontStrikeOut(true);
    strikethrough_f.setForeground(QBrush(Qt::red));
    strikethrough_f.setFontPointSize(16);

    tpos.block = 1;
    tpos.position = 0;
    tx_position = 0;
    last_size = 0;
    is_transmitting = false;
    // For debug only
    key_down_count = 0;

    // Initialize circular buffer
    ccbuf.clear();

    // Start a thread to handle transmit characters asynchronously
    tx_thread_p = new CWTX_Thread(this);
    tx_thread_p->setObjectName("CwTxThread");
    tx_thread_p->cbuf_p = &ccbuf;
    tx_thread_p->start();

    // Connect signals
    connect(this, &QTextEdit::textChanged, this, &TransmitWindow::processTextChanged, Qt::QueuedConnection);
    // connect(this, &QTextEdit::cursorPositionChanged, this, &TransmitWindow::CursorPositionChangedSlot);

    connect(this, &TransmitWindow::startTx, tx_thread_p, &CWTX_Thread::sendToSerialPortTx, Qt::QueuedConnection);
    connect(tx_thread_p, &CWTX_Thread::deQueueChar, this, &TransmitWindow::markCharAsSent, Qt::QueuedConnection);

}

TransmitWindow::~TransmitWindow() {

    qDebug() << "TransmitWindow::~TransmitWindow(): DTOR Entered";
    tx_thread_p->quit();
    qDebug() << "TransmitWindow::~TransmitWindow(): DTOR About to call wait()";
    tx_thread_p->wait();
    qDebug() << "TransmitWindow::~TransmitWindow(): DTOR wait() completed";
    delete tx_thread_p;
    qDebug() << "TransmitWindow::~TransmitWindow(): DTOR Exiting";
}

void TransmitWindow::keyPressEvent(QKeyEvent *event) {

    char c;
    int key = event->key();
    bool b;

    CBuffer &bf = ccbuf;
    // qDebug() << "TransmitWindow::keyPressEvent(): key:" << event->key();

    key_down_count++;

    if ( key == Qt::Key_Escape) {
        // Set Rig to RX immediately here
        txReset();
        goto keyPressEventDone;
    }

    // This is a debug tool
    if ( key == Qt::Key_Backslash ) {
        // R() is a C++ raw string literal - dunno where I learned that :-)
        qDebug() << R"(\\\\\)" << "text window size" << toPlainText().size() << "tx_position" << tx_position;
        goto keyPressEventDone;
    }

#if 0
    // Allow the Delete key
    if ( key == Qt::Key_Delete || key == Qt::Key_Backspace ) {
        buffMutex.lock();
        if ( !bf.deleteLast() ) {
            QApplication::beep();
        }
        buffMutex.unlock();
        goto keyPressEventDone;
    }
#endif

    if ( key == Qt::Key_Return )
        goto keyPressEventDone;

    c = (char) key;
    c = toupper(c);

    buffMutex.lock();
    b = bf.put(c);
    buffMutex.unlock();
    if ( b == false ) {
        QApplication::beep();
        // Ignore this character - buffer is full
        goto keyPressEventDone;
    }
    emit startTx(); // Send character to tx thread for transmission

    // Reset the text formatting
    // At this point, the character reported by this event hasn't hit the text window yet.
    // So size() will return zero on the first character.  Duh!
    // So we'll let the KeyReleased() event do the unhighlighting

keyPressEventDone:
    QTextEdit::keyPressEvent((event));
}

void TransmitWindow::keyReleaseEvent(QKeyEvent *event) {

    CBuffer &bf = ccbuf;

    // Suppoprt for the delete key
    if ( event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace ) {
        qDebug() << "TransmitWindow::keyReleaseEvent(): Backspace: text window size:" << toPlainText().size() << "tx_position" << tx_position;
        buffMutex.lock();
        bool rc = bf.deleteLast();
        buffMutex.unlock();
        if ( rc == false ) {
            qDebug() << "TransmitWindow::keyReleaseEvent(): buffer empty - can't backspace";
            QApplication::beep();
        }
    }

keyReleaseEventDone:
    QTextEdit::keyReleaseEvent(event);
}

void TransmitWindow::processTextChanged() {

    // This signal/slot is emitted when any change happens in the edit box including format changes
    txChar.position = cursor.position();
    // qDebug() << "TransmitWindow::processTextChanged(): Entered: Position" << txChar.position;
}

void TransmitWindow::markCharAsSent() {

    int size = toPlainText().size();
    int cpos = cursor.position();
    qDebug() << "TransmitWindow::markCharAsSent() - Slot Entered: size:" << size << "cursor.position:" << cpos << "tx_position" << tx_position;

    // Did we get a clear() event (ESC)?
    if ( size == 0 ) return;

    highlightTextMutex.lock();

    // Higlight (using strikeout and color) the character at tx_position
    cursor.setPosition(tx_position, QTextCursor::MoveAnchor);
    qDebug() << "TransmitWindow::markCharAsSent() - setPosition: size:" << size << "cursor.position:" << cpos << "tx_position" << tx_position;

    bool b = cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);    // Move right one position
    qDebug() << "TransmitWindow::markCharAsSent() - movePosition: size:" << size << "cursor.position:" << cpos << "tx_position" << tx_position;
    if ( b == false ) {
        qDebug() << "cursor.movePosition() failed6:" << "cursor anchor" << cursor.anchor() << "tx_position:" << tx_position << "text window size" << toPlainText().size();
        highlightTextMutex.unlock();
        return;
    }

    // Apply the colorization to the current cursor selection
    cursor.setCharFormat(strikethrough_f);

    // Reset for the next character
    cursor.clearSelection();
    setCurrentCharFormat(normal_f);
    highlightTextMutex.unlock();
    // qDebug() << "TransmitWindow::mackCharAsSent(): debug count =" << "tx_position =" << tx_position;

    tx_position++;
}

void TransmitWindow::mousePressEvent(QMouseEvent *event) {
    // Disable mouse click events in the transmit window
    // This essentially discards the mouse click event
    (void)event; // Supress warning about unused variable 'event'
    qDebug() << "CLICK" << "position" << cursor.position() << "anchor" << cursor.anchor();
    QTextEdit::mousePressEvent(event);
}

void TransmitWindow::CursorPositionChangedSlot() {
    // int size = toPlainText().length();
    qDebug() << "TransmitWindow::cursorPositionChangedSlot(): position:" << cursor.position();
}

void TransmitWindow::txReset() {

    CBuffer &bf = ccbuf;

    clear();
    tpos.position = 0;      // clear() sets block back to its initial value of 1
    tpos.block = 1;
    last_size = 0;
    tx_position = 0;
    buffMutex.lock();
    bf.clear();      // Clear our circular buffer
    buffMutex.unlock();
}

// *********************   QThread Class *********************************

CWTX_Thread::CWTX_Thread(TransmitWindow *p) {

    transmitNow = false;
    txwinObj_p = p;
    paused = false;
    txAvailable = true;
}

void CWTX_Thread::run() {

    qDebug() << "CWTX_Thread::run(): :Entered";
    exec();

    qDebug() << "CWTX_Thread::run(): Thread Exiting";
    return;
}

void CWTX_Thread::sendToSerialPortTx() {

    ulong spincount = 0;
    char c;
    CBuffer &b = *cbuf_p;

    // dequeue a character from the buffer and send it to the serial port
    // When the serial port reports that the transmission has completed, we strikeout the text in the edit box (not done here)
    while ( !txAvailable ) {
        QCoreApplication::processEvents();
        if ( spincount++ > 1000000 )
            break;
    }
    qDebug() << "******* ON EXIT: Spincount:" << spincount << "*************";

    qDebug() << QDateTime::currentMSecsSinceEpoch() << "CWTX_Thread::sendToSerialPortTx() slot entered: buff filled to" << b.getNumCharsQueued();
    if ( !paused ) {
        // qDebug() << "CWTX_Thread::sendToSerialPortTx(): ***transmitNow*** signal arrived: ***Locking";
        buffMutex.lock();
        c = b.get();
        // qDebug() << "CWTX_Thread::sendToSerialPortTx(): ***transmitNow*** signal arrived: ***Unlocking";
        buffMutex.unlock();
        if ( c == -1 ) {
            // Buffer is empty
            qDebug() << "CWTX_Thread::sendToSerialPortTx(): buffer empty";
            return;  // Is this what we want to do here?
        }
        currentTxChar = c;
        txAvailable = false;
        emit sendTxChar(c);     // Send character to serial port, invokes slot processTxChar()
    }
    qDebug() << "CWTX_Thread::sendToSerialPortTx(): Exiting handler";
}

void CWTX_Thread::pauseTx(bool pause) {

    // Pause is a front panel button to pause tx while you type ahead without sending
    qDebug() << "CWTX_Thread::pauseTx(): pause =" << pause;
    paused = pause;
}

void CWTX_Thread::serialPortTxCharComplete() {
    // TODO There could be a race here w/ currentTxChar.  Could it change before use here?
    emit deQueueChar();  // Performs the strikeout of transmitted char
    txAvailable = true;
    qDebug() << "CWTX_Thread::serialPortTxCharComplete(): Entered: TxAvailable: True;";
}

#if 0
int CWTX_Thread::calculate_delay(char c) {

    int a_size = sizeof(valid_keys_timing) / sizeof(valid_keys_timing[0]);
    int ms_delay = 0, i;

    for ( i=0; i<a_size; i++) {
        if ( c == valid_keys_timing[i].letter) {
            break;
        }
    }

    if ( i == a_size) {
        // Character not found
        qDebug() << "CWTX_Thread::run(): char not found" << c;
        QApplication::exit(12);
    } else {
        ms_delay = valid_keys_timing[i].duration * dit_timing_factor;
        // qDebug() << "CWTX_Thread::run(): ms_delay =" << ms_delay;
    }
    return ms_delay;
}
#endif
