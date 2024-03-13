#include <QDebug>
#include <QSqlQuery>

#include "database-tables-base.h"
#include "sqlite3-connector.h"


// ************************************ class DataBaseTableBaseClass ********************* //

DataBaseTableBaseClass::DataBaseTableBaseClass() {
}

DataBaseTableBaseClass::~DataBaseTableBaseClass() {
}

bool DataBaseTableBaseClass::createDbTable() {

    // CREATE command needs to look like this:
    // CREATE TABLE table_name (field1 TEXT, field2 TEXT, field3 TEXT);
    qDebug() << "DataBaseTableBaseClass::createDbTable(): Entered" << this;
    bool rc = false;

    // For each database table class, create the tables in the database (file) itself
    QMapIterator<int, dbfields_values_t> c(getDbFields());
    QString tablename;
    if ( c.hasNext() ) {
        // First element of each  databaase table is the table name in the db file
        c.next();   // Move to next element in Map
        tablename = c.value().fieldname;
    }
    else {
        qDebug() << "DataBaseTableBaseClass::createDbTable(): Unexpected end of database table";
        return false;
    }
    qDebug() << "DataBaseTableBaseClass::createDbTable(): TABLENAME >>>>>>>>>: " << tablename;

    // Here is where we prepare the CREATE statement
    QString cmd = "CREATE TABLE " + tablename + " (";
    while ( c.hasNext() ) {
        c.next();
        // First entry in each table is the tablename
        cmd.append(c.value().fieldname);
        cmd.append(" TEXT");
        if ( c.hasNext() )
            cmd.append(", ");
    }
    cmd.append(");");

    qDebug() << "DataBaseTableBaseClass::createDbTable(): cmd: " << cmd;

    QSqlDatabase sqlDataBase = QSqlDatabase::database("ConfigDB");

    rc = sqlDataBase.open();
    if ( rc ) {
        qDebug() << "DataBaseTableBaseClass::createDbTable(): Database: connection ok, file open";
    }
    else {
        QString err = sqlDataBase.lastError().text();
        qDebug() << "DataBaseTableBaseClass::createDbTable(): Error: connection with database failed" << err;
        return false;
    }

    QSqlQuery q(sqlDataBase);
    rc = q.prepare(cmd);
    if ( !rc ) {
        QString err = q.lastError().text();
        qDebug() << "DataBaseTableBaseClass::createDbTable(): Failed QsqlQuery prepare():" << err;
        sqlDataBase.close();
        return false;
    }

    rc = q.exec();
    if ( rc == false ) {
        qDebug() << "DataBaseTableBaseClass::createDbTable(): Unexpected failure in SQL query exec:" << q.lastError().text();
        sqlDataBase.close();
        return false;
    }

    qDebug() << "DataBaseTableBaseClass::createDbTable(): q.exec() returned" << rc;
    sqlDataBase.close();
    return true;
}

// Force the compiler to instantiate these functions
// template bool DataBaseTableBaseClass::readDbValuesIntoLocalMap_T<StationData>(StationData *);
// template bool DataBaseTableBaseClass::readDbValuesIntoLocalMap_T<SysconfigData>(SysconfigData *);
// template bool DataBaseTableBaseClass::readDbValuesIntoLocalMap_T<ContestData>(ContestData *);

template <typename T>
bool DataBaseTableBaseClass::readDbValuesIntoLocalMap_T(T *pTable) {

    int rc = false;
    qDebug() << "DataBaseTableBaseClass::readDbValuesIntoLocalMap_T(): reading values for" << pTable;

    rc = db->SyncDataTableRead_T(pTable);

    return rc;
}

// ****************** class StationData ********************* //

StationData::StationData(Sqlite3_connector *p)

{
    db = p;
    db->registerTable(new_db_fields, this);

    database_state db_state = db->getDatabaseState("station_data");
    qDebug() << "StationData::StationData(): DB STATE:" << db_state;

    // Check to see if tables already exist
    if ( db_state != DB_HAS_TABLE_DATA ) {
        createDbTable();
    } else {
        readDbValuesIntoLocalMap_T(this);
        qDebug() << "StationData::StationData(): skipping createDbTable() - they already exist" << this;
    }

    // Demo code - how to access QMap(int, struct>
#if 0
    QMapIterator<int, dbfields_values_t> m(new_db_fields);
    while ( m.hasNext() ) {
        m.next();
        dbfields_values_t v = m.value();
        qDebug() << "StationData::StationData():" << m.key() << v.fieldname << v.value << v.field_label << m.value().field_label;
        // This is also valid:
        qDebug() << "StationData::StationData():" << m.key() << m.value().fieldname << m.value().value << m.value().field_label;
    }
#endif
}

const QMap<int, dbfields_values_t> &StationData::getDbFields() {
    return new_db_fields;
}

StationData::~StationData() {

}

// ****************** class SysconfigData ********************* //
SysconfigData::SysconfigData(Sqlite3_connector *p)
//     : public DataBaseTableBaseClass
{
    db = p;
    db->registerTable(new_db_fields, this);

    database_state db_state = db->getDatabaseState("sysconfig_data");
    qDebug() << "SysconfigData::SysconfigData(): DB STATE:" << db_state;

    // Check to see if tables already exist
    if ( db_state != DB_HAS_TABLE_DATA ) {
        createDbTable();
    } else {
        qDebug() << "SysconfigData::SysconfigData(): skipping createDbTable() - they already exist" << this;
        // Read data from external database into local data map
        readDbValuesIntoLocalMap_T(this);

    }
}

const QMap<int, dbfields_values_t> &SysconfigData::getDbFields() {
    qDebug() << "SysconfigData::getDbFields(): Entered";
    return new_db_fields;
}

QString SysconfigData::getConfiguredSerialPort() {
    return local_data_map.value("serialport");
}

SysconfigData::~SysconfigData() {

}

// ****************** class ContestData ********************* //

ContestData::ContestData(Sqlite3_connector *p)
//    : public DataBaseTableBaseClass
{
    db = p;
    db->registerTable(new_db_fields, this);

    database_state db_state = db->getDatabaseState("contest_data");
    qDebug() << "ContestData::ContestData(): DB STATE:" << db_state;

    // Check to see if tables already exist
    if ( db_state != DB_HAS_TABLE_DATA ) {
        createDbTable();
    } else {
        qDebug() << "ContestData::ContestData(): skipping createDbTable() - they already exist" << this;
        // Read data from external database into local data map
        readDbValuesIntoLocalMap_T(this);
    }
}

const QMap<int, dbfields_values_t> &ContestData::getDbFields() {
    qDebug() << "ContestData::getDbFields(): Entered";
    return new_db_fields;
}

ContestData::~ContestData() {

}

// ****************** class ContestConfigData ********************* //

ContestConfigData::ContestConfigData(Sqlite3_connector *p)
//    : public DataBaseTableBaseClass
{
    db = p;
    db->registerTable(new_db_fields, this);

    database_state db_state = db->getDatabaseState("contest_config_data");
    qDebug() << "ContestConfigData::ContestConfigData(): DB STATE:" << db_state;

    // Check to see if tables already exist
    if ( db_state != DB_HAS_TABLE_DATA ) {
        createDbTable();
    } else {
        qDebug() << "ContestConfigData::ContestConfigData(): skipping createDbTable() - they already exist" << this;
        // Read data from external database into local data map
        readDbValuesIntoLocalMap_T(this);
    }
}

const QMap<int, dbfields_values_t> &ContestConfigData::getDbFields() {
    qDebug() << "ContestData::getDbFields(): Entered";
    return new_db_fields;
}

ContestConfigData::~ContestConfigData() {

}
