#include "sqlite3-connector.h"
#include <QPushButton>

#define USER_PRESSED_CONFIG 1
#define USER_PRESSED_ABORT 2
#define FETCH_LAST

Sqlite3_connector::Sqlite3_connector()
{
    // Initialize QMap function pointers
    qDebug() << "Sqlite3_connector::Sqlite3_connector(): Entered";

    initialization_succeeded = true;
    database_initialized = 0;

    enumerate_available_serial_ports();
}

Sqlite3_connector::~Sqlite3_connector() {
    db.close();
    db.removeDatabase("QSQLITE");
}

bool Sqlite3_connector::initDatabase() {

    bool rc;
    int rc_int;

    // We should first check to see if the file exists, as a first-line check
    //  and if not, go on to create it.

    getDatabaseFullPath();
    if ( !validateDatabaseFullPath() ) {
        qDebug() << "Sqlite3_connector::initDatabase(): Database path does not exist";
        if ( !create_database_path() ) {
            qDebug() << "Sqlite3_connector::initDatabase(): ERROR creating Database path";
            return false;
        }
    }

    database_state state = getDatabaseState();
    // DB_NOEXIST, DB_EMPTY, DB_HAS_STATIONTABLE, DB_HAS_MULTITABLES, DB_INVALID, DB_ERROR
    if ( state == DB_ERROR || state == DB_INVALID ) {
        qDebug() << "Sqlite3_connector::initDatabase(): database state is ERROR or INVALID";
        return false;
    }

    // Open the database
    qDebug() << "Sqlite3_connector::Sqlite3_connector(): Opening" << dbpath;
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbpath);
    qDebug() << "Sqlite3_connector::initDatabase(): Database connector is isValid" << db.isValid();

    // Database file is created on disk on open() if it doesn't already exist.
    rc = db.open();
    if ( rc == false ) {
        qDebug() << "Sqlite3_connector::Sqlite3_connector(): Error: connection with database failed";
        display_message_box("Connection with database failed");
        return false;
    }
    else {
        qDebug() << "Database: connection ok, file open";
    }

    // See if the database is populated or we need to initialize it
    QStringList db_tables = db.tables(QSql::Tables);
    if ( db_tables.isEmpty() ) {
        qDebug() <<  "DATABASE HAS NO TABLES - INITIALIZATION IS REQUIRED";
        rc_int = display_message_box("Databsase requires initialization - please configure station data", true);
        if ( rc_int != USER_PRESSED_CONFIG ) {
            qDebug() << "Sqlite3_connector::initDatabase(): User pressed Abort";
            return false;
        }

        rc = createStationTable();
        if ( rc == false ) {
            db.close();
            return false;
        }
    }

    syncStationData_read();

    return true;
}

void Sqlite3_connector::getDatabaseFullPath() {

    // Get user's home directory

    QString userHomeDir;
    userHomeDir = QDir::homePath();
    dbpath = userHomeDir + "/" + HIDDEN_WORKDIR + "/" + db_filename;
}

bool Sqlite3_connector::validateDatabaseFullPath() {

    // Isolate path name to db file
    int slash_index;
    slash_index = dbpath.lastIndexOf("/", -1);
    if ( slash_index == -1 ) {
        qDebug() << "Sqlite3_connector::validateDatabaseFullPath(): Error searching for / in str" << dbpath;
        return false;
    }

    QString s = dbpath;
    s.truncate(slash_index);
    qDebug() << "Sqlite3_connector::validateDatabaseFullPath(): path is" << s;

    // See if the path exists
    QFile file (s);

    if ( !file.exists() ) {
        qDebug() << "Sqlite3_connector::validateDatabaseFullPath(): File Not Found";
        return false;
    }

    return true;
}

bool Sqlite3_connector::create_database_path() {

    bool rc;

    QString userHomeDir;
    userHomeDir = QDir::homePath();

    // bool QDir::mkpath(const QString &dirPath) const
    QString path_to_workdir = userHomeDir;
    path_to_workdir.append("/");
    path_to_workdir.append(HIDDEN_WORKDIR);

    QDir dir;
    rc = dir.mkpath(path_to_workdir);
    if ( rc == false ) {
        qDebug() << "Sqlite3_connector::create_database_path(): mkpath failed";
        return false;
    }

    return rc;
}

bool Sqlite3_connector::createStationTable() {

    bool rc;

    qDebug() << "Sqlite3_connector::createStationTable(): Entered";

    if ( !db.isValid() ) {
        qDebug() << "Sqlite3_connector::createStationTable(): invalid database";
        display_message_box("Invalid Database Connection - isValid() is false");
        return false;
    }

    QMapIterator<int, QString> m(db_station_fields);

    QString s = "CREATE TABLE station_data (";
    while ( m.hasNext() ) {
        m.next();
        s.append(m.value());
        s.append(" TEXT");
        if ( m.hasNext() )
            s.append(", ");
    }
    s.append(");");

    qDebug() << "Sqlite3_connector::createStationTable(): s =" << s;

    QSqlQuery q;
    q.prepare(s);
    rc = q.exec();
    if ( rc == false )
        display_message_box("SQL query failed creating station table");

    qDebug() << "Sqlite3_connector::createStationTable(): q.exec() returned" << rc;
    return true;
}

bool Sqlite3_connector::syncStationData_write() {

    bool rc;
    int rowcount;
    qDebug() << "Sqlite3_connector::syncStationData_write(): Entered";

    // SQLite command needs to look like this:
    // INSERT INTO station_data (callsign, opname, gridsquare, city, state, county, country, section)
    //  VALUES ("K1AYabc","Chris","EL96av","Punta Gorda","FL","Charlotte","USA","WCF");

    QString op = "INSERT INTO station_data (";
    QString fields;
    QMapIterator<int, QString> m(db_station_fields);
    while (m.hasNext() ) {
        m.next();
        fields.append(m.value());
        if ( m.hasNext() )
            fields.append(", ");
        else
            fields.append(" ");     // Can't allow comma after last field
    }
    op.append(fields + ") ");

    // "callsign, ", "opname, ", "gridsquare, ", "city, ", "state, ",
    //     "county, ", "country, ", "section}

    op.append("VALUES (");
    op.append("\"" + get_stataion_data_table_value_by_key("callsign") + "\"" + ",");
    op.append("\"" + get_stataion_data_table_value_by_key("opname") + "\"" + ",");
    op.append("\"" + get_stataion_data_table_value_by_key("gridsquare") + "\"" + ",");
    op.append("\"" + get_stataion_data_table_value_by_key("city") + "\"" + ",");
    op.append("\"" + get_stataion_data_table_value_by_key("state") + "\"" + ",");
    op.append("\"" + get_stataion_data_table_value_by_key("county") + "\"" + ",");
    op.append("\"" + get_stataion_data_table_value_by_key("country") + "\"" + ",");

    // Note we don't add a trailing comma to the last field here
    op.append("\"" + get_stataion_data_table_value_by_key("section") + "\"");
    op.append(");");

    // qDebug() wants to escape all double quotes in QSring so we do it this way
    qDebug() << op.toUtf8().constData();

    QSqlQuery q;
    q.prepare(op);
    rc = q.exec();
    qDebug() << "Sqlite3_connector::syncStationData(): q.exec() returned" << rc;

    rowcount = getRowCount();
    qDebug() << "Sqlite3_connector::syncStationData(): rowcount = " << rowcount;

    return true;
}

bool Sqlite3_connector::syncStationData_read() {

    bool rc, result_valid;
    QMapIterator<int, QString> i(db_station_fields);

    // getRowCount();
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

    QSqlQuery q;
    q.prepare(op);
    rc = q.exec();

    // Query results are invalid unless isActive() and isSelect() are both true
    if ( !q.isActive() || !q.isSelect() ) {
        display_message_box("Database query failed - invalid");
        return false;
    }

    qDebug() << "Sqlite3_connector::syncStationData_read(): SQL Query:" << rc;

    result_valid = q.first();  // Position query to first returned result
    while ( result_valid ) {
        // Return the iterator to the front of the list (before first item)
        i.toFront();
        while (i.hasNext() ) {
            i.next();
            QString st = i.value();
            // Populate our local copy of station data
            station_data_list_local_map[st] = q.value(st).toString();
        }
        result_valid = q.next();
    }

    return rc;
}

int  Sqlite3_connector::getRowCount() {

    int rowcount = 0;
    bool rc;

    QString op = "SELECT callsign FROM station_data;";
    QSqlQuery q;
    q.prepare(op);
    rc = q.exec();
    if ( !rc ) {
        // display_message_box("Database query failed getting row count");
        return -1;
    }

    if ( q.isActive() && q.isSelect() ) {
        while ( q.next() ) {
            rowcount++;
        }
    } else {
        display_message_box("Database query failed while iterating - isActive or isSelect failed");
        return -1;
    }

    qDebug() << "Sqlite3_connector::getRowCount(): There are" << rowcount << "rows in station table";
    return rowcount;
}

bool Sqlite3_connector::dropStationTable() {

    bool rc;

    QString op = "DROP TABLE IF EXISTS station_data;";
    QSqlQuery q;
    q.prepare(op);
    rc = q.exec();
    qDebug() << "Sqlite3_connector::dropStationTable(): q.exec returned" << rc;
    if ( !rc ) {
        display_message_box("SQL query failed dropping station table");
        return -1;
    }

    return true;
}

QList<QString> Sqlite3_connector::get_station_data_table_keys() {

    QList<QString> s;

    QMapIterator<int, QString> m(db_station_fields);
    while (m.hasNext() ) {
        m.next();
        s.append(m.value());
    }
    return s;
}

QString Sqlite3_connector::get_stataion_data_table_value_by_key(QString key) {
    // qDebug() << "Sqlite3_connector::get_stataion_data_table_value_by_key(): key" << key << "value" << station_data_list_local_map.value(key);
    return station_data_list_local_map.value(key);
}

void Sqlite3_connector::dump_local_station_data() {

    qDebug() << "Dumping local station data table";
    QMapIterator<QString, QString> m(station_data_list_local_map);
    while (m.hasNext() ) {
        m.next();
        qDebug() << m.key() << ":" << m.value();
    }

}

void Sqlite3_connector::set_station_data_table_value_by_key(QString key, QString value) {
    station_data_list_local_map[key] = value;
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

    QString dbFile = "/Users/chris/";
    dbFile.append(HIDDEN_WORKDIR);
    dbFile.append("/winkeyer_db");

    QFile file (dbFile);
    QStringList db_tables;

    if ( !file.exists() ) {
        qDebug() << "Sqlite3_connector::getDatabaseState(): DB_NOEXIST";
        return DB_NOEXIST;
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbFile);
    if ( !db.open() ) {
        qDebug() << "Sqlite3_connector::getDatabaseState(): DB_ERROR during open";
        return DB_ERROR;
    }

    if ( db.isValid() ) {
        if ( db.isOpen() ) {
            // Check for empty database - ie file exists but it's got nothing in it
            db_tables = db.tables(QSql::Tables);
            if ( db_tables.isEmpty() ) {
                qDebug() << "Sqlite3_connector::getDatabaseState(): DB_EMPTY";
                db.close();
                return DB_EMPTY;
            }
        }
    } else {
        qDebug() << "Sqlite3_connector::getDatabaseState(): DB_INVALID";
        db.close();
        return DB_INVALID;
    }

    QListIterator<QString> m(db_tables);
    m.toFront();
    if ( m.hasNext() ) {
        while ( m.hasNext() ) {
            QString s = m.next();
            if ( s.contains("station") ) {
                if ( db_tables.size() > 1 ) {
                    qDebug() << "Sqlite3_connector::getDatabaseState(): DB_HAS_MULTITABLES";
                    return DB_HAS_MULTITABLES;
                }
                else {
                    db.close();
                    qDebug() << "Sqlite3_connector::getDatabaseState(): DB_HAS_STATIONTABLE only";
                    return DB_HAS_STATIONTABLE;
                }
            }
        }
    }

    if ( db_tables.size() > 1 ) {
        qDebug() << "Sqlite3_connector::getDatabaseState(): DB_HAS_MULTITABLES";
        db.close();
        return DB_HAS_MULTITABLES;
    }

    qDebug() << "Sqlite3_connector::getDatabaseState(): DB_ERROR";
    db.close();
    return DB_ERROR;
}

bool Sqlite3_connector::dbInitSucceeded() {
    return initialization_succeeded;
}

void Sqlite3_connector::setInitStatus(bool status) {
    initialization_succeeded = status;
}

// Setters and getters
void Sqlite3_connector::setSerialPtr(SerialComms *p) {
    serial_comms_p = p;
}


