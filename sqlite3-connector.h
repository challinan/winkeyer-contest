#ifndef SQLITE3_CONNECTOR_H
#define SQLITE3_CONNECTOR_H

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QObject>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <serialcomms.h>

enum  database_state {DB_NOEXIST, DB_EMPTY, DB_HAS_STATIONTABLE, DB_HAS_MULTITABLES, DB_INVALID, DB_ERROR};
#define HIDDEN_WORKDIR ".macrr"

class Sqlite3_connector : public QObject
{
    Q_OBJECT

public:
    Sqlite3_connector();
    ~Sqlite3_connector();
    bool initDatabase();
    bool syncStationData_write();
    bool syncStationData_read();
    bool syncSysconfigData_write();
    bool syncSysconfigData_read();
    void setSerialPtr(SerialComms *p);
    QList<QString> get_station_data_table_keys();
    QList<QString> get_sysconfig_table_keys();
    QString get_station_data_table_value_by_key(QString key);
    QString get_sysconfig_table_value_by_key(QString key);
    void set_station_data_table_value_by_key(QString key, QString value);
    void set_sysconfig_table_value_by_key(QString key, QString value);
    void dump_local_station_data();
    void dump_local_sysconfig_data();
    int display_message_box(QString text, bool db_init=false);
    enum database_state getDatabaseState();
    bool dbInitSucceeded();
    void setInitStatus(bool status);

private:
    bool createStationTable();
    bool createSysconfigTable();
    int  getRowCount();
    bool dropStationTable();
    void enumerate_available_serial_ports();
    bool database_initialized;
    void getDatabaseFullPath();
    bool validateDatabaseFullPath();
    bool create_database_path();
    bool initialization_succeeded;

private:
    const char *db_filename = "winkeyer_db";
    QSqlDatabase db;
    QString dbpath;
    SerialComms *serial_comms_p;

public:
    QList<QSerialPortInfo> serial_port_list;

private:
    // This defines the station database table layout - the one source of truth
    const QMap<int, QString> db_station_fields = {
        {0, "callsign"},
        {1, "opname"},
        {2, "gridsquare"},
        {3, "city"},
        {4, "state"},
        {5, "county"},
        {6, "country"},
        {7, "section"}
        // {8, "serialport"}
    };

    // This defines the system database table layout - the one source of truth
    const QMap<int, QString>db_sysconfig_fields = {
        {0, "serialport"},
        {1, "audiooutput"},
        {2, "audioinput"}
    };

public:
    const QMap<QString, QString> text_labels_for_station_data_keys = {
        {"callsign", "Call Sign"},
        {"opname", "Name"},
        {"gridsquare", "Grid Square"},
        {"city", "City"},
        {"state", "State"},
        {"county", "County"},
        {"country", "Country"},
        {"section", "Arrl Section"},
        {"serialport", "Serial Port"}
    };

    const QMap<QString, QString> text_labels_for_sysconfig_keys = {
    {"serialport", "Serial Port"},
    {"audiooutput", "Audio Output Device"},
    {"audioinput", "Audio Input Device"}
    };

private:
    // Station Dialog
    QMap<QString, QString> station_data_list_local_map;
    QMap<QString, QString> sysconfig_data_list_local_map;

signals:
    void do_config_dialog();

};

#endif // SQLITE3_CONNECTOR_H
