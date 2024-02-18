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

    initialization_succeeded = false;
    database_initialized = 0;

    // Assemble the path to the on-disk database file
    dbpath = createDatabaseFullPath();

    qDebug() << "Sqlite3_connector::Sqlite3_connector(): Initializing QSqlDatabase connection";
    QSqlDatabase sqlDataBase = QSqlDatabase::addDatabase("QSQLITE", "ConfigDB");
    sqlDataBase.setDatabaseName(dbpath);
    if ( !sqlDataBase.isValid() ) {
        qDebug() << "Sqlite3_connector::Sqlite3_connector(): Error: isValid returned false";
        return;
    }

    qDebug() << "Sqlite3_connector::Sqlite3_connector(): Instantiating DataBase Table class objects";
    pStationData = new StationData(this);
    pSysconfigData = new SysconfigData(this);
    pContestData = new ContestData(this);

    enumerate_available_serial_ports();
}

Sqlite3_connector::~Sqlite3_connector() {

    QSqlDatabase::removeDatabase("ConfigDB");
    delete pContestData;
    delete pSysconfigData;
    delete pStationData;
}

bool Sqlite3_connector::initDatabase() {

    bool rc;
    int rc_int;

    qDebug() << "Sqlite3_connector::initDatabase(); Entered";

    // We should first check to see if the file exists, as a first-line check
    //  and if not, go on to create it.
    QString &dbpath = getDatabasePath();
    if (dbpath.isEmpty() ) {
        qDebug() << "Sqlite3_connector::initDatabase(): Invalid dbpath:" << dbpath;
        return false;
    }

    if ( !validateDatabasePath() ) {
        qDebug() << "Sqlite3_connector::initDatabase(): Database path does not exist";
        if ( !create_database_path(dbpath) ) {
            qDebug() << "Sqlite3_connector::initDatabase(): ERROR creating Database path";
            return false;
        }
    }

     // Open the database
    qDebug() << "Sqlite3_connector::Sqlite3_connector(): Opening" << dbpath;
    QSqlDatabase sqlDataBase = QSqlDatabase::database("ConfigDB");

    qDebug() << "Sqlite3_connector::initDatabase(): Database connector is isValid" << sqlDataBase.isValid();

    database_state state = getDatabaseState();
    // DB_NOEXIST, DB_NOTABLES, DB_HAS_STATIONTABLE, DB_HAS_MULTITABLES, DB_INVALID, DB_ERROR

    if ( state == DB_ERROR || state == DB_INVALID ) {
        qDebug() << "Sqlite3_connector::initDatabase(): database state is ERROR or INVALID";
        return false;
    }

    // Database file is created on disk on open() if it doesn't already exist.
    rc = sqlDataBase.open();
    if ( rc == false ) {
        QString err = sqlDataBase.lastError().text();
        qDebug() << "Sqlite3_connector::Sqlite3_connector(): Error: connection with database failed" << err;
        return false;
    }
    else {
        qDebug() << "Database: connection ok, file open";
    }

    // See if the database is populated or we need to initialize it
    QStringList db_tables = sqlDataBase.tables(QSql::Tables);
    if ( db_tables.isEmpty() ) {
        qDebug() <<  "DATABASE HAS NO TABLES - INITIALIZATION IS REQUIRED";
        rc_int = display_message_box("Database requires initialization - please configure station data", true);
        if ( rc_int != USER_PRESSED_CONFIG ) {
            qDebug() << "Sqlite3_connector::initDatabase(): User pressed Abort";
            sqlDataBase.close();
            return false;
        }

        // Create the database table objects
        pStationData = new StationData(this);
        pSysconfigData = new SysconfigData(this);
        pContestData = new ContestData(this);
    }

#if 0
    rc = syncStationData_read();
    if ( rc == false ) {
        sqlDataBase.close();
        qDebug() << "Sqlite3_connector::initDatabase(): Failed syncStationData_read()";
        return false;
    }

    rc = syncContestData_read();
    if ( rc == false ) {
        sqlDataBase.close();
        return false;
    }

    rc = syncSysconfigData_read();
    if ( rc == false ) {
        sqlDataBase.close();
        return false;
    }
#endif
    sqlDataBase.close();

    return true;
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

#if 0
template <typename T>
bool Sqlite3_connector::syncGeneric_write_to_database(T *pDbClass) {

    bool rc;
    qDebug() << "Sqlite3_connector::syncGeneric_write_to_database(): Entered";

    // SQLite command needs to look like this:
    // INSERT INTO station_data (callsign, opname, gridsquare, city, state, county, country, section)
    //  VALUES ("K1AYabc","Chris","EL96av","Punta Gorda","FL","Charlotte","USA","WCF");

    // TODO: This is super ugly - fix it - find a generic way

    while (dbtables.hasNext() ) {
        // The first iteration of this loop would be station_tables
        QString table_name = dbtables.next();
        qDebug() << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << table_name;

        // Begin by creating a string like this: "INSERT INTO station_data ("
        QString op = "INSERT INTO ";
        op.append(table_name);
        op.append(" (");

        QString fields;
        QMapIterator<int, QString> m(tableMap);     // Iterate through table identified by dbtables above
        while (m.hasNext() ) {
            m.next();
            fields.append(m.value());
            if ( m.hasNext() )
                fields.append(", ");
            else
                fields.append(" ");     // Can't allow comma after last field
        }
        op.append(fields + ") ");
        op.append("VALUES (");  // Continue building SQlite command

        // Return iterator to front of list (before first item) so we can use it again
        m.toFront();    // database table map
        while (m.hasNext() ) {
            m.next();
            qDebug() << m.key() << m.value();

            // TODO: This is ugly - fix it - find a generic way
            if ( table_name == "station_data" )
                op.append("\"" + get_station_data_table_value_by_key(m.value()) + "\"");
            else if ( table_name == "sysconfig_data" )
                op.append("\"" + get_sysconfig_table_value_by_key(m.value()) + "\"");
            else if ( table_name == "contest_data" )
                op.append("\"" + get_contest_table_value_by_key(m.value()) + "\"");

            if ( m.hasNext() )
                op.append(", ");
            else
                op.append(" ");     // Can't allow comma after last field
        }

        // Note we don't add a trailing comma to the last field here
        op.append(");");

        // qDebug() wants to escape all double quotes in QSring so we do it this way
        qDebug() << "***********>>>>>>>>>>>: " << op.toUtf8().constData();

        QSqlQuery q(op, sqlDataBase);
        // q.prepare(op);
        rc = q.exec();
        if ( rc == false ) {
            qDebug() << "Sqlite3_connector::syncGeneric_write_to_database(): q.exec() returned" << rc;
            return false;
        }

    }  // while (iterate through database tables)

    return true;
}
#endif

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

// Force the compiler to instantiate these templated functions to avoid putting the def's in header file
template bool Sqlite3_connector::SyncDataTableRead_T<StationData>(StationData *);
template bool Sqlite3_connector::SyncDataTableRead_T<SysconfigData>(SysconfigData *);
template bool Sqlite3_connector::SyncDataTableRead_T<ContestData>(ContestData *);

template <typename T>
bool Sqlite3_connector::SyncDataTableRead_T(T *pDataTableClass) {

    bool rc = false;

    const QMap<int, QString> &pDbFields = pDataTableClass->getDbFields();
    QMapIterator<int, QString> i(pDbFields);

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

    i.toFront();    // Reposition db fields iterator to front of list
    i.next(); // Skip over the table name
    int index = 0;
    while ( q.next() ) {
        while (i.hasNext() ) {
            i.next();
            QString db_table_fieldname = i.value();
            // Populate our local copy of station data
            qDebug() << "Sqlite3_connector::SyncDataTableRead_T(): db_table_fieldname:" << db_table_fieldname << "value:" << q.value(db_table_fieldname).toString();
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

    // qDebug() << "Sqlite3_connector::getRowCount(): Opening" << dbpath;
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
            rowcount++;
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

enum database_state Sqlite3_connector::getDatabaseState() {

    // {DB_NOEXIST, DB_NOTABLES, DB_HAS_TABLE_DATA, DB_INVALID, DB_ERROR}
    // We want to know, sequentially: does the file exist?  Does it have tables?  Do the tables have data?
    database_state state = DB_NOEXIST;
    int count = 0;

    QString dbFile = "/Users/chris/";
    dbFile.append(HIDDEN_WORKDIR);
    dbFile.append("/winkeyer_db");

    QFile file (dbFile);

    if ( !file.exists() ) {
        qDebug() << "Sqlite3_connector::getDatabaseState(): DB_NOEXIST";
        return DB_NOEXIST;
    }

    QStringList db_tables;

    QSqlDatabase sqlDataBase = QSqlDatabase::database("ConfigDB");

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


    count = getRowCount("station_data");
    if ( count > 0 ) {
        state = DB_HAS_TABLE_DATA;
        goto dbstate_done;
    }

dbstate_done:
    sqlDataBase.close();

    return state;
}

void Sqlite3_connector::setInitStatus(bool status) {
    initialization_succeeded = status;
}

void Sqlite3_connector::setSerialPtr(SerialComms *p) {
    serial_comms_p = p;
}

void Sqlite3_connector::registerTable(const QMap<int, QString> &r) {
    table_list.append(r);
}
