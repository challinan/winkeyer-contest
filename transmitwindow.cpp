#include "transmitwindow.h"
#include "morse_table.h"

QMutex buffMutex;

CBuffer::CBuffer() {
    head = 0;
    tail = 0;
    size = 0;

}

CBuffer::~CBuffer() {
    delete cbuff;
}

bool CBuffer::setSize(int s) {
    int size = s;
    cbuff = new char[size];

    // For debug only
    // memset(cbuff, 0, size);

    if ( !cbuff ) return false;
    return true;
}

bool CBuffer::put(char c) {

    // Check for buffer full
    if ( isFull() ) {
        // Buffer full
        return false;
    }

    // We write to "tail" and read from "head"
    cbuff[tail] = c;
    tail = (tail + 1) % CBUFF_SIZE;
    return true;
}

char CBuffer::get() {

    if ( isEmpty() ) {
        // Buffer empty
        return -1;
    }

    // We write to "tail" and read from "head"
    char c = cbuff[head];
    head = (head + 1) % CBUFF_SIZE;
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

    tail = (tail == 0) ? CBUFF_SIZE - 1 : (tail - 1) % CBUFF_SIZE;

    qDebug() << "CBuffer::deleteLast(): head:" << head << "Head char:" << cbuff[head] << "Tail index:" << tail << "tail char:" << cbuff[tail];
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

    tx_position = 0;
    highlight_position = 0;
    is_transmitting = false;

    // For debug only
    key_down_count = 0;
    key_release_count = 0;
    strikeout_count = 0;

    // Initialize circular buffers
    ccbuf.setSize(CBUFF_SIZE);
    ccbuf.clear();
    txWindowBuffer.setSize(CBUFF_SIZE*2);
    txWindowBuffer.clear();

    // Start a thread to handle transmit characters asynchronously
    tx_thread_p = new CWTX_Thread(this);
    tx_thread_p->setObjectName("CwTxThread");
    tx_thread_p->cbuf_p = &ccbuf;
    tx_thread_p->start();

    // Set a default in case winkeyer is not connected or serial port open failed
    reportTxSpeed(18);

    // Connect signals
    connect(this, &QTextEdit::textChanged, this, &TransmitWindow::processTextChanged, Qt::QueuedConnection);
    connect(this, &TransmitWindow::startTx, tx_thread_p, &CWTX_Thread::sendToSerialPortTx, Qt::QueuedConnection);
    connect(tx_thread_p, &CWTX_Thread::deQueueChar, this, &TransmitWindow::strikeoutCharAsSent, Qt::QueuedConnection);
}

TransmitWindow::~TransmitWindow() {

    tx_thread_p->quit();
    tx_thread_p->wait();
    delete tx_thread_p;
}

void TransmitWindow::keyPressEvent(QKeyEvent *event) {

    char c;
    bool b;
    int key = event->key();

    CBuffer &bf = ccbuf;
    QString str;

    // Debug stuff
    switch ( key ) {
    case Qt::Key_Backspace:
        str = "BS";
        break;
    case Qt::Key_Delete:
        str = "DEL";
        break;
    case Qt::Key_Space:
        str = " ";
        break;
    case Qt::Key_Return:
        str = "CR";
        break;
    case Qt::Key_Control:
        return;
    default:
        str.append(key < 0xfff ? QChar(key) : QChar('?'));
        break;
    }

qDebug() << "                                                          TransmitWindow::keyPressEvent(): key:" << str << tx_position;

    key_down_count++;

    if ( key == Qt::Key_Escape) {
        // Set Rig to RX immediately here
        txReset();
        goto keyPressEventDone;
    }

    // Suppoprt for the delete key
    if ( event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace ) {
        qDebug() << "TransmitWindow::keyPressEvent(): Backspace: text window size:" << toPlainText().size() << "tx_position" << tx_position;
        buffMutex.lock();
        bool rc = bf.deleteLast();
        tx_position = tx_position == 0 ? 0 : tx_position - 1;
        buffMutex.unlock();
        if ( rc == false ) {
            qDebug() << "TransmitWindow::keyPressEvent(): buffer empty - can't backspace";
            QApplication::beep();
        }
        goto keyPressEventDone;
    }

    // Don't transmit Return key, but it must be accounted for in our highlighting scheme
    if ( key == Qt::Key_Return ) {
        qDebug() << "TransmitWindow::keyPressEvent(): RETURN KEY!!" << "tx_position" << tx_position;
        tx_position++;  // Return key is in the transmit window, but invisible.  Need to count it.
        emit notifyMainWindowCR(key);
        goto keyPressEventDone;
    }

    // Discard useless characters
    if ( key < Qt::Key_Space || key > Qt::Key_AsciiTilde ) {
        // qDebug() << "TransmitWindow::keyPressEvent(): Discarding key" << static_cast<char>(key) << Qt::hex << static_cast<unsigned int>(key);
        QApplication::beep();

        goto keyPressEventDone;
    }

    txCharStamp.c = event->key();
    txCharStamp.block = cursor.blockNumber();
    txCharStamp.position = cursor.position();
    txCharStamp.complete = false;

    c = (char) key;
    c = toupper(c);

    buffMutex.lock();
    b = bf.put(c);
    txCharStamp.c = b;
    txCharStamp.position = tx_position++;
    txCharStamp.block = cursor.blockNumber();
    buffMutex.unlock();
    if ( b == false ) {
        QApplication::beep();
        // Ignore this character - buffer is full
        return; // Discard this character
    }

    buffMutex.lock();
    b = txWindowBuffer.put(c);
    if ( b == false ) {
        qDebug() << "TransmitWindow::keyPressEvent(): Buffer full: key:" << static_cast<char>(c);
        // Ignore this character - buffer is full
        buffMutex.unlock();
        return; // Discard this character
    }
    buffMutex.unlock();

    // qDebug() << "TransmitWindow::keyPressEvent(): key - sending to startTX (slot CWTX_Thread::sendToSerialTx):" << static_cast<char>(event->key());
    emit startTx(); // Send character to tx thread for transmission

    // At this point, the character reported by this event may not have hit the text window yet.  There's no way to confirm

keyPressEventDone:
    QTextEdit::keyPressEvent((event));
}

void TransmitWindow::keyReleaseEvent(QKeyEvent *event) {

    key_release_count++;

    // Do we need this?
    // qDebug() << "TransmitWindow::keyReleaseEvent(): block:" << txCharStamp.block << "position:" << txCharStamp.position << "key:" << static_cast<char>(event->key());
    QTextEdit::keyReleaseEvent(event);
}

void TransmitWindow::processTextChanged() {

    // This signal/slot is emitted when any change happens in the edit box including format changes
   // qDebug() << "TransmitWindow::processTextChanged(): Entered: Position" << txChar.position;
}

void TransmitWindow::strikeoutCharAsSent() {

    strikeout_count++;
    qDebug() << "TransmitWindow::strikeoutCharAsSent() - Slot Entered: strikeout_count:" << strikeout_count
             << "cursor.position:" << cursor.position() << "tx_position" << tx_position;

    // Did we get a clear() event (ESC)?
    if ( toPlainText().size() == 0 ) return;

    buffMutex.lock();
    char current_char = txWindowBuffer.get();
    if ( current_char == -1 ) {
        qDebug() << "TransmitWindow::strikeoutCharAsSent(): txWindowBuffer buffer empty - nothing to do";
        return;
    }

    // Higlight (using strikeout and color) the character at tx_position
    cursor.setPosition(highlight_position, QTextCursor::MoveAnchor);
    if ( cursor.atBlockEnd() ) {
        highlight_position++;
        cursor.setPosition(highlight_position, QTextCursor::MoveAnchor);
        qDebug() << "TransmitWindow::strikeoutCharAsSent(): At Block End - tx_position:" << tx_position;
    }

    bool b = cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);    // Move right one position
    // qDebug() << "TransmitWindow::strikeoutCharAsSent() - movePosition: size:" << size << "cursor.position:" << cpos << "tx_position" << tx_position;
    if ( b == false ) {
        qDebug() << "TransmitWindow::strikeoutCharAsSent(): cursor.movePosition() failed6:" << "cursor anchor" << cursor.anchor() << "tx_position:" << tx_position << "text window size" << toPlainText().size();
        buffMutex.unlock();
        return;
    }

    // Don't highlight spaces
    QString strTmp = cursor.selectedText();
    if ( strTmp == QString(" ") ) {
        qDebug() << "TransmitWindow::strikeoutCharAsSent(): Ignoring space at tx_position:" << tx_position;
        goto highlight_done;
    }

    // Handle the Qt paragraph separator
    if ( strTmp == QString(QChar(QChar::ParagraphSeparator)) ) {
        qDebug() << "TransmitWindow::strikeoutCharAsSent(): Ignoring CR at tx_position:" << tx_position;
        goto highlight_done;
    }

    // Apply the formatting to the current cursor selection
    cursor.setCharFormat(strikethrough_f);

highlight_done:
    // Reset for the next character
    highlight_position++;
    cursor.clearSelection();
    setCurrentCharFormat(normal_f);
    buffMutex.unlock();
    qDebug() << "TransmitWindow::strikeoutCharAsSent(): tx_position:" << tx_position << "cursor selection" << strTmp;
}

void TransmitWindow::txReset() {

    CBuffer &bf = ccbuf;

    clear();
    tx_position = 0;
    buffMutex.lock();
    bf.clear();      // Clear our circular buffer
    buffMutex.unlock();
}

void TransmitWindow::reportTxSpeed(int speed) {
    txSpeed = speed;
    dit_timing_factor = (1200L / txSpeed) * (4.0f / 5.0f);     // We need to be slightly faster than the rig
    qDebug() << "TransmitWindow::reportTxSpeed(): dit_timing_factor:" << dit_timing_factor;
}

// *********************   QThread Class *********************************

CWTX_Thread::CWTX_Thread(TransmitWindow *p) {

    transmitNow = false;
    txwinObj_p = p;
    paused = false;
    txAvailable = true;
    strikeoutTimerRunning = false;
}

void CWTX_Thread::run() {

    qDebug() << "CWTX_Thread::run(): :Entered";
    exec();

    qDebug() << "CWTX_Thread::run(): Thread Exiting";
    return;
}

#define SPINCOUNT 1000000
// Slot on signal startTx()
void CWTX_Thread::sendToSerialPortTx() {

    ulong spincount = 0;
    char c;
    CBuffer &b = *cbuf_p;

    // dequeue a character from the buffer and send it to the serial port
    // When the serial port reports that the transmission has completed, we strikeout the text in the edit box (not done here)
    while ( !txAvailable ) {
        QApplication::processEvents();
        if ( spincount++ > SPINCOUNT ) {
            qDebug() << "CWTX_Thread::sendToSerialPortTx(): ************************************ Timeout waiting for txAvailable - spincount" << spincount;
            break;
        }
    }
    if ( spincount > SPINCOUNT )
        Q_ASSERT(false);  // FAIL!

    // qDebug() << QDateTime::currentMSecsSinceEpoch() << "CWTX_Thread::sendToSerialPortTx() slot entered: buff filled to" << b.getNumCharsQueued();
    if ( !paused ) {
        // qDebug() << "CWTX_Thread::sendToSerialPortTx(): ***transmitNow*** signal arrived: ***Locking";
        buffMutex.lock();
        c = b.get();
        // qDebug() << "CWTX_Thread::sendToSerialPortTx(): ***transmitNow*** signal arrived: ***Unlocking";
        buffMutex.unlock();

        if ( c == -1 ) {
            // Buffer is empty
            qDebug() << "CWTX_Thread::sendToSerialPortTx(): buffer empty" << txwinObj_p->tx_position << txwinObj_p->txCharStamp.position << txwinObj_p->txCharStamp.c;
            return;  // Is this what we want to do here?
        }

        ms_delay = calculateDelay(c);
        currentTxChar = c;
        txAvailable = false;
        qDebug() << "CWTX_Thread::sendToSerialPortTx(): emit sentTxChar(c) - calls SerialComms::processTxChar() which sends to serial port";
        emit sendTxChar(c);     // Send character to serial port, invokes slot processTxChar() writes to serial port
    }
}

void CWTX_Thread::pauseTx(bool pause) {

    // Pause is a front panel button to pause tx while you type ahead without sending
    qDebug() << "CWTX_Thread::pauseTx(): pause =" << pause;
    paused = pause;
}

// Slot called from signal SerialComm::TxCharComplete()
void CWTX_Thread::serialPortTxCharComplete() {

   // qDebug() << "CWTX_Thread::serialPortTxCharComplete(): Entered: strikeoutTimerRuning:" << strikeoutTimerRunning << "ms_delay" << ms_delay;

    if ( strikeoutTimerRunning ) {
        qDebug() << "CWTX_Thread::serialPortTxCharComplete(): Something is wrong!!****************";
    }

    if ( !strikeoutTimerRunning ) {
        pStrikeoutTimer = new QTimer(this);
        pStrikeoutTimer->setSingleShot(true);
        c_strikeout_timerConnx = connect(pStrikeoutTimer, &QTimer::timeout, this, &CWTX_Thread::strikeoutTimerTimeout);
        pStrikeoutTimer->start(ms_delay);
        strikeoutTimerRunning = true;
    }

    ulong sptx_timeout = 0;
    while ( strikeoutTimerRunning) {
        sptx_timeout++;
        QApplication::processEvents();
        if ( sptx_timeout > 1000000 ) {
            qDebug() << "CWTX_Thread::serialPortTxCharComplete(): *************** UNEXPECTED STRIKEOUT TIMEOUT!! ****************" << sptx_timeout;
            strikeoutTimerRunning = false;
            QApplication::exit(19);
        }
    }
    // qDebug() << "CWTX_Thread::serialPortTxCharComplete(): Exiting: sptx_timeout:" << sptx_timeout;
}

void CWTX_Thread::strikeoutTimerTimeout() {

    // qDebug() << "CWTX_Thread::strikeoutTimerTimeout(): ************** strikeOut Timeout";
    disconnect(c_strikeout_timerConnx);
    strikeoutTimerRunning = false;
    delete pStrikeoutTimer;
    txAvailable = true;
    emit deQueueChar();  // Performs the strikeout of transmitted char
}

int CWTX_Thread::calculateDelay(char c) {

    int d = morseTimingMap.value(c, -1);

    if ( d == -1 ) {
        // Character not found - that's a fatal error
        qDebug() << "CWTX_Thread::calculateDelay(): char not found- exiting!!!!!" << c << static_cast<char>(c) << Qt::hex << static_cast<unsigned int>(c);;
        QApplication::exit(12);
    } else {
        ms_delay = d * txwinObj_p->dit_timing_factor;
        // qDebug() << "**************************************CWTX_Thread::calculateDelay(): ms_delay =" << ms_delay;
    }
    return ms_delay;
}
