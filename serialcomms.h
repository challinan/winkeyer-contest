#ifndef SERIALCOMMS_H
#define SERIALCOMMS_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QThread>
#include <QList>
#include <QMutex>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QThread>   // For msleep()

#define MAX_TX_BUFFER_SIZE 64
#define MAX_RX_BUFFER_SIZE 64
#define SPEEDPOT_MIN 0x0a   // 10
#define SPEEDPOT_MAX 0x2a   // 42

class NetworkComms;
class SerialComms;
class Sqlite3_connector;
class SerialRxCommsThread;

class SerialComms : public QObject
{
    Q_OBJECT
public:
    explicit SerialComms(QObject *parent = nullptr, Sqlite3_connector *p = nullptr);
    ~SerialComms();
    QList<QSerialPortInfo> &get_pList();
    bool openSerialPort();
    int WriteSerialData();
    int readserialdata();
    void setNetcommObjPointer(NetworkComms *pNetworkObj);
    void add_byte(char c);
    void doEchoTest();
    void readVCC();
    void close_serial_port();
    void display_all_bytes(QByteArray &r);
    void setSpeed(int);
    void setupSpeedPotRange(uchar min, uchar max);
    void clearWinkeyerBuffer();

public slots:
    void console_data_2_serial_out(QByteArray &b);
    void slot_readyRead();

private:
    void open_winkeyer();
    void close_winkeyer();

private:
    QString config_serial_port;
    QSerialPortInfo selected_serial_port_from_config;
    // char write_buffer[MAX_TX_BUFFER_SIZE];
    // char read_buffer[MAX_RX_BUFFER_SIZE];
    QByteArray write_buffer;
    QSerialPort *active_serial_port_p;
    SerialRxCommsThread *pRxThread;

    // Indicate whether Winkeyer can accept input commands
    bool winkeyer_open;

    // Pointer to database object
    Sqlite3_connector *db;

public:
    QByteArray read_buffer;
    QMutex scomm_mutex;
    bool runRxThread;

signals:
    void on_serial_port_detected(QString &s);
    void serial_rx(QByteArray &p);
    void readyRead();
    void channelReadyRead(int channel);

};


class SerialRxCommsThread : public QThread
{
    Q_OBJECT

public:
    void run() override;
    void setSerialCommsPtr(SerialComms *p);

private:
    SerialComms *pSerialComm;

signals:
    void serialRxReady(int i);
};

#endif // SERIALCOMMS_H
