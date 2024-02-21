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

    // TODO: This can be deleted
    virtual QMap<int, QString> &getLocalDataMap() = 0;

    bool createDbTable();

    template <typename T>
    bool readDbValuesIntoLocalMap_T(T *pTable);

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
    QMap<int, QString> &getLocalDataMap() { return local_data_map; }

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
        {8, {"section", "", "ARRL Section"}},
    };

private:
    // This table holds a copy of what is on disk, and should always be sync'd to it
    QMap<int, QString> local_data_map;

    Sqlite3_connector *pStationData;

};

// ************************************ class SysconfigData ********************* //

class SysconfigData : public DataBaseTableBaseClass {

    Q_OBJECT

public:
    SysconfigData(Sqlite3_connector *p);
    ~SysconfigData();

    const QMap<int, dbfields_values_t> &getDbFields();
    QMap<int, QString> &getLocalDataMap() { return local_data_map; }

    // This defines the system configuration database table layout - the one source of truth
    const QMap<int, dbfields_values_t> new_db_fields {
        {0, {"sysconfig_data", "", "Table Name"}},
        {1, {"serialport", "", "Serial Port"}},
        {2, {"audiooutput", "", "Audio Out Port"}},
        {3, {"audioinput", "", "Audio In Port"}}
    };

private:
    // Temporary local copy of external database
    QMap<int, QString> local_data_map;
    Sqlite3_connector *pSysconfigData;

};

// ************************************ class ContestData ********************* //

class ContestData : public DataBaseTableBaseClass {

    Q_OBJECT

public:
    ContestData(Sqlite3_connector *p);
    ~ContestData();

    const QMap<int, dbfields_values_t> &getDbFields();
    QMap<int, QString> &getLocalDataMap() { return local_data_map; }

    // This defines the system configuration database table layout - the one source of truth
    const QMap<int, dbfields_values_t> new_db_fields {
        {0, {"contest_data", "", "Table Name"}},
        {1, {"sequence", "", "Sequence Num"}},
        {2, {"rst", "", "Signal Report"}}
    };

private:
    QMap<int, QString> local_data_map;
    Sqlite3_connector *pContestData;

};

#endif // DATABASETABLESBASE_H
