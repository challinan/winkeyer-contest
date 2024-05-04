#include "sqlite3-connector.h"
#include "serialcomms.h"

#define DISPLAY_ALL_BYTES
#define READ_BUFFER_SIZE 8
// #define SIMULATE_TX_SERIAL_DELAY

SerialComms::SerialComms(QObject *parent, Sqlite3_connector *p)
    : QObject{parent}
{
    db = p;
    winkeyer_open = false;
    active_serial_port_p = nullptr;
    speedpot_timer_fired = false;
    simTimerRunning = false;

    // Set size and clear read and write buffers
    // QString ptrStr = QString( "0x%1" ).arg( reinterpret_cast<quintptr>(read_buffer.data()),
    //                                      QT_POINTER_SIZE * 2, 16, QChar('0') );
    read_buffer.resize(READ_BUFFER_SIZE);

    // Fetch the configured serial port from the database tables
    config_serial_port = db->getSysconfigDbClassPtr()->getConfiguredSerialPort();

    qDebug() << "SerialComms::SerialComms() ctor: Port" << config_serial_port;
}

SerialComms::~SerialComms() {

    if ( active_serial_port_p ) {
        close_serial_port();
        delete active_serial_port_p;
    }
    // Disconnect signals ???

}

bool SerialComms::openSerialPort() {

    // Qt Default values: No Parity, 8-Bit data, 1 Stop bit
    active_serial_port_p = new QSerialPort;
    active_serial_port_p->setPortName(config_serial_port);
    active_serial_port_p->setBaudRate(QSerialPort::Baud1200);
    qDebug() << "SerialComms::openSerialPort(): selected serial port is " << active_serial_port_p->portName();
    bool retcode = active_serial_port_p->open(QIODeviceBase::ReadWrite);
    if ( !retcode ) {
        qDebug() << "SerialComms::openSerialPort(): serial port open() failed:" << active_serial_port_p->errorString();
        delete active_serial_port_p;
        active_serial_port_p = nullptr;
        return false;
    }
    active_serial_port_p->clear();

    connect(active_serial_port_p, &QIODevice::readyRead, this, &SerialComms::slot_readyRead, Qt::QueuedConnection);
    connect(this, &SerialComms::serial_rx, this, &SerialComms::processRxMessage);

    open_winkeyer();
    return true;
}

void SerialComms::close_serial_port() {

    close_winkeyer();

    // Make sure all bytes have been transmitted
    while ( active_serial_port_p->bytesToWrite() > 0 ) {
        active_serial_port_p->waitForBytesWritten(100);
    }

    active_serial_port_p->close();
}

void SerialComms::open_winkeyer() {

    // Winkeyer Wk3 Open Command: 0x00, 0x02
    qDebug() << "SerialComms::open_winkeyer(): Entered";
    write_buffer.append(static_cast<char>(0x0));
    write_buffer.append(static_cast<char>(0x02));
    writeSerialData();

    // Validate that open succeeded by waiting for open
    int spincount = 0;
    while ( !winkeyer_open ) {
        QApplication::processEvents();
        if ( spincount++ > 100000 ) {
            qDebug() << "SerialComms::open_winkeyer(): Timeout waiting for winkeyer version" << "spincount: " << spincount;
            break;
        }
    }
    qDebug() << "winkeyer_open(): spincount:" << spincount;

    // On successful open, connect signal
    c_statusByteConnx = connect(this, &SerialComms::statusByteReceived, this, &SerialComms::processStatusByte);
    c_speedPotRxConnx = connect(this, &SerialComms::speedPotValueReceived, this, &SerialComms::processSpeedPot);
}

void SerialComms::processRxMessage(QByteArray &b) {

    if ( b.size() > 0 ) {
        char c = b.at(0);
        // qDebug() << "SerialComms::processRxMessage(): bytearray size:" << b.size() << "value:" << b.at(0);
        if ( c == 0x1f) {
            version_string = QString::number(c);
            qDebug() << "SerialComms::processRxMessage(): Reporting winkeyer open!! - Version:" << version_string;
            winkeyer_open = true;
        }
    }
}

void SerialComms::setSpeed(int speed) {

    if ( winkeyer_open ) {

        // Winkeyer Wk3 Set Speed command <0x02> <nn>
        qDebug() << "SerialComms::setSpeed(): Entered - speed:" << speed;
        write_buffer.append(static_cast<char>(0x02));
        write_buffer.append(static_cast<char>(speed));
        writeSerialData();  // Buffer cleared by write function
        currentSpeed = speed;

    } else {

        qDebug() << "SerialComms::setSpeed(): winkeyer not open";
    }
}

int SerialComms::getSpeed() {
    return currentSpeed;
}

void SerialComms::setupSpeedPotRange(uchar min, uchar range) {

    // Winkeyer Wk3 Setup Speed Pot <05><nn1><nn2><nn3> nn1 = MIN, nn2 = RANGE, nn3 = 0
    qDebug() << "SerialComms::setupSpeedPotRange(): Entered - min:" << min << "range:" << range;
    write_buffer.append(static_cast<char>(0x05));
    write_buffer.append(static_cast<char>(min));
    write_buffer.append(static_cast<char>(range));
    write_buffer.append(static_cast<char>(0));
    writeSerialData();
}

void SerialComms::close_winkeyer() {

    // Winkeyer Wk3 Close command 0x00, 0x03
    qDebug() << "SerialComms::close_winkeyer(): Entered: serial port:" << active_serial_port_p;
    reset_winkeyer();
    active_serial_port_p->waitForBytesWritten(100);


    // Send Winkeyer close command
    write_buffer.append(static_cast<char>(0x00));
    write_buffer.append(static_cast<char>(0x03));
    writeSerialData();
    active_serial_port_p->waitForBytesWritten(100);

    winkeyer_open = false;
    disconnect(c_statusByteConnx);
    disconnect(c_speedPotRxConnx);
}

void SerialComms::clearWinkeyerBuffer() {
    // Winkeyer Wk3 Clear Buffer command 0x0A
    write_buffer.append(static_cast<char>(0x0A));
    writeSerialData();
}

void SerialComms::reset_winkeyer() {
    // Winkeyer Wk3 Close command 0x00, 0x01
    write_buffer.append(static_cast<char>(0x00));
    write_buffer.append(static_cast<char>(0x01));
    writeSerialData();
}

void SerialComms::doEchoTest() {

    // Winkeyer Wk3 echo command 0x00 0x04
    write_buffer.append(static_cast<char>(0x00));
    write_buffer.append(static_cast<char>(0x04));
    write_buffer.append(static_cast<char>('A'));
    writeSerialData();
}

int SerialComms::writeSerialData() {

    int rc;
    bool bytes_written_ok;
    QElapsedTimer timer;
    timer.start();

    // qDebug() << "SerialComms::writeSerialData(): size = " << write_buffer.size() << "buffer contains: " << write_buffer;
    if ( active_serial_port_p == nullptr ) {
        // Winkeyer is not connected or serial port open failed.  Let's simulate tx
        emit TxCharComplete();
        return 0;
    }

    rc = active_serial_port_p->write(write_buffer);
    // qDebug() << "SerialComms::writeSerialData(): wrote " << rc << "bytes";
    if ( rc == -1 ) {
        qDebug() << "SerialComms::writeSerialData(): write error:" << active_serial_port_p->errorString();
    }
    write_buffer.clear();

    // Wait for tx complete.  Timeout value in mS
    bytes_written_ok = active_serial_port_p->waitForBytesWritten(1000);
    if ( !bytes_written_ok ) {
        qDebug() << "SerialComms::writeSerialData(): Timeout waiting for bytes written";
    }
    emit TxCharComplete();
    qDebug() << "SerialComms::writeSerialData(): Elapsed time" << timer.nsecsElapsed() << "nanoseconds";
    return rc;
}

void SerialComms::slot_readyRead() {
    readSerialData();

}

#if 0
// Pseudo code from Wk3 datasheet V1.3
Serial Comm Thread {
    while (1) {
        if (host has a command to send to WK3) {
               send command to WK3;
        }
        else if (WK3:uart_byte_ready) {
            wkbyte = WK3:uart_read();
            if (( wkbyte & 0xc0) == 0xc0 {
               // it’s a status byte. (Host may or may not have asked for it.)
               // process status change, note that it could be a pushbutton change
           }
           else if ((wkbyte & 0xc0) == 0x80) {
               it’s a speed pot byte (Host may or may not have asked for it.)
               process speed pot change
           }
           else {
               it must be an echo back byte
               if (break-in==1) { it’s a paddle echo }
               else { it’s a serial echo }
           }
        }
    }
}
#endif

int SerialComms::readSerialData() {

    int rc = 0;
    int countReady = 0;

    if ( active_serial_port_p == nullptr )
        return -1;

    // qDebug() << "SerialComms::readSerialData(): Entered - read_buffer size:" << read_buffer.size();
    // scomm_mutex.lock();

    while ( (countReady = active_serial_port_p->bytesAvailable()) ) {
        if ( countReady < 0 || countReady > 8 ) {
            qDebug() << "SerialComms::readSerialData(): ******************* countReady unusual value:" << countReady;
            continue;
        }

        // qDebug() << "SerialComms::readSerialData(): bytesAvailable:" << countReady;
        read_buffer.resize(countReady);
        rc = active_serial_port_p->read(read_buffer.data(), countReady < READ_BUFFER_SIZE ? countReady : READ_BUFFER_SIZE-1);
        if ( rc == -1 ) {
            qDebug() << "SerialComms::readSerialData(): ERROR:" << active_serial_port_p->error();
        }

        if ( (read_buffer.at(0) & 0xc0) == 0xc0 ) {
            // It’s a status byte. (Host may or may not have asked for it.)
            // qDebug() << "SerialComms::readSerialData(): Status byte detected";
            emit statusByteReceived(read_buffer.at(0));
        } else {
            if ( (read_buffer.at(0) & 0xc0) == 0x80) {
                // Speed Pot value received (The two MS Bits of a Speed Pot status byte will always be b10)
                emit speedPotValueReceived(read_buffer.at(0) & 0x3f);
            }
        }
        // qDebug() << "SerialComms::readSerialData(): return value" << rc << "byte count:" << rc << "data:" << read_buffer;
        emit serial_rx(read_buffer);
    }
    return rc;
}

void SerialComms::add_byte(char c) {
        write_buffer.append(c);
}

void SerialComms::display_all_bytes(QByteArray &r) {

#ifdef DISPLAY_ALL_BYTES
    qDebug() << "SerialComms::display_all_bytes(): Entered with" << r.size() << "bytes";
    QByteArray::iterator i;
    int count = 0;
    for (i = r.begin(); i != r.end() ; i++ ) {
        QByteArray test1Byte(1,0);  // Define a 1 Byte fixed length variable
        test1Byte[0]= r.at(count++);    // Assign each byte of the QByteArray to this variable
        qDebug() << test1Byte.toHex();  // Print that 1 Byte in hex format
    }
#endif
}

void SerialComms::readVCC() {
    char outc = 0x0;
    write_buffer.append(outc);
    outc = 21;
    write_buffer.append(outc);
    qDebug() << "SerialComms::readVCC(): write_buffer size is now" << write_buffer.size();
    display_all_bytes(write_buffer);
    writeSerialData();
    write_buffer.clear();
    qDebug() << "SerialComms::readVCC(): Done";
}

void SerialComms::processStatusByte(uchar status) {

    // qDebug() << "SerialComms::processStatusByte: Slot entered" << status;
    // Bit 7-5: 110   (0xC0 mask) identiies status byte
    // bit 4: WAIT    (Winkeyer waiting for internal event to finish
    // bit 3: KEYDOWN (keydown status when 1)
    // bit 2: BUSY    (winkeyer busy sending more when 1)
    // bit 1: BREAKIN (paddle breaking active when 1)

    if ( status & WINKEYER_WAIT ) qDebug() << "    winkey status byte: wait active";
    if ( status & WINKEYER_KEYDOWN ) qDebug() << "    winkey status byte: keydown active:";
    if ( status & WINKEYER_BUSY ) qDebug() << "    winkey status byte: busy active:";
    if ( status & WINKEYER_BREAKIN ) qDebug() << "    winkey status byte: breakin active";
    if ( status & WINKEYER_XOFF ) qDebug() << "    winkey status byte: XOFF active - buffer 2/3 full";
    if ( (status & 0xc0) == 0xc0 ) qDebug() << "    winkey status byte: idle";

}

void SerialComms::processSpeedPot(uchar speed) {

    uchar setSpeed = speed + SPEEDPOT_MIN;
    currentSpeed = setSpeed;
    qDebug() << "SerialComms::processSpeedPot: Slot entered: speed" << setSpeed;

    if ( speedpot_timer_fired == false ) {
        speedpot_timer_fired = true;
        speedPotTimer = new QTimer(this);
        speedPotTimer->setSingleShot(true);
        c_speedpot_timerConnx = connect(speedPotTimer, &QTimer::timeout, this, &SerialComms::slotSendWinkeyerSpeed);
        speedPotTimer->start(350);
        return;
    } else  {
        speedPotTimer->setInterval(350); // Bump timer
    }
}

void SerialComms::slotSendWinkeyerSpeed() {

    qDebug() << "SerialComms::slotSendWinkeyerSpeed(): speed" << currentSpeed;
    speedpot_timer_fired = false;
    disconnect(c_speedpot_timerConnx);
    delete speedPotTimer;

    setSpeed(currentSpeed);
}

// Slot on signal sendTxChar()
void SerialComms::processTxChar(char c) {

    qDebug() << "SerialComms::processTxChar(): Char:" << c;

#ifdef SIMULATE_TX_SERIAL_DELAY
    if ( !simTimerRunning ) {
        txSimTimer = new QTimer(this);
        txSimTimer->setSingleShot(true);
        c_sim_timerConnx = connect(txSimTimer, &QTimer::timeout, this, &SerialComms::simTimerTimeout);
        txSimTimer->start(750);
        simTimerRunning = true;
    }
#else
    write_buffer.append(c);
    writeSerialData();
#endif
}

void SerialComms::simTimerTimeout() {

#ifdef SIMULATE_TX_SERIAL_DELAY
    simTimerRunning = false;
    qDebug() << "SerialComms::simTimerTimeout(): ************** simTimer Timeout";
    disconnect(c_sim_timerConnx);
    delete txSimTimer;
    emit TxCharComplete();  // Let the CwTxThread object know we've sent the character
#endif
}
