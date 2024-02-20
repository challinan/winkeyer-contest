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
    virtual const QMap<int, QString> &getDbFields() = 0;
    virtual const QMap<QString, QString> &getTextLabelFields() = 0;
    virtual void createSingleDbTable();
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

    const QMap<int, QString> &getDbFields();
    const QMap<QString, QString> &getTextLabelFields();
    void createSingleDbTable();
    QMap<int, QString> &getLocalDataMap() { return local_data_map; }

private:

    const QMap<int, dbfields_values_t> new_db_fields {
        {0, {"callsign", "", "Call Sign"}},
        {1, {"opname", "", "Name"}},
        {2, {"gridsquare", "", "Grid Square"}},
        {3, {"city", "", "City"}},
        {4, {"state", "", "State"}},
        {5, {"county", "", "County"}},
        {6, {"country", "", "Country"}},
        {7, {"section", "", "ARRL Section"}},
    };

    // This defines the database table layout - the one source of truth
    const QMap<int, QString> database_fields = {
        // First field is the name of the database table
        {0, "station_data"},
        {1, "callsign"},
        {2, "opname"},
        {3, "gridsquare"},
        {4, "city"},
        {5, "state"},
        {6, "county"},
        {7, "country"},
        {8, "section"}
    };

    const QMap<QString, QString> text_labels = {
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

    const QMap<int, QString> &getDbFields();
    const QMap<QString, QString> &getTextLabelFields();
    void createSingleDbTable();
    QMap<int, QString> &getLocalDataMap() { return local_data_map; }

    // This defines the system configuration database table layout - the one source of truth
    const QMap<int, QString> database_fields = {
        // First field is the name of the database table
        {0, "sysconfig_data"},
        {1, "serialport"},
        {2, "audiooutput"},
        {3, "audioinput"}
    };

    const QMap<QString, QString> text_labels = {
        {"serialport", "Serial Port"},
        {"audiooutput", "Audio Out Port"},
        {"audioinput", "Audio In Port"}
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

    const QMap<int, QString> &getDbFields();
    const QMap<QString, QString> &getTextLabelFields();
    void createSingleDbTable();
    QMap<int, QString> &getLocalDataMap() { return local_data_map; }

    const QMap<int, QString> database_fields = {
        // First field is the name of the database table
        {0, "contest_data"},
        {1, "sequence"},
        {2, "rst"}
    };

    const QMap<QString, QString> text_labels = {
        {"sequence", "Sequence Num"},
        {"section", "ARRL Section"},
        {"rst", "Signal Report"}
    };

private:
    QMap<int, QString> local_data_map;
    Sqlite3_connector *pContestData;

};

#endif // DATABASETABLESBASE_H
