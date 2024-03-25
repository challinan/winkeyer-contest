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
        displayRecordList();
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
                // fields_state = F_CTY;
                QString s = record.mid(field_start, position_index - field_start);
                s = s.trimmed();
                r.alias_prefixes.append(s);
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
                r.alias_prefixes.append(s);
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
            r.alias_prefixes.append(s);
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
            if ( r.country.contains("Spain"))
                qDebug() << "Spain";
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

void CountryFileParser::displayRecordList() {

    // country_record_list
    struct country_record_t r;

    QListIterator<struct country_record_t> m(country_record_list);
    while ( m.hasNext() ) {
        r = m.next();
        qDebug() << "Country:" << r.country << "Uniques:" << r.unique_calls.size();
        if ( r.country == "Vienna Intl Ctr" || r.country.contains("Spain")) {
            qDebug() << "Uniques:" << r.unique_calls;
            qDebug() << "Aliases:" << r.alias_prefixes;
        }

        // qDebug() << "Aliases:" << r.alias_prefixes;
    }
}
