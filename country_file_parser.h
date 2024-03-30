#ifndef COUNTRYFILEPARSER_H
#define COUNTRYFILEPARSER_H

#include <QObject>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QByteArray>
#include <QApplication>
#include <QString>

#include "callsign_lookup.h"

// Source for N1MM cty file: https://www.country-files.com/contest/n1mm/
// N1MM Downloads: https://n1mmwp.hamdocs.com/mmfiles/
// Format:
// COUNTRY, CQ ZONE, ITU ZONE, CONTINENT, Lat, Lon, UTC_OFFSET, BasePREFIX
//  other prefixes and actual known calls (=)
// Semicolon ';' marks the end of every record

#define CTY_FILENAME "/Users/chris/sandbox/QtProjects/winkeyer-contest/cty_wt_mod.dat"

struct alias_t {
    QString country = "";
    QString prefix = "";
    QString CQ_Zone = "";
    QString ITU_Zone = "";
    float lat = 0.0;
    float lon = 0.0;
};

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
    QList<struct alias_t> alias_prefixes;
    QList<QString> unique_calls;
};

class CountryFileParser : public QObject
{
    Q_OBJECT
public:
    explicit CountryFileParser(QObject *parent = nullptr);
    ~CountryFileParser();
    bool openCtyFile();
    bool lookupPartial(const QString arg1, CallSignLookup *pCallSign = nullptr);
    void setCallsignWindowP(CallSignLookup *callsign_window_p);

private:
    bool parseFileIntoRecords();
    QString stripUnique(QString &s);

    // File format: https://www.country-files.com/cty-dat-format
    bool parseEachRecord();
    void scanRecord(QString record);
    void scanFileForSpecialChars();
    alias_t parseAliasPrefix(QString &alias);
    bool displayRecordList();
    QList<QString> country_records;
    inline bool isAlpha(char c);
    inline bool isDigit(char c);
    int processUniqueCall(QString &record, struct country_record_t &r, int position_index);
    int getLowestValidInt(int lp, int lb, int lab);
    QString active_country;
    QString active_prefix;
    bool entry_in_progress;

    // For debug TODO
    void unitTestPartialScanner();

public slots:
    void callsignEditBoxReportEmpty();

private:
    QByteArray file_buffer;
    QList<struct country_record_t> country_record_list;
    CallSignLookup *pCallSign;

signals:
};

#endif // COUNTRYFILEPARSER_H
