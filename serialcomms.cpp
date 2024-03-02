#include "sqlite3-connector.h"
#include "serialcomms.h"

#define DISPLAY_ALL_BYTES
#define READ_BUFFER_SIZE 8

class SerialRxCommsThread;

SerialComms::SerialComms(QObject *parent, Sqlite3_connector *p)
    : QObject{parent}
{
    db = p;
    winkeyer_open = false;
    runRxThread = false;
    active_serial_port_p = nullptr;

    // Set size and clear read and write buffers
    // QString ptrStr = QString( "0x%1" ).arg( reinterpret_cast<quintptr>(read_buffer.data()),
    //                                      QT_POINTER_SIZE * 2, 16, QChar('0') );
    read_buffer.resize(READ_BUFFER_SIZE);

    // Fetch the configured serial port from the database tables
    config_serial_port = db->getSysconfigDbClassPtr()->getConfiguredSerialPort();

    qDebug() << Qt::endl << "SerialComms::SerialComms() ctor: Port" << config_serial_port;
}

SerialComms::~SerialComms() {

    qDebug() << "SerialComms::~SerialComms(): Destructor entered ****";
    close_serial_port();
    if ( active_serial_port_p )
        delete active_serial_port_p;
    // Disconnect signals ???

    // Stop and kill our worker thread
    runRxThread = false;
#if 0
    pRxThread->terminate();
    pRxThread->wait();
    delete pRxThread;
#endif
}

bool SerialComms::openSerialPort() {

    // Qt Default values: No Parity, 8-Bit data, 1 Stop bit
    active_serial_port_p = new QSerialPort;
    // active_serial_port_p->setPort(config_serial_port);
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

    open_winkeyer();
    // setupSpeedPotRange();

    return true;
}

void SerialComms::close_serial_port() {

    qDebug() << "SerialComms::close_serial_port(): closing serial port";
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
    WriteSerialData();

    // TODO: Validate that open succeeded by waiting for the version string
    winkeyer_open = true;
}

void SerialComms::setSpeed(int speed) {

    if ( winkeyer_open ) {

        // Winkeyer Wk3 Set Speed command <0x02> <nn>
        qDebug() << "SerialComms::setSpeed(): Entered - speed:" << speed;
        write_buffer.append(static_cast<char>(0x02));
        write_buffer.append(static_cast<char>(speed));
        WriteSerialData();  // Buffer cleared by write function

    } else {

        qDebug() << "SerialComms::setSpeed(): winkeyer not open";
    }
}

void SerialComms::setupSpeedPotRange(uchar min, uchar max) {

    // Winkeyer Wk3 Setup Speed Pot <05><nn1><nn2><nn3> nn1 = MIN, nn2 = RANGE, nn3 = 0
    qDebug() << "SerialComms::setupSpeedPotRange(): Entered";
    write_buffer.append(static_cast<char>(0x05));
    write_buffer.append(static_cast<char>(min));
    write_buffer.append(static_cast<char>(max));
    write_buffer.append(static_cast<char>(0));
    WriteSerialData();

    // TODO: Validate that open succeeded by waiting for the version string
    winkeyer_open = true;
}

void SerialComms::close_winkeyer() {

    // Winkeyer Wk3 Close command 0x00, 0x03
    qDebug() << "SerialComms::close_winkeyer(): Entered";
    write_buffer.append(static_cast<char>(0x00));
    write_buffer.append(static_cast<char>(0x03));
    WriteSerialData();
    winkeyer_open = false;
}

void SerialComms::clearWinkeyerBuffer() {
    // Winkeyer Wk3 Clear Buffer command 0x0A
    write_buffer.append(static_cast<char>(0x0A));
    WriteSerialData();
}

void SerialComms::doEchoTest() {

    // Winkeyer Wk3 echo command 0x00 0x04
    write_buffer.append(static_cast<char>(0x00));
    write_buffer.append(static_cast<char>(0x04));
    write_buffer.append(static_cast<char>('A'));
    WriteSerialData();
}

int SerialComms::WriteSerialData() {
    int rc;

    qDebug() << "SerialComms::WriteSerialData(): size = " << write_buffer.size() << "buffer contains: " << write_buffer;
    if ( active_serial_port_p == nullptr )
        return 0;

    rc = active_serial_port_p->write(write_buffer);
    // qDebug() << "SerialComms::WriteSerialData(): wrote " << rc << "bytes";
    if ( rc == -1 ) {
        qDebug() << "SerialComms::WriteSerialData(): write error:" << active_serial_port_p->errorString();
    }
    write_buffer.clear();
    return rc;
}

// Slot - called by signal serial_out() in networkcomms to indicate serial data ready to send out
void SerialComms::console_data_2_serial_out(QByteArray &b) {

    qDebug() << "SerialComms::console_data_2_serial_out(): Signal received ***************";
    write_buffer.clear();
    write_buffer = b;
    WriteSerialData();
}

void SerialComms::slot_readyRead() {
    readserialdata();

#if 0
    pRxThread = new SerialRxCommsThread;
    pRxThread->setSerialCommsPtr(this);
    runRxThread = true;
    connect(pRxThread, &SerialRxCommsThread::serialRxReady, this, &SerialComms::readserialdata);
    connect(pRxThread, &SerialRxCommsThread::finished, pRxThread, &QObject::deleteLater);

    pRxThread->start();
#endif

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
               it’s a status byte. (Host may or may not have asked for it.)
               process status change, note that it could be a pushbutton change
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

int SerialComms::readserialdata() {

    int rc = 0;
    int countReady = 0;

    if ( active_serial_port_p == nullptr )
        return -1;

    // qDebug() << "SerialComms::readserialdata(): Entered - read_buffer size:" << read_buffer.size();
    // scomm_mutex.lock();

    while ( (countReady = active_serial_port_p->bytesAvailable()) ) {
        if ( countReady < 0 || countReady > 8 ) {
            qDebug() << "SerialComms::readserialdata(): ******************* countReady unusual value:" << countReady;
            continue;
        }
        rc = active_serial_port_p->read(read_buffer.data(), countReady < READ_BUFFER_SIZE ? countReady : READ_BUFFER_SIZE-1);
        if ( rc == -1 ) {
            qDebug() << "SerialComms::readserialdata(): ERROR:" << active_serial_port_p->error();
        }

        if ( (read_buffer.at(0) & 0xc0) == 0xc0 ) {
            // It’s a status byte. (Host may or may not have asked for it.)
            qDebug() << "SerialComms::readserialdata(): Status byte detected";
        } else {
            if ( (read_buffer.at(0) & 0xc0) == 0x80) {
                // The two MS Bits of a Speed Pot status byte will always be b10
                int speedPot = read_buffer.at(0) & 0x3f;
                qDebug() << "SerialComms::readserialdata(): Speed Pot byte detected" << read_buffer << speedPot;
            }
        }
        qDebug() << "SerialComms::readserialdata(): return value" << rc << "byte count:" << rc << "data:" << read_buffer;
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
    WriteSerialData();
    write_buffer.clear();
    qDebug() << "SerialComms::readVCC(): Done";
}

void SerialRxCommsThread::run() {

    qDebug() << "SerialRxCommsThread::run(): Entered";


    while ( pSerialComm->runRxThread ) {
        // pSerialComm->scomm_mutex.lock();

        qDebug() << "SerialRxCommsThread::run(): runRxThread:" << pSerialComm->runRxThread;
        // emit serialRxReady(count);
        QThread::sleep(1);
    }
}

void SerialRxCommsThread::setSerialCommsPtr(SerialComms *p) {
    pSerialComm = p;
}
