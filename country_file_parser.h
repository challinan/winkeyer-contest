#ifndef COUNTRYFILEPARSER_H
#define COUNTRYFILEPARSER_H

#include <QObject>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QByteArray>
#include <QApplication>

// Source for N1MM cty file: https://www.country-files.com/contest/n1mm/
// N1MM Downloads: https://n1mmwp.hamdocs.com/mmfiles/
// Format:
// COUNTRY, CQ ZONE, ITU ZONE, CONTINENT, Lat, Lon, UTC_OFFSET, BasePREFIX
//  other prefixes and actual known calls (=)
// Semicolon ';' marks the end of every record

#define CTY_FILENAME "/Users/chris/sandbox/QtProjects/winkeyer-contest/cty_wt_mod.dat"

struct country_record_t {
    QString country = "";
    QString CQ_Zone = "";
    QString ITU_Zone = "";
    QString continent = "";
    float lat = 0.0;
    float lon = 0.0;
    bool darc_waedc_list = false;
    float utc_ofset = 0.0;
    QString primary_dxcc_prefix = "";
    QList<QString> alias_prefixes;
    QList<QString> unique_calls;
};

class CountryFileParser : public QObject
{
    Q_OBJECT
public:
    explicit CountryFileParser(QObject *parent = nullptr);
    ~CountryFileParser();
    bool openCtyFile();

private:
    bool parseFileIntoRecords();

    // File format: https://www.country-files.com/cty-dat-format
    bool parseEachRecord();
    void scanRecord(QString record);
    void scanFileForSpecialChars();
    void displayRecordList();
    QList<QString> country_records;
    inline bool isAlpha(char c);
    inline bool isDigit(char c);
    int processUniqueCall(QString &record, struct country_record_t &r, int position_index);

private:
    QByteArray file_buffer;
    QList<struct country_record_t> country_record_list;

signals:
};

#endif // COUNTRYFILEPARSER_H
