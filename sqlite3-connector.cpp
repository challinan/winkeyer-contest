#include "sqlite3-connector.h"

#define FETCH_LAST

Sqlite3_connector::Sqlite3_connector()
{
    // Initialize QMap function pointers
    qDebug() << "Sqlite3_connector::Sqlite3_connector(): Entered";
#if 0
    QList<QString>::iterator e;
    for (e = db_station_fields.begin(); e != db_station_fields.end(); ++e) {
        QString s = *e;
        functionMap.insert(s, 0x0);
        qDebug() << "Sqlite3_connector::Sqlite3_connector():" << s;
    }
#endif

    // Populate the local station db table (QMap) with dummy QString objects
    QMapIterator<QString, int> m(db_station_fields_tmp); // m is reference to db_station_fields_tmp
    while (m.hasNext() ) {
        m.next();
        QString s = "";
        station_data_list_local_map.insert(s, "");
    }

    initDatabase();
    syncStationData_read();

#if 0
    QMapIterator<QString, int> n(db_station_fields_tmp);
    while (n.hasNext() ) {
        n.next();
        qDebug() << n.key() << ":" << n.value() << "->" << station_data_list_local.at(n.value());
    }
#endif
}

Sqlite3_connector::~Sqlite3_connector() {
    db.close();
}

bool Sqlite3_connector::initDatabase() {

    bool rc;

    // Get user's home directory
    QString userHomeDir;
    userHomeDir = QDir::homePath();

    QDir databasePath;
    rc = databasePath.setCurrent(userHomeDir);
    QString dbPath = databasePath.currentPath() + "/.macrr" + "/" + db_filename;

    qDebug() << "Sqlite3_connector::Sqlite3_connector(): database path = " << dbPath << "rc =" << rc;

    // We should first check to see if the file exists, as a first-line check
    //  and if not, go on to create it.

    // Open the database
    qDebug() << "Sqlite3_connector::Sqlite3_connector(): Opening" << dbPath;
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    qDebug() << "Sqlite3_connector::initDatabase(): Database connector is isValid" << db.isValid();

    rc = db.open();
    if ( rc == false ) {
        qDebug() << "Sqlite3_connector::Sqlite3_connector(): Error: connection with database failed";
        return false;
    }
    else {
        qDebug() << "Database: connection ok, file open";
    }

    // See if the database is populated or we need to initialize it
    QStringList db_tables = db.tables(QSql::Tables);
    if ( db_tables.isEmpty() ) {
        qDebug() <<  "DATABASE HAS NO TABLES - INITIALIZATION IS REQUIRED";
        rc = createStationTable();
        if ( rc == false )
            return false;
    }

    return true;
}

bool Sqlite3_connector::createStationTable() {

    bool rc;

    if ( !db.isValid() ) {
        qDebug() << "Sqlite3_connector::createStationTable(): invalid database";
        return false;
    }

    QString s = "CREATE TABLE station_data ( "
                      "  callsign TEXT, "
                      "  opname TEXT, "
                      "  gridsquare TEXT, "
                      "  city TEXT, "
                      "  state TEXT, "
                      "  county TEXT, "
                      "  country TEXT, "
                      "  section TEXT, "
                      "  serialport TEXT "
                      " );";

    qDebug() << "Sqlite3_connector::createStationTable(): s =" << s;

    QSqlQuery q;
    q.prepare(s);
    rc = q.exec();
    qDebug() << "Sqlite3_connector::createStationTable(): q.exec() returned" << rc;
    return true;
}

bool Sqlite3_connector::syncStationData_write() {

    bool rc;
    int rowcount;

    QString op = "INSERT INTO station_data (";
    QString str;
    foreach (str, db_station_fields)
        op.append(str);
    op.append(") ");

    // "callsign, ", "opname, ", "gridsquare, ", "city, ", "state, ",
    //     "county, ", "country, ", "section, ", "serialport"}

    op.append("VALUES (");
    op.append("\"" + getCallSign() + "\"" + ",");
    op.append("\"" + getName() + "\"" + ",");
    op.append("\"" + getGridSquare() + "\"" + ",");
    op.append("\"" + getCity() + "\"" + ",");
    op.append("\"" + getState() + "\"" + ",");
    op.append("\"" + getCounty() + "\"" + ",");
    op.append("\"" + getCountry() + "\"" + ",");
    op.append("\"" + getArrlSection() + "\"" + ",");
    op.append("\"" + getSerialPort() + "\"");
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
    QString s;
    QMapIterator<QString, int> i(db_station_fields_tmp);

    getRowCount();
#ifdef FETCH_LAST
    // Fetch the last row from the station table
    QString op = "SELECT * FROM station_data ORDER BY rowid DESC LIMIT 1;";

#else
    // Construct the query
    QString op = "SELECT ";
    while (i.hasNext() ) {
        i.next();
        op.append(i.key());
        if ( i.hasNext() )
            op.append(", ");
        else
            op.append(" ");     // Can't allow comma after last field
    }
    op.append("FROM station_data;");
    qDebug() << op;
#endif

    QSqlQuery q;
    q.prepare(op);
    rc = q.exec();

    // Query results are invalid unless isActive() and isSelect() are both true
    if ( !q.isActive() || !q.isSelect() )
        return false;

    qDebug() << "Sqlite3_connector::syncStationData_read(): SQL Query:" << rc;

    result_valid = q.first();  // Position query to first returned result
    while ( result_valid ) {
        // Return the iterator to the front of the list (before first item)
        i.toFront();
        while (i.hasNext() ) {
            i.next();
            QString st = i.key();
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
    if ( !rc ) return -1;

    if ( q.isActive() && q.isSelect() ) {
        while ( q.next() ) {
            rowcount++;
        }
    } else {
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
    if ( !rc ) return -1;

    return true;
}

QList<QString> Sqlite3_connector::get_station_data_table_keys() {

    return db_station_fields_tmp.keys();
}

void Sqlite3_connector::insert_local_station_data_field(QString key, QString value) {
    station_data_list_local_map.insert(key, value);
}

QString Sqlite3_connector::get_stataion_data_table_value_by_key(QString key) {
    qDebug() << "Sqlite3_connector::get_stataion_data_table_value_by_key(): key" << key << "value" << station_data_list_local_map.value(key);
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

// Setters and getters
QString Sqlite3_connector::getName() {
    return opname;
}

QString Sqlite3_connector::getCallSign() {
    return callSign;
}

QString Sqlite3_connector::getGridSquare() {
    return gridSquare;
}

QString Sqlite3_connector::getCity() {
    return city;
}

QString Sqlite3_connector::getState() {
    return state;
}

QString Sqlite3_connector::getCountry() {
    return country;
}

QString Sqlite3_connector::getCounty() {
    return county;
}

QString Sqlite3_connector::getSerialPort() {
    return serial_port;
}

QString Sqlite3_connector::getArrlSection() {
    return arrl_section;
}

void Sqlite3_connector::setName(QString s) {
    opname = s;
}

void Sqlite3_connector::setCallSign(QString s) {
    callSign = s;
}

void Sqlite3_connector::setGridSquare(QString s) {
    gridSquare = s;
}

void Sqlite3_connector::setCity(QString s) {
    city = s;
}

void Sqlite3_connector::setState(QString s) {
    state = s;
}

void Sqlite3_connector::setCountry(QString s) {
    country = s;
}

void Sqlite3_connector::setCounty(QString s) {
    county = s;
}

void  Sqlite3_connector::setSerialPort(QString s) {
    serial_port = s;
}

void Sqlite3_connector::setSerialPtr(SerialComms *p) {
    serial_comms_p = p;
}

void  Sqlite3_connector::setArrlSection(QString s) {
    arrl_section = s;
}


