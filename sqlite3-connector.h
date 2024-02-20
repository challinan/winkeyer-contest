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
#include "database-tables-base.h"
#include <serialcomms.h>

#define WINKEYER_DB_NAME "winkeyer_db"

// We want to know, sequentially: does the file exist?  Does it have tables?  Do the tables have data?
// DB_NOEXIST means no file on disk
// DB_NOTABLES means file has been created, but is empty (no tables)
// DB_HAS_TABLE_DATA means there are at least one row in a table
// DB_INVALID and DB_ERROR are states that exist in the underlying infrastructure/engine
enum  database_state {DB_NOEXIST, DB_NOTABLES, DB_HAS_TABLE_DATA, DB_INVALID, DB_ERROR};

#define HIDDEN_WORKDIR ".macrr"

// Forward declarations
class DataBaseTableBaseClass;
class StationData;
class SysconfigData;
class ContestData;


class Sqlite3_connector : public QObject
{
    Q_OBJECT

public:
    Sqlite3_connector();
    ~Sqlite3_connector();

    // Read tables from database file on disk, populate local table maps
    bool syncStationData_read();
    bool syncSysconfigData_read();
    bool syncContestData_read();

    template <typename T>
    bool SyncDataTableRead_T(T *pDataTableClass);

    // Save data from Database class' local data map to database on local storage (hard disk, etc)
    template <typename T>
    bool syncGeneric_write_to_database_T(T *pDbClass);

    // Continue initialization after connect() signals/slots have been made
    void initContinue();

public:
    void setSerialPtr(SerialComms *p);
    QList<QString> get_xxx_table_keys(QMap<int, QString> map);
    QString get_station_data_table_value_by_key(int key);
    QString get_sysconfig_table_value_by_key(int key);
    QString get_contest_table_value_by_key(int key);
    bool checkIfDatabaseTablesAreEmpty();

    StationData *getStationDbClassPtr() { return pStationData; }
    SysconfigData *getSysconfigDbClassPtr() { return pSysconfigData; }
    ContestData *getContestDbClassPtr() { return pContestData; }

    void registerTable(const QMap<int, dbfields_values_t> &r);
    inline QList<QMap<int, QString>> &getTableList() {return table_list; };
    QList<QString> GetTableNameList();

    void set_station_data_table_value_by_key(int key, QString value);
    void set_sysconfig_table_value_by_key(int key, QString value);
    void set_contest_table_value_by_key(int key, QString value);
    void dump_local_station_data();
    void dump_local_sysconfig_data();
    void dump_local_contest_data();
    int display_message_box(QString text, bool db_init=false);
    enum database_state getDatabaseState(QString tableName);
    QString &getDatabasePath();

private:
    int  getRowCount(QString table);
    bool dropStationTable();
    void enumerate_available_serial_ports();

    // Construct the path name to our external database on local storate (hard drive, etc)
    QString createDatabaseFullPath();
    bool validateDatabasePath();
    bool create_database_path(QString &rpath);

private:
    StationData *pStationData;
    SysconfigData *pSysconfigData;
    ContestData *pContestData;
    QList<QMap<int, QString>> table_list;

    // Full path to database on local storage (hard drive, etc)
    QString dbpath;
    const char *db_filename = WINKEYER_DB_NAME;

    // Selects which serial port will be used for Winkeyer
    SerialComms *serial_comms_p;

public:
    QList<QSerialPortInfo> serial_port_list;

public:
    const QMap<QString, QString> text_labels_for_sysconfig_keys = {
        {"serialport", "Serial Port"},
        {"audiooutput", "Audio Output Device"},
        {"audioinput", "Audio Input Device"}
    };

    const QMap<QString, QString> text_labels_for_contest_keys = {
        {"sequence", "Sequence Number"},
        {"section", "ARRL Section"},
        {"rst", "Signal Report (RST)"}
    };

signals:
    void do_config_dialog();

};

#endif // SQLITE3_CONNECTOR_H
