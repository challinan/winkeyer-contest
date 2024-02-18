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
    bool initDatabase();

    // Read tables from database file on disk, populate local table maps
    bool syncStationData_read();
    bool syncSysconfigData_read();
    bool syncContestData_read();

    template <typename T>
    bool SyncDataTableRead_T(T *pDataTableClass);

    template <typename T>
    bool syncGeneric_write_to_database_T(T *pDbClass) {

        bool rc;
        qDebug() << "Sqlite3_connector::syncGeneric_write_to_database_T(): Entered" << pDbClass;

        // SQLite command needs to look like this:
        // INSERT INTO station_data (callsign, opname, gridsquare, city, state, county, country, section)
        //  VALUES ("K1AYabc","Chris","EL96av","Punta Gorda","FL","Charlotte","USA","WCF");

        const QMap<int, QString> &pFields = pDbClass->getDbFields();
        QString table_name = pFields.value(0);    // First item in table is the database table name
        qDebug() << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << table_name;

        // Begin by creating a string like this: "INSERT INTO station_data ("
        QString op = "INSERT INTO ";
        op.append(table_name);
        op.append(" (");

        QString fields;

        QMapIterator<int, QString> e(pFields);
        if ( e.hasNext() ) e.next();  // Skip the table label (ie "station_data")

        while ( e.hasNext()) {
            e.next();

            fields.append(e.value());
            if ( e.hasNext() )
                fields.append(", ");
            else
                fields.append(" ");     // Can't allow comma after last field
        }

        op.append(fields + ") ");
        op.append("VALUES (");  // Continue building SQlite command

        // Get local map - from which comes the local copy of the fields data
        QMap<int, QString> &rMap = pDbClass->getLocalDataMap();
        QMapIterator<int, QString> m(rMap);
        while (m.hasNext() ) {
            m.next();

            op.append("\"" + m.value() + "\"");
            // Note we don't add a trailing comma to the last field here
            if ( m.hasNext() )
                op.append(", ");
            else
                op.append(" ");     // Can't allow comma after last field
        }

        op.append(");");

        // qDebug() wants to escape all double quotes in QSring so we do it this way
        qDebug() << "***********>>>>>>>>>>>: " << op.toUtf8().constData();

        QSqlDatabase sqlDataBase = QSqlDatabase::database("ConfigDB");

        rc = sqlDataBase.open();
        if ( rc ) {
            qDebug() << "Sqlite3_connector::syncGeneric_write_to_database_T(): Database: connection ok, file open";
        }
        else {
            QString err = sqlDataBase.lastError().text();
            qDebug() << "Sqlite3_connector::syncGeneric_write_to_database_T(): Error: connection with database failed" << err;
            return false;
        }

        // If QSqlQuery "op" parameter present in this call, it is executed.  See docs for QSqlQuery
        QSqlQuery q(sqlDataBase);
        rc = q.prepare(op);
        if ( !rc ) {
            QString err = q.lastError().text();
            qDebug() << "Sqlite3_connector::syncGeneric_write_to_database_T(): q.prepare() returned" << rc << err;
            return false;
        }

        rc = q.exec();
        if ( !rc ) {
            QString err = q.lastError().text();
            qDebug() << "Sqlite3_connector::syncGeneric_write_to_database_T(): q.exec() returned" << rc << err;
            return false;
        }
        return true;
    }

public:
    void setSerialPtr(SerialComms *p);
    QList<QString> get_xxx_table_keys(QMap<int, QString> map);
    QString get_station_data_table_value_by_key(int key);
    QString get_sysconfig_table_value_by_key(int key);
    QString get_contest_table_value_by_key(int key);

    StationData *getStationDbClassPtr() { return pStationData; }
    SysconfigData *getSysconfigDbClassPtr() { return pSysconfigData; }
    ContestData *getContestDbClassPtr() { return pContestData; }

    void registerTable(const QMap<int, QString> &r);
    inline QList<QMap<int, QString>> &getTableList() {return table_list; };

    void set_station_data_table_value_by_key(int key, QString value);
    void set_sysconfig_table_value_by_key(int key, QString value);
    void set_contest_table_value_by_key(int key, QString value);
    void dump_local_station_data();
    void dump_local_sysconfig_data();
    void dump_local_contest_data();
    int display_message_box(QString text, bool db_init=false);
    enum database_state getDatabaseState();
    void setInitStatus(bool status);
    QString &getDatabasePath();

private:
    int  getRowCount(QString table);
    bool dropStationTable();
    void enumerate_available_serial_ports();
    bool database_initialized;

    // Construct the path name to our external database on local storate (hard drive, etc)
    QString createDatabaseFullPath();
    bool validateDatabasePath();
    bool create_database_path(QString &rpath);

private:
    bool initialization_succeeded;
    StationData *pStationData;
    SysconfigData *pSysconfigData;
    ContestData *pContestData;
    QList<QMap<int, QString>> table_list;
    // QSqlDatabase sqlDataBase;

    // Full path to database on local storage (hard drive, etc)
    QString dbpath;
    const char *db_filename = WINKEYER_DB_NAME;

    // Selects which serial port will be used for Winkeyer
    SerialComms *serial_comms_p;

    // Define the database tables - the one source of truth
    // const QList<QString> database_tables {"station_data", "sysconfig_data", "contest_data"};

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
