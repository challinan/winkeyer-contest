#ifndef SQLITE3_CONNECTOR_H
#define SQLITE3_CONNECTOR_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QObject>
#include <QDebug>
#include <QDir>
#include <serialcomms.h>

class Sqlite3_connector : public QObject
{
public:
    Sqlite3_connector();
    ~Sqlite3_connector();
    bool initDatabase();
    bool syncStationData();
    void setSerialPtr(SerialComms *p);

private:
    bool createStationTable();
    int  getRowCount();
    bool dropStationTable();

private:
    const char *db_filename = "winkeyer_db";
    QSqlDatabase db;
    SerialComms *serial_comms_p;

    // Station Dialog
private:
    QString callSign;
    QString gridSquare;
    QString opname;
    QString location;
    QString state;
    QString country;
    QString county;
    QString serial_port;
    QString arrl_section;

public:
    QString getName();
    QString getCallSign();
    QString getGridSquare();
    QString getLocation();
    QString getState();
    QString getCountry();
    QString getCounty();
    QString getSerialPort();
    QString getArrlSection();
    void setName(QString s);
    void setCallSign(QString s);
    void setGridSquare(QString s);
    void setLocation(QString s);
    void setState(QString s);
    void setCountry(QString s);
    void setCounty(QString s);
    void setSerialPort(QString s);
    void setArrlSection(QString s);

};

#endif // SQLITE3_CONNECTOR_H
