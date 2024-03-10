#ifndef SQLITE3_CONNECTOR_H
#define SQLITE3_CONNECTOR_H

#include <QApplication>
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

    // Read the database, and put the data into the local map (our local mirror of what's in the table on disk)
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
    bool checkIfDatabaseTablesAreEmpty();

    StationData *getStationDbClassPtr() { return pStationData; }
    SysconfigData *getSysconfigDbClassPtr() { return pSysconfigData; }
    ContestData *getContestDbClassPtr() { return pContestData; }
    ContestConfigData *getContestConfigDbClassPtr() { return pContestConfigData; }

    // void registerTable(const QMap<int, QString> &r);
    template <typename T>
    void registerTable(const  QMap<int, dbfields_values_t> &r, T *p);
    inline QList<QMap<int, dbfields_values_t>> &getTableList() {return table_list; };
    QList<QString> GetTableNameList();

    void dump_local_station_data();
    void dump_local_sysconfig_data();
    void dump_local_contest_data();
    void dump_local_contest_config_data();

    int display_message_box(QString text, bool db_init=false);
    enum database_state getDatabaseState(QString tableName);
    QString &getDatabasePath();

private:
    int  getRowCount(QString table);
    bool dropStationTable();

    // Construct the path name to our external database on local storate (hard drive, etc)
    QString createDatabaseFullPath();
    bool validateDatabasePath();

private:
    StationData *pStationData;
    SysconfigData *pSysconfigData;
    ContestData *pContestData;
    ContestConfigData *pContestConfigData;

    QList<QMap<int, dbfields_values_t>> table_list;

    // Full path to database on local storage (hard drive, etc)
    QString dbpath;
    const char *db_filename = WINKEYER_DB_NAME;

    // This is a pointer to our serialComms object.  Do we need it here?
    SerialComms *serial_comms_p;

signals:
    void do_config_dialog();

};

#endif // SQLITE3_CONNECTOR_H
