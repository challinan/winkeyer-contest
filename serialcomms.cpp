#include "serialcomms.h"

SerialComms::SerialComms(QObject *parent)
    : QObject{parent}
{
    // Enumerate available usb serial ports
    port_info_list = QSerialPortInfo::availablePorts();
    qDebug() << Qt::endl << "SerialComms::SerialComms() entered";
    write_buffer.clear();

    enumerate_serial_devices();
    // TODO - Hardcoded
    // config_serial_str should come from database sysconfig data
    config_serial_str = QString("cu.usbserial-D30BNKJU");

    sendEcho = false;
}

SerialComms::~SerialComms() {
    close_serial_port();
    qDebug() << "SerialComms::~SerialComms(): Destructor entered ****";
    active_serial_port_p->close();
    delete active_serial_port_p;
    // Disconnect signals ???

}

QList<QSerialPortInfo> &SerialComms::get_pList() {
    return port_info_list;
}

void SerialComms::setNetcommObjPointer(NetworkComms *pNetworkObj) {
    network_comm_obj_p = pNetworkObj;
}

void SerialComms::enumerate_serial_devices() {

    qDebug() << "SerialComms::enumerate_serial_devices() entered";
    QList<QSerialPortInfo>::iterator i;
    for (i = port_info_list.begin(); i != port_info_list.end(); i++) {
        QString s_tmp = i->portName();
        if ( s_tmp.startsWith("cu.usbserial") ) {
            emit on_serial_port_detected(s_tmp);
            qDebug() << "SerialComms::enumerate_serial_devices() **** detected" << i->portName();
            if ( s_tmp.compare(config_serial_str) == 0 ) {        // This is our configured value
                selected_serial_port_from_config = *i;
            }
        }
    }
}

int SerialComms::openSerialPort() {

    // Qt Default values: No Parity, 8-Bit data, 1 Stop bit
    active_serial_port_p = new QSerialPort;
    active_serial_port_p->setPort(selected_serial_port_from_config);
    active_serial_port_p->setBaudRate(QSerialPort::Baud1200);
    qDebug() << "SerialComms::openSerialPort(): selected serial port is " << selected_serial_port_from_config.portName();
    bool retcode = active_serial_port_p->open(QIODeviceBase::ReadWrite);
    if ( !retcode ) {
        qDebug() << "SerialComms::openSerialPort(): serial port open() failed with"
                 << retcode << active_serial_port_p->errorString();
        return -1;
    }
    clear_serial_port_inbuffer();
    connect(active_serial_port_p, &QIODevice::readyRead, this, &SerialComms::slot_readyRead);

    QThread::msleep(500);
    open_winkeyer();
    return 0;
}

void SerialComms::close_serial_port() {
    qDebug() << "SerialComms::close_serial_port(): closing serial port";
    write_buffer.clear();
    close_winkeyer();
}

void SerialComms::open_winkeyer() {
    qDebug() << "SerialComms::open_winkeyer(): Entered";
    char outc = 0x0;
    write_buffer.append(outc);
    outc = 0x02;
    write_buffer.append(outc);
    display_all_bytes(write_buffer);
    write_serial_data();
    write_buffer.clear();
}

void SerialComms::close_winkeyer() {
    qDebug() << "SerialComms::close_winkeyer(): Entered";
    char outc = 0x0;
    write_buffer.append(outc);
    outc = 0x03;
    write_buffer.append(outc);
    write_serial_data();
    write_buffer.clear();
}

void SerialComms::doEchoTest() {
    char outc = 0x0;
    write_buffer.append(outc);
    outc = 0x04;
    write_buffer.append(outc);
    outc = 65;
    write_buffer.append(outc);
    qDebug() << "SerialComms::doEchoTest(): write_buffer size is now" << write_buffer.size();
    display_all_bytes(write_buffer);
    write_serial_data();
    write_buffer.clear();
    sendEcho = false;
    qDebug() << "SerialComms::doEchoTest(): sendEcho is false" << "write_buffer size after clear" << write_buffer.size();
}

void SerialComms::setConfigSerialStr(QString s) {
    config_serial_str = s;
}

int SerialComms::write_serial_data() {
    int rc;
    char cc = *(write_buffer.data());
    QString str = QString::number(cc);

    qDebug() << "SerialComms::write_serial_data(): size = " << write_buffer.size() << "buffer contains: " << str;
    rc = active_serial_port_p->write(write_buffer);
    display_all_bytes(write_buffer);
    qDebug() << "SerialComms::write_serial_data(): wrote " << rc << "bytes";
    if ( rc == -1 ) {
        qDebug() << "SerialComms::write_serial_data(): write error:" << active_serial_port_p->errorString();
        write_buffer.clear();
        return rc;
    }
    write_buffer.clear();
    return rc;
}

// Slot - called by signal serial_out() in networkcomms to indicate serial data ready to send out
void SerialComms::console_data_2_serial_out(QByteArray &b) {

    qDebug() << "SerialComms::console_data_2_serial_out(): Signal received ***************";
    write_buffer.clear();
    write_buffer = b;
    write_serial_data();
}

// Slot
void SerialComms::slot_readyRead() {
    qDebug() << "SerialComms::slot_readyRead(): Signal received - Slot entered";
    read_serial_data();
}

int SerialComms::read_serial_data() {

    int rc = -2;
    int countReady = 0;
    char rbuff[512];

    qDebug() << "SerialComms::read_serial_data(): Entered";

    if ( (countReady = active_serial_port_p->bytesAvailable()) ) {
        qDebug() << "SerialComms::read_serial_data(): countReady = " << countReady;
        while ( (rc = active_serial_port_p->read(rbuff, countReady)) > 0) {
            qDebug() << "SerialComms::read_serial_data(): read returned" << rc << countReady;
            read_buffer.append(rbuff, countReady);
            display_all_bytes(read_buffer);
        }
    }

    read_buffer.clear();
    return rc;
}

void SerialComms::clear_serial_port_inbuffer() {
    int rc = 0;
    char b[64];
    int count = 0;
    qDebug() << "SerialComms::clear_serial_port_inbuffer(): entered - waiting";

    // This function blocks until readReady - default time out 30 seconds, let's try 1 seconds
    if ( active_serial_port_p->waitForReadyRead(250) == false ) {
        qDebug() << "SerialComms::clear_serial_port_inbuffer(): waitForReadyRead = false";
        return;
    }
    while ( count < 100 ) {
        int i = 0;
        if ( active_serial_port_p->QSerialPort::bytesAvailable() )
            rc += active_serial_port_p->read(&b[i++], 1);
        count++;
        QThread::msleep(2); // Wait a tiny bit for more data
    }
    qDebug() << "SerialComms::clear_serial_port_inbuffer(): read" << rc << "bytes over" << count << "loops" << b;
}

void SerialComms::add_byte(char c) {
    qDebug() << "SerialComms::add_byte(): " << c;
    write_buffer.insert(1, c);
}

void SerialComms::display_all_bytes(QByteArray &r) {

    qDebug() << "SerialComms::display_all_bytes(): Entered with" << r.size() << "bytes";
    QByteArray::iterator iteratorByte;
    int count = 0;
    for (iteratorByte = r.begin(); iteratorByte != r.end() ; iteratorByte++ ) {
        QByteArray test1Byte(1,0); //Define a 1 Byte fixed length variable
        test1Byte[0]= r.at(count++); //Assign each byte of the QByteArray to this variable
        qDebug() << test1Byte.toHex(); //Print that 1 Byte in hex format
    }
}

void SerialComms::slot_channelReadyRead(int channel) {
    qDebug() << "SerialComms::slot_channelReadyRead(): signal received on channel" << channel;
}

void SerialComms::readVCC() {
    char outc = 0x0;
    write_buffer.append(outc);
    outc = 21;
    write_buffer.append(outc);
    qDebug() << "SerialComms::readVCC(): write_buffer size is now" << write_buffer.size();
    display_all_bytes(write_buffer);
    write_serial_data();
    write_buffer.clear();
    qDebug() << "SerialComms::readVCC(): Done";
}
