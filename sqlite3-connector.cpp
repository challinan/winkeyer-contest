#include <QPushButton>
#include <QString>

#include "sqlite3-connector.h"

#define USER_PRESSED_CONFIG 1
#define USER_PRESSED_ABORT 2
#define FETCH_LAST

Sqlite3_connector::Sqlite3_connector() {

    qDebug() << "Sqlite3_connector::Sqlite3_connector(): Entered";

    // Assemble the path to the on-disk database file
    dbpath = createDatabaseFullPath();

    qDebug() << "Sqlite3_connector::Sqlite3_connector(): Initializing QSqlDatabase connection";
    QSqlDatabase sqlDataBase = QSqlDatabase::addDatabase("QSQLITE", "ConfigDB");
    sqlDataBase.setDatabaseName(dbpath);
    if ( !sqlDataBase.isValid() ) {
        qDebug() << "Sqlite3_connector::Sqlite3_connector(): Error: isValid returned false";
        return;
    }
}

void Sqlite3_connector::initContinue() {

    qDebug() << "Sqlite3_connector::Sqlite3_connector(): Instantiating DataBase Table class objects";
    pStationData = new StationData(this);
    pSysconfigData = new SysconfigData(this);
    pContestData = new ContestData(this);

    checkIfDatabaseTablesAreEmpty();
}

Sqlite3_connector::~Sqlite3_connector() {

    QSqlDatabase::removeDatabase("ConfigDB");
    delete pContestData;
    delete pSysconfigData;
    delete pStationData;
}

QString Sqlite3_connector::createDatabaseFullPath() {

    // Get user's home directory

    QString userHomeDir;
    userHomeDir = QDir::homePath();
    QString fullPath = userHomeDir;
    fullPath.append("/");
    fullPath.append(HIDDEN_WORKDIR);
    fullPath.append("/");
    fullPath.append(db_filename);
    return fullPath;
}

QString &Sqlite3_connector::getDatabasePath() {
    return dbpath;
}

bool Sqlite3_connector::validateDatabasePath() {

    // Isolate path name to db file
    int slash_index;
    slash_index = dbpath.lastIndexOf("/", -1);
    if ( slash_index == -1 ) {
        qDebug() << "Sqlite3_connector::validateDatabasePath(): Error searching for / in str" << dbpath;
        return false;
    }

    QString s = dbpath;
    s.truncate(slash_index);
    qDebug() << "Sqlite3_connector::validateDatabasePath(): path is" << s;

    // See if the path exists
    QFile file (s);

    if ( !file.exists() ) {
        qDebug() << "Sqlite3_connector::validateDatabasePath(): File Not Found";
        return false;
    }

    return true;
}

QList<QString> Sqlite3_connector::GetTableNameList() {


    // Build a simple list of QStrings containing our table names
    QList<QString> items;

    QList<QMap<int, dbfields_values_t>> &table_list = getTableList();
    QListIterator m(table_list);
    while ( m.hasNext() ) {
        // Isolate the fieldname - first entry in each database table
        QString st = m.next().value(0).fieldname;
        items.append(st);
        // qDebug() << "Table value = " << st;
    }
    qDebug() << "Sqlite3_connector::GetTableNameList()" << items;
    return items;
}

bool Sqlite3_connector::checkIfDatabaseTablesAreEmpty() {

    bool rc;
    int rc_int;

    QSqlDatabase sqlDataBase = QSqlDatabase::database("ConfigDB");
    rc = sqlDataBase.open();
    if ( rc ) {
        qDebug() << "Sqlite3_connector::checkIfDatabaseTablesAreEmpty(): Database: connection ok, file open";
    }
    else {
        QString err = sqlDataBase.lastError().text();
        qDebug() << "Sqlite3_connector::checkIfDatabaseTablesAreEmpty(): Error: opening database failed" << err;
        return false;
    }

    QStringList db_tables = sqlDataBase.tables(QSql::Tables);
    bool database_needs_config = false;
    // If we have tables, check if any are empty and trigger config it so
    if ( !db_tables.isEmpty() ) {
        QListIterator<QString> m(db_tables);
        while ( m.hasNext() ) {
            QString tbl_name = m.next();
            if ( getRowCount(tbl_name) == 0 ) {
                database_needs_config = true;
                break;
            }
        }
    } else {
            // Database has no tables, trigger config
            database_needs_config = true;
    }


    if ( database_needs_config ) {
        qDebug() <<  "DATABASE HAS NO TABLES OR SOME TABLES EMPTY - INITIALIZATION IS REQUIRED";
        rc_int = display_message_box("Database requires initialization - please configure station data", true);
        if ( rc_int != USER_PRESSED_CONFIG ) {
            qDebug() << "Sqlite3_connector::checkIfDatabaseTablesAreEmpty(): User pressed Abort";
            sqlDataBase.close();
            return false;
        }
    }

    sqlDataBase.close();
    return true;
}

// Force the compiler to instantiate these three specific funcions to avoid link errors
template bool Sqlite3_connector::syncGeneric_write_to_database_T(StationData *pDbClass);
template bool Sqlite3_connector::syncGeneric_write_to_database_T(SysconfigData *pDbClass);
template bool Sqlite3_connector::syncGeneric_write_to_database_T(ContestData *pDbClass);

template <typename T>
bool Sqlite3_connector::syncGeneric_write_to_database_T(T *pDbClass) {

    bool rc;
    qDebug() << "Sqlite3_connector::syncGeneric_write_to_database_T(): Entered" << pDbClass;

    // SQLite command needs to look like this:
    // INSERT INTO station_data (callsign, opname, gridsquare, city, state, county, country, section)
    //  VALUES ("K1AYabc","Chris","EL96av","Punta Gorda","FL","Charlotte","USA","WCF");

    const QMap<int, dbfields_values_t> &pFields = pDbClass->getDbFields();
    QString table_name = pFields.value(0).fieldname;    // First item in table is the database table name
    // qDebug() << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << table_name;

    // Begin by creating a string like this: "INSERT INTO station_data ("
    QString op = "INSERT INTO ";
    op.append(table_name);
    op.append(" (");

    // This QList will hold the keys in order of their appearance
    QList<QString> database_field_keys;
    QString fieldsTmp;

    QMapIterator<int, dbfields_values_t> e(pFields);
    if ( e.hasNext() ) e.next();  // Skip the table label (ie "station_data")

    while ( e.hasNext()) {
        e.next();

        // TODO Looks like fieldsTmp and database_field_keys are redundant
        database_field_keys.append(e.value().fieldname);
        fieldsTmp.append(e.value().fieldname);
        if ( e.hasNext() )
            fieldsTmp.append(", ");
        else
            fieldsTmp.append(" ");     // Can't allow comma after last field
    }

    op.append(fieldsTmp + ") ");
    op.append("VALUES (");  // Continue building SQlite command

    // Get local map - from which comes the local copy of the database fields
    QMap<QString, QString> &rMap = pDbClass->getLocalDataMap();

    e.toFront(); // Reset the database fields iterator
    if ( e.hasNext() ) e.next();  // Skip the table label (ie "station_data")

    while (e.hasNext() ) {
        QString s = e.next().value().fieldname;

        op.append("\"" + rMap.value(s) + "\"");
        // Note we don't add a trailing comma to the last field here
        if ( e.hasNext() )
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

// Force the compiler to instantiate these templated functions to avoid putting the def's in header file
template bool Sqlite3_connector::SyncDataTableRead_T<StationData>(StationData *);
template bool Sqlite3_connector::SyncDataTableRead_T<SysconfigData>(SysconfigData *);
template bool Sqlite3_connector::SyncDataTableRead_T<ContestData>(ContestData *);

template <typename T>
bool Sqlite3_connector::SyncDataTableRead_T(T *pDataTableClass) {

    bool rc = false;

    qDebug() << "Sqlite3_connector::SyncDataTableRead_T: Entered:" << pDataTableClass;
    const QMap<int, dbfields_values_t> &pDbFields = pDataTableClass->getDbFields();
    QMapIterator<int, dbfields_values_t> dbFieldsIter(pDbFields);

    // First entry of each database table is the table's name
    QString tablename = pDbFields.value(0).fieldname;

#ifdef FETCH_LAST
    // Fetch the last row from the station table
    QString op = "SELECT * FROM ";
    op.append(tablename);
    op.append(" ORDER BY rowid DESC LIMIT 1;");

#else
    // Construct the query
    QString op = "SELECT ";
    while (i.hasNext() ) {
        i.next();
        op.append(i.value());
        if ( i.hasNext() )
            op.append(", ");
        else
            op.append(" ");     // Can't allow comma after last field
    }
    op.append("FROM station_data;");
    qDebug() << op;
#endif

    // qDebug() << "Sqlite3_connector::SyncDataTableRead_T(): CMD" << op;
    QSqlDatabase sqlDataBase = QSqlDatabase::database("ConfigDB");

    if ( !sqlDataBase.isValid() ) {
        qDebug() << "Sqlite3_connector::SyncDataTableRead_T(): Error: isValid returned false";
        return false;
    }

    QSqlQuery q(sqlDataBase);
    rc = q.prepare(op);
    if ( !rc ) {
        QString err = q.lastError().text();
        qDebug() << "Sqlite3_connector::SyncDataTableRead_T(): Failed QsqlQuery prepare():" << err;
        sqlDataBase.close();
        return false;
    }

    rc = q.exec();
    if ( !rc ) {
        QString err = q.lastError().text();
        qDebug() << "Sqlite3_connector::SyncDataTableRead_T(: Failed sql query exec():" << err;
        sqlDataBase.close();
        return false;
    }

    // Query results are invalid unless isActive() and isSelect() are both true
    if ( !q.isActive() || !q.isSelect() ) {
        qDebug() << "Sqlite3_connector::SyncDataTableRead_T(): Database query failed - invalid reading station_data table";
        return false;
    }

    // qDebug() << "Sqlite3_connector::SyncDataTableRead_T(): SQL Query:" << rc;

    QMap<QString, QString> &lMap = pDataTableClass->getLocalDataMap();

    dbFieldsIter.toFront();    // Reposition db fields iterator to front of list
    dbFieldsIter.next(); // Skip over the table name
    while ( q.next() ) {
        while (dbFieldsIter.hasNext() ) {
            dbFieldsIter.next();
            QString db_table_fieldname = dbFieldsIter.value().fieldname;
            // Populate our local copy of station data
            qDebug() << "Sqlite3_connector::SyncDataTableRead_T(): db_table_fieldname:" << db_table_fieldname << "value:"
                     << q.value(db_table_fieldname).toString();
            lMap.insert(db_table_fieldname, q.value(db_table_fieldname).toString());
        }
    }
    return rc;
}


int  Sqlite3_connector::getRowCount(QString table) {

    int rowcount = 0;
    bool rc;

    qDebug() << "Sqlite3_connector::getRowCount(): Entered" << table;
    QSqlDatabase sqlDataBase = QSqlDatabase::database("ConfigDB");

    rc = sqlDataBase.open();
    if ( rc ) {
        // qDebug() << "Sqlite3_connector::getRowCount(): Database: connection ok, file open";
    }
    else {
        qDebug() << "Sqlite3_connector::getRowCount(): Error: connection with database failed";
        display_message_box("Connection with database failed");
        return false;
    }

    QString op = "SELECT Count(*) FROM ";

    op.append(table + ";");
    // qDebug() << "Sqlite3_connector::getRowCount(): op:" << op;

    QSqlQuery q(sqlDataBase);
    rc = q.prepare(op);
    if ( !rc ) {
        QString err = q.lastError().text();
        qDebug() << "Sqlite3_connector::getRowCount(): Failed QsqlQuery prepare():" << err;
        sqlDataBase.close();
        return false;
    }

    rc = q.exec();
    if ( !rc ) {
        QString err = q.lastError().text();
        qDebug() << "Sqlite3_connector::getRowCount(): Failed sql query exec():" << err;
        sqlDataBase.close();
        return false;
    }

    if ( q.isActive() && q.isSelect() ) {
        while ( q.next() ) {
            // qDebug() << "Sqlite3_connector::getRowCount():" << q.value(0).toInt();
            rowcount = q.value(0).toInt();
        }
    } else {
        display_message_box("Database query failed while iterating - isActive or isSelect failed");
        sqlDataBase.close();
        return false;
    }

    // qDebug() << "Sqlite3_connector::getRowCount(): There are" << rowcount << "rows in" << table;

    sqlDataBase.close();
    return rowcount;
}

bool Sqlite3_connector::dropStationTable() {

    bool rc;

    QSqlDatabase sqlDataBase = QSqlDatabase::database("QSQLITE");

    QString op = "DROP TABLE IF EXISTS station_data;";

    QSqlQuery q(sqlDataBase);
    rc = q.prepare(op);
    if ( !rc ) {
        QString err = q.lastError().text();
        qDebug() << "Sqlite3_connector::dropStationTable(): Failed QsqlQuery prepare():" << err;
        sqlDataBase.close();
        return false;
    }

    rc = q.exec();
    qDebug() << "Sqlite3_connector::dropStationTable(): q.exec returned" << rc;
    if ( !rc ) {
        display_message_box("SQL query failed dropping station table");
        return false;
    }

    return true;
}

QList<QString> Sqlite3_connector::get_xxx_table_keys(QMap<int, QString> map) {

    QList<QString> s;

    QMapIterator<int, QString> m(map);
    while (m.hasNext() ) {
        m.next();
        qDebug() << "QList<QString> Sqlite3_connector::get_xxx_table_keys()" << m.value();
        s.append(m.value());
    }
    return s;
}

void Sqlite3_connector::dump_local_station_data() {

    qDebug() << "Dumping local station data table";

    QMap<QString, QString> &rMap = pStationData->getLocalDataMap();
    if ( rMap.isEmpty() ) {
        qDebug() << "Sqlite3_connector::dump_local_station_data(): Local Station Map empty";
        return;
    }

    QMapIterator<QString, QString> m(rMap);
    while (m.hasNext() ) {
        m.next();
        qDebug() << m.key() << ":" << m.value();
    }
}

void Sqlite3_connector::dump_local_sysconfig_data() {

    qDebug() << "Dumping local system config data table";
    QMap<QString, QString> &rMap = pSysconfigData->getLocalDataMap();
    QMapIterator<QString, QString> m(rMap);
    while (m.hasNext() ) {
        m.next();
        qDebug() << m.key() << ":" << m.value();
    }
}

void Sqlite3_connector::dump_local_contest_data() {

    qDebug() << "Dumping local contest data table";
    QMap<QString, QString> &rMap = pContestData->getLocalDataMap();
    QMapIterator<QString, QString> m(rMap);
    while (m.hasNext() ) {
        m.next();
        qDebug() << m.key() << ":" << m.value();
    }
}

int Sqlite3_connector::display_message_box(QString text, bool db_init) {

    qDebug() << "Sqlite3_connector::display_message_box(): Entered with db_init =" << db_init << "about to display message box********";
    QMessageBox msgBox(QMessageBox::Warning, "Warning", text,
                       QMessageBox::Abort);

    QAbstractButton *config_button = nullptr;
    QAbstractButton *ok_button = nullptr;
    if ( db_init )
        config_button = msgBox.addButton("Configure", QMessageBox::AcceptRole);
    else
        ok_button = msgBox.addButton("OK", QMessageBox::AcceptRole);

    msgBox.setText(text);
    msgBox.exec();
    if ( (db_init && msgBox.clickedButton() == config_button) ) {
        qDebug() << "Sqlite3_connector::display_message_box(): User pressed Config";
        emit do_config_dialog();
        return USER_PRESSED_CONFIG;
    }
    else
        if ( msgBox.clickedButton() == ok_button )
            return USER_PRESSED_CONFIG;
    else
        return USER_PRESSED_ABORT;

    return 0;
}

enum database_state Sqlite3_connector::getDatabaseState(QString tableName) {

    // {DB_NOEXIST, DB_NOTABLES, DB_HAS_TABLE_DATA, DB_INVALID, DB_ERROR}
    // We want to know, sequentially: does the file exist?  Does it have tables?  Do the tables have data?
    database_state state = DB_NOEXIST;
    int count = 0;

    QString &dbFile = getDatabasePath();
    QFile file (dbFile);

    if ( !file.exists() ) {
        qDebug() << "Sqlite3_connector::getDatabaseState(): DB_NOEXIST";
        return DB_NOEXIST;
    }

    QSqlDatabase sqlDataBase = QSqlDatabase::database("ConfigDB");
    QStringList db_tables;
    if ( sqlDataBase.isValid() ) {
        if ( sqlDataBase.isOpen() ) {
            // Check for empty database - ie file exists but it's got nothing in it
            db_tables = sqlDataBase.tables(QSql::Tables);
            if ( db_tables.isEmpty() ) {
                qDebug() << "Sqlite3_connector::getDatabaseState(): DB_NOTABLES";
                sqlDataBase.close();
                state = DB_NOTABLES;
                goto dbstate_done;
            }
        }
    } else {
        qDebug() << "Sqlite3_connector::getDatabaseState(): DB_INVALID";
        sqlDataBase.close();
        state = DB_INVALID;
        goto dbstate_done;
    }

    // If we get this far, we know the file exists, and it's not invalid, and it has tables
    // But it might only have one table, and not the table calling this function
    if ( !db_tables.contains(tableName) ) {
        state = DB_NOTABLES;
        goto dbstate_done;
    }

    count = getRowCount(tableName);
    if ( count > 0 ) {
        state = DB_HAS_TABLE_DATA;
        goto dbstate_done;
    } else {
        state = DB_NOTABLES;
    }

dbstate_done:
    sqlDataBase.close();

    return state;
}

void Sqlite3_connector::setSerialPtr(SerialComms *p) {
    serial_comms_p = p;
}

// Force the compiler to instantiate every instance of our templated function
template void Sqlite3_connector::registerTable(const QMap<int, dbfields_values_t> &r, StationData *p);
template void Sqlite3_connector::registerTable(const QMap<int, dbfields_values_t> &r, SysconfigData *p);
template void Sqlite3_connector::registerTable(const QMap<int, dbfields_values_t> &r, ContestData *p);

template <typename T>
void Sqlite3_connector::registerTable(const QMap<int, dbfields_values_t> &r, T *p) {
    if ( typeid(p) == typeid(StationData) ) {

    }
    table_list.append(r);
}
