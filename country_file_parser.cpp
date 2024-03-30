#include "country_file_parser.h"

CountryFileParser::CountryFileParser(QObject *parent)
    : QObject{parent}
{
    if ( openCtyFile() ) {
        // scanFileForSpecialChars();  // Test code: Isolate the non-alphanumeric characters used in file
        parseFileIntoRecords();
        if ( (parseEachRecord() == false) ) {
            qDebug() << "CountryFileParser::CountryFileParser: Failed to parse country data file:" << CTY_FILENAME;
            QApplication::exit(8);
        }
        // displayRecordList();

        // unitTestPartialScanner();
        // Q_ASSERT(false);
    }

    // Initialize class variables
    entry_in_progress = false;
}

void CountryFileParser::unitTestPartialScanner() {
    const static char a = 'A';
    const static char z = 'Z';
    const static char c_0 = '0';
    const static char c_9 = '9';

    QList<QString> prefix_pairs;
    QList<QString> prefix_triplets;

    for ( char i=a; i<=z; i++) {
        for ( char j=a; j<=z; j++) {
            QString s = QChar(i);
            s.append(QChar(j));
            prefix_pairs.append(s);
        }
    }
    // qDebug().noquote() << prefix_pairs;
    qDebug() << prefix_pairs.size();

    for ( char i=a; i<=z; i++) {
        for ( char j=a; j<=z; j++) {
            for ( char k = c_0; k < c_9; k++ ) {
                QString s = QChar(i);
                s.append(QChar(j));
                s.append(QChar(k));
                prefix_triplets.append(s);
            }
        }
    }

    for ( char i=a; i<=z; i++) {
        for ( char j=c_0; j <= c_9; j++) {
            for (  char k = a; k < z; k++ ) {
                QString s = QChar(i);
                s.append(QChar(j));
                s.append(QChar(k));
                prefix_triplets.append(s);
            }
        }
    }

    for ( char i=c_0; i <= c_9; i++) {
        for ( char j=a; j<=z; j++) {
            for (  char k = a; k < z; k++ ) {
                QString s = QChar(i);
                s.append(QChar(j));
                s.append(QChar(k));
                prefix_triplets.append(s);
            }
        }
    }

    QListIterator<QString> m(prefix_pairs);
    while ( m.hasNext() ) {
        QString s = m.next();
        if ( lookupPartial(s) )
            qDebug() << "Matched pair:" << s;
    }

    QListIterator<QString> n(prefix_triplets);
    while ( n.hasNext() ) {
        QString s = n.next();
        if ( lookupPartial(s) )
                qDebug() << "Matched Triplet:" << s;
    }
}

CountryFileParser::~CountryFileParser() {

}

#define MAX_READ_SIZE 1024
bool CountryFileParser::openCtyFile() {

    bool rc;
    int bytes_read, total_bytes;
    int filesize;
    char b[MAX_READ_SIZE];

    QFile f(CTY_FILENAME);

    rc = f.open(QIODeviceBase::ReadOnly);
    if ( !rc ) {
        qDebug() << "CountryFileParser::openCtyFile(): Open Failed:" << f.errorString();
        return false;
    }
    filesize = f.size();
    qDebug() << "CountryFileParser::openCtyFile(): file size is:" << filesize;

    total_bytes = 0;
    while ( (bytes_read = f.read(b, MAX_READ_SIZE)) ) {
        if ( bytes_read == -1 ) {
            qDebug() << "CountryFileParser::openCtyFile(): FILE READ FAILED:" << f.errorString();
            f.close();
            return false;
        }
        total_bytes += bytes_read;
        // qDebug() << bytes_read << "end of buff" << b[bytes_read-1] << "Char at end of buff:" << static_cast<unsigned char>(b[bytes_read-1]);
        file_buffer.append(b, bytes_read);
    }
    qDebug() << "  total bytes read:" << total_bytes << "QByteArray size:" << file_buffer.size();

    f.close();
    return true;
}

enum parse_state_e { LINE_START, RECORD_START, RECORD_END, COMMENT, SKIP_POUND };

bool CountryFileParser::parseFileIntoRecords() {

    // This index is our current pointer into the file_buffer QByteArray.
    int position_index = 0;
    int record_count = 0;
    parse_state_e p_state = LINE_START;
    int fbsize = file_buffer.size();

    int record_start = 0;
    while ( position_index < fbsize ) {
        char c = file_buffer.at(position_index);

        switch (p_state ) {
        case LINE_START:
            // Make sure this is not a Windows formatted file
            if ( c == '\r' ) {
                qDebug() << "CountryFileParser::parseFileIntoRecords(): \r FOUND PLEASE CONVERT TO UNIX STYLE FILE";
                Q_ASSERT(false);
            }
            // Discard any lines starting with #
            if ( c == '#')  {
                p_state = COMMENT;
            } else {
                if ( isAlpha(c) ) {
                    p_state = RECORD_START;
                    record_start = position_index;
                }
            }
            if ( c == Qt::Key_Space ) {
                qDebug() << "CountryFileParser::parseFileIntoRecords(): UNEXPECTED SPACE CHARACTER AT START OF LINE @" << position_index;
            }
            break;

        case RECORD_START:
            // Detect end of record (;)
            if ( c == ';' ) {
                QString s = file_buffer.mid(record_start, position_index - record_start + 1);
                country_records.append( s );
                record_count++;
                // qDebug().noquote() << "Record @: [" << position_index << "]" << s  << '\n';
                p_state = RECORD_END;
            }
            break;

        case RECORD_END:
            if ( c == '\n' ) {
                p_state = LINE_START;
            }
            break;

        case COMMENT:
            if ( c == '\n' ) {
                p_state = LINE_START;
            }
            break;

        default:
            qDebug() << "UNKNOWN STATE";
            break;

            qDebug() << "Buffer at position_index:" << position_index << c << "file_buffer size:" << fbsize;

            // Isolate a record - each record ends w/ a semicolon

        } // switch

        position_index++;
    }
    qDebug() << "CountryFileParser::parseFileIntoRecords(): Record count is:" << record_count;
    return true;
}

enum alias_state_e {START, SCAN, CQZ, ITUZ, SUFFIX, LATLON};
int CountryFileParser::processUniqueCall(QString &record, struct country_record_t &r, int position_index) {

    // We come into this function after having detecting a '=' char indicating unique call
    // Get a pointer to the QString data elements
    const QChar *p = record.constData();
    int rec_size = record.size();
    int field_start;
    alias_state_e a_state = START;

    while ( position_index < rec_size ) {
        const char c = p[position_index].toLatin1();

        switch (a_state) {
        case START:
            if ( isAlpha(c) || isDigit(c) ) {
                field_start = position_index;
                a_state = SCAN;
            }
            else {
                qDebug() << "CountryFileParser::processUniqueCall: ERROR: UNEXPECTED CHARACTER PARSING UNIQUE CALL:" << record << "Char:" << c;
                return -1;
            }
            break;

        case SCAN:
            switch (c) {
            case '(':   // CQ Zone detected
                a_state = CQZ;
                break;

            case '[':  // ITU Zone detected
                a_state = ITUZ;
                break;

            case '/':   // SUFFIX detected
                a_state = SUFFIX;
                break;

            case'<':   // LATLON detected
                a_state = LATLON;
                break;

            case ';':   // End of record detected - store it
                r.unique_calls.append(record.mid(field_start, position_index - field_start));
            }
            break;

        default:
            break;
        }
        position_index++;
    }

    return position_index;
}

bool CountryFileParser::isAlpha(char c) {

    if ( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') )
        return true;
    return false;
}

bool CountryFileParser::isDigit(char c) {

    if ( (c >= '0' && c <= '9') )
        return true;
    return false;
}

bool CountryFileParser::parseEachRecord() {

    // Walk through each record and scan / parse it
    QListIterator<QString> m(country_records);
    while ( m.hasNext() ) {

        // Process each record in the country_records list
        QString record = m.next();
        scanRecord(record);
    }
    return true;
}

enum scan_state_e {SR_START_OF_RECORD, SR_SCAN, SR_FIELD_START, SR_FIELD_TERMINATE, SR_COMMENT, SR_ALIAS};
enum scan_fields_e {F_START, F_CTY, F_CQZ, F_ITZ, F_CONT, F_LAT, F_LON, F_OFFSET, F_PRIMARY, F_ALIAS, F_UNIQUE};
void CountryFileParser::scanRecord(QString record) {

    // Characters in file other than alphanumeric: # : / . - _ ' ; , & < > = * ( ) [ ] " ~
    //  Relevant characters: # : / ; , < > = * ( ) [ ] \ ~
    //  ' " (single and double quote) not significant, used in names and contractions
    //  & used only in names
    //  * used to indicate coutry is on DARC WAEDC list
    //  # indicates comment
    //  : indicates end of field in a country record
    //  / inside angle brackets separates lat/lon override
    //  / outside of angle brackets indicates callsign suffix
    //  ; indicates end of country record.
    //  , separates aliases
    //  <> indicates lat/lon
    //  () indicated CQ Zone
    //  [] indicates ITU Zone
    //  {} is a continent override
    //  ~ is used as a local utc offset override
    //
    // The following special characters can be applied after an alias prefix:
    //  (#)	Override CQ Zone
    //  [#]	Override ITU Zone
    //  <#/#>	Override latitude/longitude
    //  {aa}	Override Continent
    //  ~#~	Override local time offset from GMT

    struct country_record_t r = {};
    scan_state_e scan_state = SR_START_OF_RECORD;
    scan_fields_e fields_state = F_START;

    // Process each field in the  country records
    int rec_size = record.size();
    int position_index = 0;
    int field_start = 0;

    // Get a pointer to the QString data elements
    const QChar *p = record.constData();

    while ( position_index < rec_size ) {
        const char c = p[position_index].toLatin1();

        // Relevant characters  # : / ; , < > = * ( ) [ ] ~ \n
        switch(c) {
        case '#':
            scan_state = SR_COMMENT;
            break;

        case '\n':
            // New Line terminates a comment
            if ( scan_state == SR_COMMENT ) {
                scan_state = SR_SCAN;
            }
            break;

        case ':':
            // Which field were we processing?
            switch ( fields_state ) {

            case F_CTY: {
                QString s = record.mid(field_start, position_index - field_start);
                s = s.trimmed();
                r.country = s;
                fields_state = F_CQZ;
                scan_state = SR_SCAN;
                break;
            }

            case F_CQZ:
                r.CQ_Zone = record.mid(field_start, position_index - field_start);
                fields_state = F_ITZ;
                scan_state = SR_SCAN;
                break;

            case F_ITZ:
                r.ITU_Zone = record.mid(field_start, position_index - field_start);
                fields_state = F_CONT;
                scan_state = SR_SCAN;
                break;

            case F_CONT: {
                QString s = record.mid(field_start, position_index - field_start);
                s = s.trimmed();
                r.continent = s;
                fields_state = F_LAT;
                scan_state = SR_SCAN;
                break;
            }

            case F_LAT:
                r.lat = record.mid(field_start, position_index - field_start).toFloat();
                fields_state = F_LON;
                scan_state = SR_SCAN;
                break;

            case F_LON:
                r.lon = record.mid(field_start, position_index - field_start).toFloat();
                fields_state = F_OFFSET;
                scan_state = SR_SCAN;
                break;

            case F_OFFSET:
                r.utc_ofset = record.mid(field_start, position_index - field_start).toFloat();
                fields_state = F_PRIMARY;
                scan_state = SR_SCAN;
                break;

            case F_PRIMARY: {
                QString s = record.mid(field_start, position_index - field_start);
                s = s.trimmed();
                r.primary_dxcc_prefix.append(s);
                fields_state = F_ALIAS;
                scan_state = SR_SCAN;
                break;
            }

            case F_ALIAS: {
                // TODO: This code is never reached
                // fields_state = F_CTY;
                QString s = record.mid(field_start, position_index - field_start);
                s = s.trimmed();
                struct alias_t a = parseAliasPrefix(s);
                a.country = r.country;
                r.alias_prefixes.append(a);
                break;
            }

            default:
                break;

            } // switch fields_state

            // Character is a ':' - we're at end of primary prefix and probably at end of line
            // Protect against reading past end of record
            // if ( position_index < rec_size - 2 && record.at(position_index + 1) == '\n');
            // field_start = position_index + 2;
            break;

        case '/':
            break;

        case ';': {
            // End of record
            if ( fields_state == F_ALIAS ) {
                QString s = record.mid(field_start, position_index - field_start);
                s = s.trimmed();
                struct alias_t a = parseAliasPrefix(s);
                a.country = r.country;  // Link the prefixes to the country
                r.alias_prefixes.append(a);
                fields_state = F_CTY;
                scan_state = SR_SCAN;
            }

            if ( fields_state == F_UNIQUE ) {
                QString s = record.mid(field_start, position_index - field_start);
                s = s.trimmed();
                r.unique_calls.append(s);
                scan_state = SR_SCAN;
                fields_state = F_CTY;
            }

            country_record_list.append(r);
            break;
        }

        case ',': {
            // Indicates end of alias prefix or unique call sign field
            if ( fields_state == F_ALIAS ) {
            QString s = record.mid(field_start, position_index - field_start);
            s = s.trimmed();
            // Indicates end of alias prefix field, add alias to our list

            // Debug code TODO
            // if ( r.country.contains("Spain") )
            //     qDebug() << "Spain";

            struct alias_t a = parseAliasPrefix(s);
            a.country = r.country;  // Link the prefixes to the country
            r.alias_prefixes.append(a);
            scan_state = SR_SCAN;
            }

            if ( fields_state == F_UNIQUE ) {
                QString s = record.mid(field_start, position_index - field_start);
                s = s.trimmed();
                r.unique_calls.append(s);
                scan_state = SR_SCAN;
                fields_state = F_ALIAS;
            }

            break;
        }

        case '<':
            break;

        case '>':
            break;

        case '=':
            // Unique callsigns can end with a variety of characters including
            //  ';' semicolon, the simple case!
            //  ',' comma
            //  '(' Left paren (closed by right - CQ Zone override
            //  '<' left angle bracket (closed by right - Lat/Lon override
            //  '[' left square bracket (closed by right - ITU Zone override
            // Overrides including
            //    * CQ Zone '()'
            //    * ITU Zone '[]'
            //    * Lat/Lon '<>'
            // They can be in the middle of a line of alias prefixes, with or without overrides

            fields_state = F_UNIQUE;
            scan_state = SR_SCAN;
            break;

        case '*':
            break;

        case '(':
            break;

        case ')':
            break;

        case '[':
            break;

        case ']':
            break;

        case '~':
            break;

        default:
            if ( scan_state == SR_START_OF_RECORD || fields_state == F_START ) {
                if ( isAlpha(c) || isDigit(c) || c == Qt::Key_Space || c == '-') {
                    field_start = position_index;
                    fields_state = F_CTY;
                    scan_state = SR_SCAN;
                } else {
                    qDebug() << "CountryFileParser::scanRecord(): UNEXPECTED CHARACTER:" << c << static_cast<unsigned char>(c);
                    Q_ASSERT(false);
                }
            }

            if ( scan_state == SR_SCAN ) {
                if ( isAlpha(c) || isDigit(c) || c == '-' ) {
                    field_start = position_index;
                    scan_state = SR_FIELD_START;
                }
            }
            break;
        }

        position_index++;
    }
}

// For debug only
void CountryFileParser::scanFileForSpecialChars() {

    int bsize = file_buffer.size();
    int position_index = 0;
    QByteArray b;

    qDebug() << "CountryFileParser::scanFile(): buffer size:" << bsize;

    while ( position_index < bsize ) {
        char c = file_buffer.at(position_index);
        if ( !isAlpha(c) && !isDigit(c) && !(c == Qt::Key_Space) ) {
            if ( !b.contains(c) )
            b.append(c);
        }
        position_index++;
    }
    qDebug().noquote() << "Characters in file other than alphanumeric:" << b;

    for ( int i = 0; i<b.size(); i++ ) {
        qDebug().noquote() << b.at(i) << static_cast<unsigned char>(b.at(i));
    }
}

struct alias_t CountryFileParser::parseAliasPrefix(QString &alias) {
    // Alias records look like this:
    // 3H0A(23)[42]<43.75/-87.75>
    // They can have any, all or none of (CQZONE), [ITUZONE] or <lat/lon>

    struct alias_t a;

    if ( alias.contains('(') || alias.contains('<') || alias.contains('[') ) {
        // An Alias can contain any, all, or none of CQZone, ITUZone and lat/lon
        // We want to isolate just the prefix part here.
        int lp = alias.indexOf('(');
        int lb = alias.indexOf('[');
        int lab = alias.indexOf('<');
        int min = getLowestValidInt(lp, lb, lab);
        if ( min > 0 )
            a.prefix = alias.left(min);
    } else {
        a.prefix = alias;
    }

    // Isolate the CQ Zone if present
    if ( alias.contains('(') ) {
        // We have a CQ Zone
        int left_index = alias.indexOf('(');
        int right_index = alias.indexOf(')');
        left_index++;    // Position on the first character of CQ Zone
        a.CQ_Zone = alias.mid(left_index, right_index - left_index);
    }

    // Isolate the ITU Zone if present
    if ( alias.contains('[') ) {
        // We have a CQ Zone
        int left_index = alias.indexOf('[');
        int right_index = alias.indexOf(']');
        left_index++;    // Position on the first character of CQ Zone
        a.ITU_Zone = alias.mid(left_index, right_index - left_index);
    }

    // Isolate the Lat/Lon if present
    if ( alias.contains('<') ) {
        // We have a CQ Zone
        int left_index = alias.indexOf('<');
        int right_index = alias.indexOf('/');   // Lat/Lon are separated by
        left_index++;    // Position on the first character of CQ Zone
        a.lat = alias.mid(left_index, right_index - left_index).toFloat();
        left_index = right_index + 1;
        right_index = alias.indexOf('>');
        a.lon = alias.mid(left_index, right_index - left_index).toFloat();
    }
    return a;
}

int CountryFileParser::getLowestValidInt(int lp, int lb, int lab) {

    int smallest = 500; // Randomly large number

    // Could be -1
    // We assume we only get called in here if at least one of the parameters
    // is greater than zero.  See the caller for details.

    if ( lp > 0 && lp < smallest )
        smallest = lp;

    if ( lb > 0 && lb < smallest )
        smallest = lb;

    if (lab > 0 && lab < smallest )
        smallest = lab;

     return smallest;
}

bool CountryFileParser::displayRecordList() {

    // country_record_list
    struct country_record_t r;

    QListIterator<struct country_record_t> m(country_record_list);
    while ( m.hasNext() ) {
        r = m.next();
        qDebug().noquote() << "Country:" << r.country << "CQ ZONE:" << r.CQ_Zone << "ITU Zone:" << r.ITU_Zone << "Lat/Lon:" << r.lat << r.lon << "Uniques:" << r.unique_calls.size();

        if ( 1 || r.country == "Vienna Intl Ctr" || r.country.contains("Spain")) {
            if ( !r.unique_calls.isEmpty() )
                qDebug().noquote() << "Uniques:" << r.unique_calls;

            if ( !r.alias_prefixes.isEmpty() ) {
                QListIterator<struct alias_t> i(r.alias_prefixes);
                QString tmpS;
                while ( i.hasNext() ) {
                    struct alias_t a = i.next();
                    QString s;
                    s.append(a.country + ": ");
                    s.append(a.prefix + ": ");
                    if ( a.CQ_Zone != "" )
                        s.append("(" + a.CQ_Zone + ") ");

                    if ( a.ITU_Zone != "" )
                        s.append("[" + a.ITU_Zone + "] ");

                    if ( a.lat != 0.0 ) {
                        s.append("<" + QString::number(a.lat) + "/");
                        s.append(QString::number(a.lon) + "> ");
                    }
                    tmpS.append(s);
                }
                qDebug().noquote() << "Aliases:" << tmpS << '\n';
            }
        }

        // qDebug() << "Aliases:" << r.alias_prefixes;
    }
    return true;
}

bool CountryFileParser::lookupPartial(const QString arg1, CallSignLookup *pCallSign) {

    // Lookup callsign
    // There are two possible outcomes:
    //   1) prefix is one of the primary dxcc prefixes
    //   2) prefix is one of the aliases.  Both must be checked.
    // qDebug() << "CountryFileParser::lookupPartial() Entered: arg1:" << arg1;

    bool primary_match = false;
    bool alias_match = false;
    bool unique_match = false;

    if ( pCallSign != nullptr ) { // Accomodate some unit testing
        if ( !entry_in_progress ) {
            pCallSign->call_sign_line_edit_p->clear();
            pCallSign->country_text_edit_p->clear();
            pCallSign->prefix_text_edit_p->clear();
            pCallSign->unique_line_edit_p->clear();
            pCallSign->country_text_edit_p->update();
            pCallSign->prefix_text_edit_p->update();
            pCallSign->unique_line_edit_p->update();
            pCallSign->call_sign_line_edit_p->update();
        }
    }

    if ( arg1.isEmpty() )
        return false;

    QMap<QString, QString> country_list;
    QMap<QString, QString> prefix_list;
    QMap<QString, QString> uniques_list;

    if ( pCallSign != nullptr ) { // Accomodate some unit testing
        pCallSign->call_sign_line_edit_p->setText(arg1);
    }

    // *****************  PRIMARY FIRST  *********************
    // First see if the prefix matches any of the "Primary" dxcc prefixes

    QListIterator<struct country_record_t> m(country_record_list);
    int c_count = 0;
    int p_count = 0;
    while ( m.hasNext() ) {
        struct country_record_t r;
        c_count++;
        r = m.next();
        if ( r.primary_dxcc_prefix.startsWith(arg1) ) {
            QString s = r.country;
            primary_match = true;
            country_list.insert(s, "");
            qDebug().noquote() << "CountryFileParser::lookupPartial(): Primary: arg1:" << arg1 << "Country:" << s;

            if ( pCallSign != nullptr ) {// Accomodate some unit testing
                pCallSign->prefix_text_edit_p->append(r.primary_dxcc_prefix);
            }
        }

        // ***************** PREFIX / ALIAS  **********************
        // Find compabitle prefixes based on arg1
        // Each country record contains a list of aliases
        // Here is the alias list.  Let's see if there's a match
        QListIterator<struct alias_t> b(r.alias_prefixes);
        while ( b.hasNext() ) {
            struct alias_t alias = b.next();
            if ( alias.prefix.startsWith(arg1) ) {
                country_list.insert(alias.country, "");
                prefix_list.insert(alias.prefix, "");
                qDebug().noquote() << "CountryFileParser::lookupPartial(): Prefix: arg1:" << arg1 << "Prefix:" << alias.prefix << "Country:" << alias.country;
                alias_match = true;
            }
        }
        qDebug() << "CountryFileParser::lookupPartial(): Prefix: country_list size:" << country_list.size() << "prefix list size:" << prefix_list.size();


        // ****************  UNIQUE  ******************
        // Each country record contains a list of uniqe calls
        // Here is the uniques list.  Let's see if there's a match
        QListIterator<QString> n(r.unique_calls);
        int len = arg1.size();
        while ( n.hasNext() ) {
            QString s_unique = n.next();
            QString unique_stripped = stripUnique(s_unique);
            if ( unique_stripped.left(len) == arg1 ) {
                uniques_list.insert(unique_stripped, "");
                unique_match = true;
            }
        }
    }
    qDebug() << "CountryFileParser::lookupPartial():" << c_count << "Country records processed - country_list size:" << country_list.size();

    // Now that we've found all the countries and prefixes, display them
    if ( pCallSign != nullptr ) { // Accomodate some unit testing
        QList<QString> country_keys = country_list.keys();
        QListIterator<QString> l(country_keys);
        c_count = 0;
        while ( l.hasNext() ) {
            QString key = l.next();
            pCallSign->country_text_edit_p->append(key);
            c_count++;
        }

        QList<QString> prefix_keys = prefix_list.keys();
        QListIterator<QString> k(prefix_keys);
        while ( k.hasNext() ) {
            QString key = k.next();
            pCallSign->prefix_text_edit_p->append(key);
            p_count++;
        }

        QList<QString> uniques_keys = uniques_list.keys();
        QListIterator<QString> u(uniques_keys);
        QString results;
        while ( u.hasNext() ) {
            QString key = u.next();
            results.append(key + " ");
        }
        pCallSign->unique_line_edit_p->setText(results);
    }
    qDebug() << "CountryFileParser::lookupPartial(): Prefix: country count" << c_count << "prefix count" << p_count;

    if ( primary_match || alias_match || unique_match )
        return true;

    return false;
}

void CountryFileParser::callsignEditBoxReportEmpty() {
    qDebug() << "CountryFileParser::callsignEditBoxReportEmpty(): report empty";
    entry_in_progress = false;
    pCallSign->call_sign_line_edit_p->clear();
    pCallSign->country_text_edit_p->clear();
    pCallSign->prefix_text_edit_p->clear();
    pCallSign->unique_line_edit_p->clear();
    pCallSign->country_text_edit_p->update();
    pCallSign->prefix_text_edit_p->update();
    pCallSign->unique_line_edit_p->update();
    pCallSign->call_sign_line_edit_p->update();
}

QString CountryFileParser::stripUnique(QString &s) {

    // TODO this is duplicated code
    QString a;
    if ( s.contains('(') || s.contains('<') || s.contains('[') ) {
        // An Alias can contain any, all, or none of CQZone, ITUZone and lat/lon
        // We want to isolate just the prefix part here.
        int lp = s.indexOf('(');
        int lb = s.indexOf('[');
        int lab = s.indexOf('<');
        int min = getLowestValidInt(lp, lb, lab);
        if ( min > 0 )
            a = s.left(min);
    } else {
        a = s;
    }
    return a;
}

void CountryFileParser::setCallsignWindowP(CallSignLookup *callsign_window_p) {
    pCallSign = callsign_window_p;
}
