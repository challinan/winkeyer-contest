#include "contest_configuration.h"

ContestConfiguration::ContestConfiguration(QObject *parent, Sqlite3_connector *p)
    : QObject{parent}
{
    db = p; // Copy of our database class
    runMode = SNP_MODE;     // Default to Search & Pounce

    qDebug() << "ContestConfiguration::ContestConfiguration(): Entered";
    // createGlobalContestTable();
    readCwDefaultMsgFile();
    displayFunctionKeyMap();
}

bool ContestConfiguration::getDatabaseConnection() {

    qDebug() << "ContestConfiguration::openDatabase(): Entered";

    dbpath = db->getDatabasePath();
    sqlDatabase = QSqlDatabase::addDatabase("QSQLITE", "TempDBConnx");
    return true;
}

bool ContestConfiguration::createGlobalContestTable(){

    // CREATE command needs to look like this:
    // CREATE TABLE table_name (field1 TEXT, field2 TEXT, field3 TEXT);

    qDebug() << "ContestConfiguration::createGlobalContestTable(): Entered";
    bool rc = false;

    // Prepare the CREATE statement
    QString cmd = "CREATE TABLE ContestGlobal (";
    cmd.append("CabrilloName TEXT, CabrilloID INT, DisplayName TEXT );");

    qDebug() << "ContestConfiguration::createGlobalContestTable(): cmd: " << cmd;

    QSqlDatabase sqlDataBase = QSqlDatabase::database("ConfigDB");

    rc = sqlDataBase.open();
    if ( rc ) {
        qDebug() << "ContestConfiguration::createGlobalContestTable(): Database: connection ok, file open";
    }
    else {
        QString err = sqlDataBase.lastError().text();
        qDebug() << "ContestConfiguration::createGlobalContestTable(): Error: connection with database failed" << err;
        return false;
    }

    QSqlQuery q(sqlDataBase);
    rc = q.prepare(cmd);
    if ( !rc ) {
        QString err = q.lastError().text();
        qDebug() << "ContestConfiguration::createGlobalContestTable(): Failed QsqlQuery prepare():" << err;
        sqlDataBase.close();
        return false;
    }

    rc = q.exec();
    if ( rc == false ) {
        qDebug() << "ContestConfiguration::createGlobalContestTable(): Unexpected failure in SQL query exec:" << q.lastError().text();
        sqlDataBase.close();
        return false;
    }

    qDebug() << "ContestConfiguration::createGlobalContestTable(): q.exec() returned" << rc;

    // SQLite command needs to look like this:
    // INSERT INTO station_data (callsign, opname, gridsquare, city, state, county, country, section)
    //  VALUES ("K1AYabc","Chris","EL96av","Punta Gorda","FL","Charlotte","USA","WCF");

    QListIterator<struct cabrillo_contest_names_t> m(cabrillo_contest_data);
    while ( m.hasNext() ) {
        cmd.clear();
        struct cabrillo_contest_names_t c = m.next();
        QString strCabrilloName = c.cabrillo_name;
        int cabrilloID = c.contest_id;
        QString id;  id.setNum(cabrilloID);
        QString strCabrilloDisplayName = c.contest;

        // CabrilloName TEXT, CabrilloID INT, DisplayName TEXT
        cmd.append("INSERT INTO ContestGlobal (CabrilloName, CabrilloID, DisplayName) VALUES (");
        cmd.append("\"" + strCabrilloName + "\", ");
        cmd.append(id); cmd.append(", ");
        cmd.append("\"" + strCabrilloDisplayName + "\")");
        qDebug().noquote() << cmd;

        rc = q.prepare(cmd);
        if ( !rc ) {
            QString err = q.lastError().text();
            qDebug() << "ContestConfiguration::createGlobalContestTable(): Failed QsqlQuery prepare():" << err;
            sqlDataBase.close();
            return false;
        }

        rc = q.exec();
        if ( rc == false ) {
            qDebug() << "ContestConfiguration::createGlobalContestTable(): Unexpected failure in SQL query exec:" << q.lastError().text();
            sqlDataBase.close();
            return false;
        }
    }

    sqlDataBase.close();
    return true;
}

bool ContestConfiguration::readCwDefaultMsgFile() {

    bool rc;

    QString fileName = "/Users/chris/.macrr/CW-Default-Messages.mc";
    QFile cwFile (fileName);
    rc = cwFile.open(QIODeviceBase::ReadWrite | QIODeviceBase::Text);

    if ( !rc )
        return false;

    state_e state = UNKNOWN_MODE;
    while ( !cwFile.atEnd() ) {

        QString line = cwFile.readLine();
        struct func_key_t fkey_s;

        if ( line.contains("RUN Messages") ) {
            state = RUN_MODE;
            continue;
        }

        if ( line.contains("S&P Messages") ) {
            state = SNP_MODE;
            continue;
        }

        if ( line.startsWith("#") ) // Ignore comment only lines
            continue;

        if ( line.startsWith("F") ) {
            if ( state == UNKNOWN_MODE ) {
                qDebug() << "ContestConfiguration::readCwDefaultMsgFile(): unknown state parsing mc file: line:" << line;
                cwFile.close();
                return false;
            }

            // Isolate the function key
            int i = line.indexOf(" ");
            QString fkey = line.left(i);
            // qDebug() << "ContestConfiguration::readCwDefaultMsgFile(): Function key:" << fkey;
            fkey_s.functionKey = fkey;
            line.remove(0, fkey.size()+1);

            // Isolate the Function Key label
            i = line.indexOf(",");
            QString fLabel = line.left(i);
            // qDebug() << "ContestConfiguration::readCwDefaultMsgFile(): fLabel:" << fLabel;
            fkey_s.label = fLabel;

            line.remove(0, i+1);
            // qDebug() << "ContestConfiguration::readCwDefaultMsgFile(): exchange:" << line;
            fkey_s.exchange = line;

            // Add the run state
            fkey_s.run_state = state;

            // Store this function key in a QMap
            cwFuncKeys.append(fkey_s);
        }
    }

    cwFile.close();
    return true;
}

void ContestConfiguration::displayFunctionKeyMap() {

    qDebug() << "ContestConfiguration::displayFunctionKeyMap(): Map data follows";

    QListIterator l(cwFuncKeys);
    while ( l.hasNext() ) {
        struct func_key_t f = l.next();
        QString runstate = f.run_state == 1 ? "Run Mode" : "S&P Mode";
        qDebug() << "    " << runstate  << f.functionKey << f.label << f.exchange;
    }
}

state_e ContestConfiguration::getRunMode() {
    return runMode;
}

void ContestConfiguration::setRunMode(state_e mode) {
    runMode = mode;
}
