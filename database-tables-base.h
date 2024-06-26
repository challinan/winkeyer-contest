#ifndef DATABASETABLESBASE_H
#define DATABASETABLESBASE_H

#include <QObject>
#include <QMap>
#include <QVector>

class Sqlite3_connector;

typedef struct dbfields_values {
    QString fieldname;
    QString value;
    QString field_label;
} dbfields_values_t;


// ************************************ class DataBaseTableBaseClass ********************* //

class DataBaseTableBaseClass : public QObject {

    Q_OBJECT

public:
    explicit DataBaseTableBaseClass();
    ~DataBaseTableBaseClass();
    virtual const QMap<int, dbfields_values_t> &getDbFields() = 0;

    // Local copy of database
    virtual QMap<QString, QString> &getLocalDataMap() = 0;

    bool createDbTable();

    template <typename T>
    bool readDbValuesIntoLocalMap_T(T *pTable);

    template <typename T>
    bool syncLocalMapToDatabase_T(T *pTable);

    Sqlite3_connector *db;

};

// ************************************ class StationData ********************* //

class StationData : public DataBaseTableBaseClass
{

    Q_OBJECT

public:
    StationData(Sqlite3_connector *p);
    ~StationData();

    const QMap<int, dbfields_values_t> &getDbFields();
    QMap<QString, QString> &getLocalDataMap() { return local_data_map; }

private:
    // This defines the database table layout - the one source of truth
    const QMap<int, dbfields_values_t> new_db_fields {
        {0, {"station_data", "", "Table Name"}},
        {1, {"callsign", "", "Call Sign"}},
        {2, {"opname", "", "Name"}},
        {3, {"gridsquare", "", "Grid Square"}},
        {4, {"city", "", "City"}},
        {5, {"state", "", "State"}},
        {6, {"county", "", "County"}},
        {7, {"country", "", "Country"}},
        {8, {"cqzone", "", "CQ Zone"}},
        {9, {"arrlsection", "", "ARRL Section"}},
        {10, {"ituzone", "", "ITU Zone"}}

    };

private:
    // This table holds a copy of what is on disk, and should always be sync'd to it
    QMap<QString, QString> local_data_map;

    Sqlite3_connector *pStationData;

};

// ************************************ class SysconfigData ********************* //

class SysconfigData : public DataBaseTableBaseClass {

    Q_OBJECT

public:
    SysconfigData(Sqlite3_connector *p);
    ~SysconfigData();

    const QMap<int, dbfields_values_t> &getDbFields();
    QMap<QString, QString> &getLocalDataMap() { return local_data_map; }
    QString getConfiguredSerialPort();

    // This defines the system configuration database table layout - the one source of truth
    const QMap<int, dbfields_values_t> new_db_fields {
        {0, {"sysconfig_data", "", "Table Name"}},
        {1, {"serialport", "", "Serial Port"}},
        {2, {"audiooutput", "", "Audio Out Port"}},
        {3, {"audioinput", "", "Audio In Port"}}
    };

private:
    // Temporary local copy of external database
    QMap<QString, QString> local_data_map;
    Sqlite3_connector *pSysconfigData;

};

// ************************************ class ContestData ********************* //

class ContestData : public DataBaseTableBaseClass {

    Q_OBJECT

public:
    ContestData(Sqlite3_connector *p);
    ~ContestData();

    const QMap<int, dbfields_values_t> &getDbFields();
    QMap<QString, QString> &getLocalDataMap() { return local_data_map; }

    // This defines the system configuration database table layout - the one source of truth
    const QMap<int, dbfields_values_t> new_db_fields {
        {0, {"contest_data", "", "Table Name"}},
        {1, {"sequence", "", "Sequence Num"}},
        {2, {"rst", "", "Signal Report"}}
    };

private:
    QMap<QString, QString> local_data_map;
    Sqlite3_connector *pContestData;

};

// ************************************ class ContestConfigData ********************* //
class ContestConfigData : public DataBaseTableBaseClass {

    Q_OBJECT

public:
    ContestConfigData(Sqlite3_connector *p);
    ~ContestConfigData();

    const QMap<int, dbfields_values_t> &getDbFields();
    QMap<QString, QString> &getLocalDataMap() { return local_data_map; }

    // This defines the system configuration database table layout - the one source of truth
    QMap<int, dbfields_values_t> new_db_fields {
        {0, {"contest_config_data", "", "Table Name"}},
        {1, {"current_contest_name", "", "Cabrillo Name"}},
        {2, {"exchange", "", "Exchange"}},
        {3, {"esm_mode", "", "ESM Mode"}},
        {4, {"run_mode", "", "Run Mode"}},
        {5, {"cfgsection", "", "Section"}},
        {6, {"sent", "", "RST Sent"}},
        {7, {"rcvd", "", "RST Rcvd"}}
    };

private:
    QMap<QString, QString> local_data_map;
    Sqlite3_connector *pContestData;

};

#endif // DATABASETABLESBASE_H
