#include "county_list.h"

CountyList::CountyList(QObject *parent)
    : QObject{parent}
{
    // Initialize variables

    QFile file("/Users/chris/sandbox/QtProjects/winkeyer-contest/us-counties.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if ( line.left(2) == QString("//") )
            continue;   // Skip comment lines
        process_line(line, file);
    }

    // displayTable();
}

void CountyList::process_line(QByteArray &line, QFile &file) {

    struct state_t state_tmp;
    static int countyCount = 0;

    if ( line.startsWith(Qt::Key_Space) ) {
        qDebug() << "CountyList::process_line(): Invalid space found at start of line";
        Q_ASSERT(false);
    }

    if ( line.contains("Counties")) {
        // Found a state declaration which includes the number of counties in the state
        countyCount = findCountyCount(line);
        line = line.trimmed();  // Remove any trailing spaces

        QString us_state = findState(line);
        us_state = us_state.trimmed();
        state_tmp.state = us_state;
        // qDebug() << "State:" << us_state << "has" << countyCount << "counties - HAVE STATE***";
        if ( us_state.isEmpty() )
            qDebug() << "State:" << us_state << "has" << countyCount << "counties - NO STATE***";
    } else {
        return;
    }

    // We can assume the next 'n' entries are the counties of a state
    for ( int i=0; i<countyCount; i++ ) {
        if ( !file.atEnd() ) {
            QByteArray line = file.readLine();
            if ( line.contains("County") ) {
                // We have a valid county line, isolate the name
                QString s = findCountyName(line);
                // qDebug() << "State: FOUND_STATE: County =" << s;
                state_tmp.counties.append(s);
            }
        }
        else {
            return;
        }
    }
    us_state_county_table.append(state_tmp);
}



QString CountyList::findCountyName(QByteArray &line) {

    int index = line.indexOf("County");
    return line.left(index-1).trimmed();

}

QString CountyList::findState(QByteArray line) {

    int index = 0;
    QByteArray::iterator i = line.begin();

    for ( ; i < line.end(); i++, index++ ) {
        if ( isNumber(*i) ) {
            break;
        }
    }

    return line.first(index-1);
}

int CountyList::findCountyCount(QByteArray line) {

    int i = 0, firstDigitIndex;
    int count;
    bool ok;

    while ( i < line.size() ) {
        if ( isNumber(line.at(i))  )
            break;
        i++;
    }
    if ( i == line.size() ) return -1;

    // OK if we get to here, i points to the first digit of the county count
    // We need to find the next space to isolate the count b/c it could be 1, 2 or 3 digits long
    firstDigitIndex = i;
    while ( i < line.size() ) {
        if ( line.at(i) == Qt::Key_Space )
            break;
        i++;
    }
    if ( i == line.size() ) return -1;  // Should never happen

    QByteArray bt = line.mid(firstDigitIndex, i-firstDigitIndex);
    count = bt.toInt(&ok);
    if ( ok )
        return count;
    return -1;
}

bool CountyList::isNumber(char c) {

    if ( c >= Qt::Key_0 && c <= Qt::Key_9 )
        return true;
    return false;
}

CountyList::~CountyList()
{
}

void CountyList::displayTable() {

    QListIterator<struct state_t> i(us_state_county_table);
    while ( i.hasNext() ) {
        struct state_t t = i.next();
        qDebug().noquote() << "state =" << t.state;
        if ( t.state.isEmpty() )
            qDebug() << "CountyList::displayTable(): NO STATE";

        QListIterator<QString> m(t.counties);
        while ( m.hasNext() ) {
            QString s = m.next();
            qDebug().noquote() << "    " << t.state << ":" << s;
        }
    }
}
