#ifndef SQLITE3_CONNECTOR_H
#define SQLITE3_CONNECTOR_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
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
    bool syncStationData_write();
    bool syncStationData_read();
    void setSerialPtr(SerialComms *p);
    QList<QString> get_station_data_table_keys();
    QString get_stataion_data_table_value_by_key(QString key);
    // station_data_list_local_map.insert(s, "");
    void insert_local_station_data_field(QString key, QString value);
    void dump_local_station_data();

private:
    bool createStationTable();
    int  getRowCount();
    bool dropStationTable();

private:
    const char *db_filename = "winkeyer_db";
    QSqlDatabase db;
    SerialComms *serial_comms_p;
    QList<QString> db_station_fields = {"callsign, ", "opname, ", "gridsquare, ", "city, ", "state, ",
                                         "county, ", "country, ", "section, ", "serialport"};

    // This defines the station database table - the one source of truth
    const QMap<QString, int> db_station_fields_tmp = {
        {"callsign", 0},
        {"opname", 1},
        {"gridsquare", 2},
        {"city", 3},
        {"state", 4},
        {"county", 5},
        {"country", 6},
        {"section", 7},
        {"serialport", 8}
    };

    // Station Dialog
private:
    QMap<QString, QString> station_data_list_local_map;
    QString callSign;
    QString opname;
    QString gridSquare;
    QString city;
    QString state;
    QString county;
    QString country;
    QString arrl_section;
    QString serial_port;

public:
    QString getCallSign();
    QString getName();
    QString getGridSquare();
    QString getCity();
    QString getState();
    QString getCounty();
    QString getCountry();
    QString getArrlSection();
    QString getSerialPort();

    void setCallSign(QString s);
    void setName(QString s);
    void setGridSquare(QString s);
    void setCity(QString s);
    void setState(QString s);
    void setCounty(QString s);
    void setCountry(QString s);
    void setArrlSection(QString s);
    void setSerialPort(QString s);

};

#endif // SQLITE3_CONNECTOR_H
