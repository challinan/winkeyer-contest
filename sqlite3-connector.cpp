#include "sqlite3-connector.h"

Sqlite3_connector::Sqlite3_connector()
{

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
    qDebug() << "Sqlite3_connector::initDatabase(): isValid returned" << db.isValid();

    rc = db.open();
    if ( rc == false ) {
        qDebug() << "Sqlite3_connector::Sqlite3_connector(): Error: connection with database failed";
        return false;
    }
    else {
        qDebug() << "Database: connection ok";
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

bool Sqlite3_connector::syncStationData() {

    bool rc;
    int rowcount;

    QString op = "INSERT INTO station_data (";
    QString str;
    foreach (str, db_station_fields)
        op.append(str);
    op.append(") ");

    // op.append(
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

int  Sqlite3_connector::getRowCount() {

    int rowcount = 0;
    bool rc;

    QString op = "select callsign from station_data;";
    QSqlQuery q;
    q.prepare(op);
    rc = q.exec();
    if ( !rc ) return -1;

    if ( q.isActive() && q.isSelect() ) {
        while ( q.next() ) {
            rowcount++;
            qDebug() << "rowcount" << rowcount;
        }
    } else {
        return -1;
    }

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


