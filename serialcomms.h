#ifndef SERIALCOMMS_H
#define SERIALCOMMS_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QList>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QThread>   // For msleep()

#define MAX_TX_BUFFER_SIZE 64
#define MAX_RX_BUFFER_SIZE 64

class NetworkComms;
class SerialComms;

class SerialComms : public QObject
{
    Q_OBJECT
public:
    explicit SerialComms(QObject *parent = nullptr);
    ~SerialComms();
    QList<QSerialPortInfo> &get_pList();
    void setConfigSerialStr(QString s);
    int openSerialPort();
    int write_serial_data();
    int read_serial_data();
    void setNetcommObjPointer(NetworkComms *pNetworkObj);
    void add_byte(char c);
    void doEchoTest();
    void readVCC();
    QSerialPort *active_serial_port_p;
    void close_serial_port();
    void display_all_bytes(QByteArray &r);

public slots:
    void console_data_2_serial_out(QByteArray &b);
    void slot_readyRead();
    void slot_channelReadyRead(int channel);

private:
    void open_winkeyer();
    void close_winkeyer();
    void clear_serial_port_inbuffer();
    void enumerate_serial_devices();

private:
    QString config_serial_str;
    QSerialPortInfo selected_serial_port_from_config;
    QList<QSerialPortInfo> port_info_list;
    // char write_buffer[MAX_TX_BUFFER_SIZE];
    // char read_buffer[MAX_RX_BUFFER_SIZE];
    QByteArray write_buffer;
    QByteArray read_buffer;
    NetworkComms *network_comm_obj_p;

public:
    bool sendEcho;

signals:
    void on_serial_port_detected(QString &s);
    void serial_rx(QByteArray &p);
    void readyRead();
    void channelReadyRead(int channel);

};


#endif // SERIALCOMMS_H
