#include <QPushButton>
#include <QString>

#include "sqlite3-connector.h"

#define USER_PRESSED_CONFIG 1
#define USER_PRESSED_ABORT 2
#define FETCH_LAST

Sqlite3_connector::Sqlite3_connector()
{
    // Initialize QMap function pointers
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

    enumerate_available_serial_ports();

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

bool Sqlite3_connector::create_database_path(QString &rpath) {

    bool rc;

    // Validate that the path components to the database file on local storage is valid
    QDir dir;
    rc = dir.mkpath(rpath);
    if ( rc == false ) {
        qDebug() << "Sqlite3_connector::create_database_path(): mkpath failed";
    }

    return rc;
}

 QList<QString> Sqlite3_connector::GetTableNameList() {

    // Build a simple list of QStrings containing our table names
    QList<QString> items;

    QList<QMap<int, QString>> &table_list = getTableList();
    QListIterator m(table_list);
    while ( m.hasNext() ) {
        QString st = m.next().value(0);
        items.append(st);
        qDebug() << "Table value = " << st;
    }
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

bool Sqlite3_connector::syncStationData_read() {

    bool rc = false;

    const QMap<int, QString> &pDbFields = pStationData->getDbFields();
    QMapIterator<int, QString> i(pDbFields);

#ifdef FETCH_LAST
    // Fetch the last row from the station table
    QString op = "SELECT * FROM station_data ORDER BY rowid DESC LIMIT 1;";

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

    qDebug() << "Sqlite3_connector::syncStationData_read(): CMD" << op;

    QSqlDatabase sqlDataBase = QSqlDatabase::database("ConfigDB");

    if ( !sqlDataBase.isValid() ) {
        qDebug() << "SSqlite3_connector::syncStationData_read(): Error: isValid returned false";
        return false;
    }

    QSqlQuery q(sqlDataBase);
    rc = q.prepare(op);
    if ( !rc ) {
        QString err = q.lastError().text();
        qDebug() << "Sqlite3_connector::syncStationData_read(): Failed QsqlQuery prepare():" << err;
        sqlDataBase.close();
        return false;
    }

    rc = q.exec();

    // Query results are invalid unless isActive() and isSelect() are both true
    if ( !q.isActive() || !q.isSelect() ) {
        display_message_box("Database query failed - invalid reading station_data table");
        return false;
    }

    qDebug() << "Sqlite3_connector::syncStationData_read(): SQL Query:" << rc;

    QMap<int, QString> &rMap = pStationData->getLocalDataMap();

    i.toFront();    // Reposition db fields iterator to front of list
    i.next(); // Skip over the table name
    int index = 1;
    while ( q.first() ) {
        while (i.hasNext() ) {
            i.next();
            QString st = i.value();
            // Populate our local copy of station data
            qDebug() << "Sqlite3_connector::syncStationData_read(): st:" << st << "value:" << q.value(st).toString();
            rMap.insert(index, q.value(st).toString());
        }
        q.next();
    }
    return rc;
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

// Force the compiler to instantiate these templated functions to avoid putting the def's in header file
template bool Sqlite3_connector::SyncDataTableRead_T<StationData>(StationData *);
template bool Sqlite3_connector::SyncDataTableRead_T<SysconfigData>(SysconfigData *);
template bool Sqlite3_connector::SyncDataTableRead_T<ContestData>(ContestData *);

template <typename T>
bool Sqlite3_connector::SyncDataTableRead_T(T *pDataTableClass) {

    bool rc = false;

    const QMap<int, QString> &pDbFields = pDataTableClass->getDbFields();
    QMapIterator<int, QString> dbFieldsIter(pDbFields);

    QString tablename = pDbFields.value(0);

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

    qDebug() << "Sqlite3_connector::SyncDataTableRead_T(): CMD" << op;

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

    qDebug() << "Sqlite3_connector::SyncDataTableRead_T(): SQL Query:" << rc;

    QMap<int, QString> &lMap = pDataTableClass->getLocalDataMap();

    dbFieldsIter.toFront();    // Reposition db fields iterator to front of list
    dbFieldsIter.next(); // Skip over the table name
    int index = 0;
    while ( q.next() ) {
        while (dbFieldsIter.hasNext() ) {
            dbFieldsIter.next();
            QString db_table_fieldname = dbFieldsIter.value();
            // Populate our local copy of station data
            // qDebug() << "Sqlite3_connector::SyncDataTableRead_T(): db_table_fieldname:" << db_table_fieldname << "value:"
            //          << q.value(db_table_fieldname).toString();
            lMap.insert(index++, q.value(db_table_fieldname).toString());
        }
    }
    return rc;
}

bool Sqlite3_connector::syncSysconfigData_read() {

    bool rc = false;
#if 0
    bool result_valid;
    QMapIterator<int, QString> i(db_sysconfig_fields);

// getRowCount();
#ifdef FETCH_LAST
    // Fetch the last row from the sysconfig data table
    QString op = "SELECT * FROM sysconfig_data ORDER BY rowid DESC LIMIT 1;";

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
    op.append("FROM sysconfig_data;");
    qDebug() << op;
#endif

    qDebug() << "Sqlite3_connector::syncSysconfigData_read(): CMD" << op;

    QSqlQuery q(op, sqlDataBase);
    // q.prepare(op);
    rc = q.exec();

    // Query results are invalid unless isActive() and isSelect() are both true
    if ( !q.isActive() || !q.isSelect() ) {
        display_message_box("Database query failed - invalid reading sysconfig_data table");
        return false;
    }

    qDebug() << "Sqlite3_connector::syncSysconfigData_read(): SQL Query:" << rc;

    result_valid = q.first();  // Position query to first returned result
    while ( result_valid ) {
        // Return the iterator to the front of the list (before first item)
        i.toFront();
        while (i.hasNext() ) {
            i.next();
            QString st = i.value();
            // Populate our local copy of sysconfig data
            sysconfig_data_list_local_map[st] = q.value(st).toString();
        }
        result_valid = q.next();
    }
#endif
    return rc;
}

bool Sqlite3_connector::syncContestData_read() {

    bool rc = false;
#if 0
    bool result_valid;
    QMapIterator<int, QString> i(db_contest_fields);

#ifdef FETCH_LAST
    // Fetch the last row from the contest data table
    QString op = "SELECT * FROM contest_data ORDER BY rowid DESC LIMIT 1;";

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
    op.append("FROM contest_data;");
    qDebug() << op;
#endif

    qDebug() << "Sqlite3_connector::syncContestData_read(): CMD" << op;

    QSqlQuery q(op, sqlDataBase);
    // q.prepare(op);
    rc = q.exec();

    // Query results are invalid unless isActive() and isSelect() are both true
    if ( !q.isActive() || !q.isSelect() ) {
        display_message_box("Database query failed - invalid reading contest_data table");
        return false;
    }

    qDebug() << "Sqlite3_connector::syncContestData_read(): SQL Query:" << rc;

    result_valid = q.first();  // Position query to first returned result
    while ( result_valid ) {
        // Return the iterator to the front of the list (before first item)
        i.toFront();
        while (i.hasNext() ) {
            i.next();
            QString st = i.value();
            // Populate our local copy of contest data
            contest_data_list_local_map[st] = q.value(st).toString();
        }
        result_valid = q.next();
    }
#endif
    return rc;
}

int  Sqlite3_connector::getRowCount(QString table) {

    int rowcount = 0;
    bool rc;

    qDebug() << "Sqlite3_connector::getRowCount(): Entered";
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
    qDebug() << "Sqlite3_connector::getRowCount(): op:" << op;

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
            qDebug() << "Sqlite3_connector::getRowCount():" << q.value(0).toInt();
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

QString Sqlite3_connector::get_station_data_table_value_by_key(int key) {

    QMap<int, QString> &r = pStationData->getLocalDataMap();
    return r.value(key);
}

QString Sqlite3_connector::get_sysconfig_table_value_by_key(int key) {

    QMap<int, QString> &r = pSysconfigData->getLocalDataMap();
    return r.value(key);
}

QString Sqlite3_connector::get_contest_table_value_by_key(int key) {

    QMap<int, QString> &r = pContestData->getLocalDataMap();
    return r.value(key);

}

void Sqlite3_connector::set_station_data_table_value_by_key(int key, QString value) {
    QMap<int, QString> &r = pStationData->getLocalDataMap();
    r.insert(key, value);
}

void Sqlite3_connector::set_sysconfig_table_value_by_key(int key, QString value) {
    QMap<int, QString> &r = pSysconfigData->getLocalDataMap();
    r.insert(key, value);
}

void Sqlite3_connector::set_contest_table_value_by_key(int key, QString value) {
    QMap<int, QString> &r = pContestData->getLocalDataMap();
    r.insert(key, value);
}

void Sqlite3_connector::dump_local_station_data() {

    qDebug() << "Dumping local station data table";

    QMap<int, QString> &rMap = pStationData->getLocalDataMap();
    if ( rMap.isEmpty() ) {
        qDebug() << "Sqlite3_connector::dump_local_station_data(): Local Station Map empty";
        return;
    }

    QMapIterator<int, QString> m(rMap);
    while (m.hasNext() ) {
        m.next();
        qDebug() << m.key() << ":" << m.value();
    }
}

void Sqlite3_connector::dump_local_sysconfig_data() {

    qDebug() << "Dumping local system config data table";
    QMap<int, QString> &rMap = pSysconfigData->getLocalDataMap();
    QMapIterator<int, QString> m(rMap);
    while (m.hasNext() ) {
        m.next();
        qDebug() << m.key() << ":" << m.value();
    }
}

void Sqlite3_connector::dump_local_contest_data() {

    qDebug() << "Dumping local contest data table";
    QMap<int, QString> &rMap = pContestData->getLocalDataMap();
    QMapIterator<int, QString> m(rMap);
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

void Sqlite3_connector::enumerate_available_serial_ports() {

    serial_port_list.clear();
    serial_port_list = QSerialPortInfo::availablePorts();
    QList<QSerialPortInfo>::iterator i;
    for (i = serial_port_list.begin(); i != serial_port_list.end(); ++i) {
        qDebug() << "Sqlite3_connector::enumerate_available_serial_ports():" << i->portName();
    }
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

void Sqlite3_connector::registerTable(const QMap<int, QString> &r) {
    table_list.append(r);
}
