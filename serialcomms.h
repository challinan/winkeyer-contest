#ifndef SERIALCOMMS_H
#define SERIALCOMMS_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QList>
#include <QMutex>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QThread>   // For msleep()

#define MAX_TX_BUFFER_SIZE 64
#define MAX_RX_BUFFER_SIZE 64
#define SPEEDPOT_MIN    0x0a   // 10
#define SPEEDPOT_RANGE  0x28   // 40

class NetworkComms;
class SerialComms;
class Sqlite3_connector;

#define WINKEYER_XOFF       0x01
#define WINKEYER_BREAKIN    0x02
#define WINKEYER_BUSY       0x04
#define WINKEYER_KEYDOWN    0x08
#define WINKEYER_WAIT       0x10
#define WINKEYER_STATUS     0xc0

class SerialComms : public QObject
{
    Q_OBJECT
public:
    explicit SerialComms(QObject *parent = nullptr, Sqlite3_connector *p = nullptr);
    ~SerialComms();
    QList<QSerialPortInfo> &get_pList();
    bool openSerialPort();
    int writeSerialData();
    int readSerialData();
    void setNetcommObjPointer(NetworkComms *pNetworkObj);
    void add_byte(char c);
    void doEchoTest();
    void readVCC();
    void close_serial_port();
    void display_all_bytes(QByteArray &r);
    void setSpeed(int);
    int getSpeed();
    void setupSpeedPotRange(uchar min, uchar range);
    void clearWinkeyerBuffer();

public slots:
    void slot_readyRead();
    void processStatusByte(uchar status);
    void processSpeedPot(uchar speed);
    void slotSendWinkeyerSpeed();
    void processTxChar(char c);
    void simTimerTimeout();
    void processRxMessage(QByteArray &b);

private:
    void open_winkeyer();
    void close_winkeyer();
    void reset_winkeyer();

private:
    QString config_serial_port;
    QSerialPortInfo selected_serial_port_from_config;
    // char write_buffer[MAX_TX_BUFFER_SIZE];
    // char read_buffer[MAX_RX_BUFFER_SIZE];
    QByteArray write_buffer;
    QSerialPort *active_serial_port_p;
    QMetaObject::Connection c_statusByteConnx;
    QMetaObject::Connection c_speedPotRxConnx;
    QMetaObject::Connection c_speedpot_timerConnx;
    QMetaObject::Connection c_sim_timerConnx;
    bool speedpot_timer_fired;
    QTimer *speedPotTimer;
    QTimer *txSimTimer;
    uchar currentSpeed;
    bool simTimerRunning;
    QString version_string;

    // Indicate whether Winkeyer can accept input commands
    bool winkeyer_open;

    // Pointer to database object
    Sqlite3_connector *db;

public:
    QByteArray read_buffer;
    QMutex scomm_mutex;

signals:
    void on_serial_port_detected(QString &s);
    void serial_rx(QByteArray &p);
    void readyRead();
    void statusByteReceived(uchar status);
    void speedPotValueReceived(uchar speed);
    void TxCharComplete();

};

#endif // SERIALCOMMS_H
