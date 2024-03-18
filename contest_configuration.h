#ifndef CONTESTCONFIGURATION_H
#define CONTESTCONFIGURATION_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QSqlDatabase>
#include <QDebug>

#include "sqlite3-connector.h"

/* This class manages the diffirent configurations required for specific contest types
 *   hopefully the design will be flexible enough to manage any contest with
 *   the appropriate exchange data, rules and even, yes, points calculation someday
 *
 *    Here are some examples of contests and their exchanges
 *    FIELD DAY: ARRL/RAC members: Station Class and ARRL Section
 *    ARRL CW SWEEPSTAKES: Seq#, Precedence, Callsign, Check (year of being licensed) and ARRL Section
 *    ARRL ROOKIE ROUNDUP: Callsign, Name, 2-digit yr first licensed, state (or canadian province, mexican call area or DX)
 *    CQ WORLDWIDE DX CONTEST CW: RST + CQ Zone
 *    FLORIDA QSO PARTY: RS(T) and Location: (FL Ops send county, US Ops send state, CA send province, DX: DXCC Prefix)
 *    ARRL 10 METER CONTEST:  CW or SSB: RST + State (Canadians send Province, Mexico send State/Province)
 *    ARRL 160 METER CONTEST: CW Only: RST + Section.  DX signal report only
 *    ARRL INT'L DX CONTEST: W/VE RST State/Province, DX stations send RST and Power (can be abbreviated)
 *
 * Each contest has it's own set of rules for scoring, with multipliers, power, modes, multi, entry categories
 *   etc. Some provision to store contest rules and attributes needs to be designed
 *
 */

enum modes_e {CW, USB, LSB, FM, DIGI, RTTY, MIXED};
enum state_e {UNKNOWN_MODE, RUN_MODE, SNP_MODE};

struct us_state_abbrev_t {
    QString full_state;
    QString two_letter_abbrev;
};

struct arrl_sections_t {
    QString full_section;
    QString section_abbrev;
};

struct cabrillo_contest_names_t {
    int contest_id;
    QString contest;
    QString cabrillo_name;
};

struct contest_data_t {
    QString contest_name;
    bool has_sequence;
    bool has_rst;
    bool has_section;
    bool has_state;
    bool has_precedence;
    bool has_member_number;
    bool has_cqzone;
    bool has_ituzone;
    bool has_age_group; // OM, UL, Youth YL, or Youth
    bool has_name;
    modes_e runMode;
};

struct func_key_t {
    state_e run_state;
    QString functionKey;
    QString label;
    QString exchange;
};

class ContestConfiguration : public QObject
{
    Q_OBJECT
public:
    explicit ContestConfiguration(QObject *parent = nullptr, Sqlite3_connector *p = nullptr);
    bool createGlobalContestTable();
    bool getDatabaseConnection();
    state_e getRunMode();
    void setRunMode(state_e mode);

private:
    bool readCwDefaultMsgFile();
    void displayFunctionKeyMap();

public:
    QList<struct func_key_t> cwFuncKeys;

private:
    Sqlite3_connector *db;
    QSqlDatabase sqlDatabase;
    QString dbpath;
    state_e runMode;

    const QMap<QString, struct contest_data_t> contest_list;
    const QList<struct us_state_abbrev_t> us_states_list {
        {"Alabama", "AL"},
        {"Alaska", "AK"},
        {"American Samoa", "AS"},
        {"Arizona", "AZ"},
        {"Arkansas", "AR"},
        {"California", "CA"},
        {"Colorado", "CO"},
        {"Connecticut", "CT"},
        {"Delaware", "DE"},
        {"District of Columbia", "DC"},
        {"Florida", "FL"},
        {"Georgia", "GA"},
        {"Guam", "GU"},
        {"Hawaii", "HI"},
        {"Idaho", "ID"},
        {"Illinois", "IL"},
        {"Indiana", "IN"},
        {"Iowa", "IA"},
        {"Kansas", "KS"},
        {"Kentucky", "KY"},
        {"Louisiana", "LA"},
        {"Maine", "ME"},
        {"Maryland", "MD"},
        {"Massachusetts", "MA"},
        {"Michigan", "MI"},
        {"Minnesota", "MN"},
        {"Mississippi", "MS"},
        {"Missouri", "MO"},
        {"Montana", "MT"},
        {"Nebraska", "NE"},
        {"Nevada", "NV"},
        {"New Hampshire", "NH"},
        {"New Jersey", "NJ"},
        {"New Mexico", "NM"},
        {"New York", "NY"},
        {"North Carolina", "NC"},
        {"North Dakota", "ND"},
        {"Ohio", "OH"},
        {"Oklahoma", "OK"},
        {"Oregon", "OR"},
        {"Pennsylvania", "PA"},
        {"Puerto Rico", "PR"},
        {"Rhode Island", "RI"},
        {"South Carolina", "SC"},
        {"South Dakota", "SD"},
        {"Tennessee", "TN"},
        {"Texas", "TX"},
        {"Utah", "UT"},
        {"Vermont", "VT"},
        {"Virgin Islands", "VI"},
        {"Virginia", "VA"},
        {"Washington", "WA"},
        {"West Virginia", "WV"},
        {"Wisconsin", "WI"},
        {"Wyoming", "WY"}

    };
    const QList<struct arrl_sections_t> arrl_section_list {
        { "Colorado", "CO"},
        { "Iowa", "IA"},
        { "Kansas", "KS"},
        { "Minnesota", "MN"},
        { "Missouri", "MO"},
        { "North Dakota", "ND"},
        { "Nebraska", "NE"},
        { "South Dakota", "SD"},
        { "Connecticut", "CT"},
        { "Eastern Massachusetts", "EMA"},
        { "Maine", "ME"},
        { "New Hampshire", "NH"},
        { "Rhode Island", "RI"},
        { "Vermont", "VT"},
        { "Western Massachusetts", "WMA"},
        { "Eastern New York", "ENY"},
        { "New York City-Long Island", "NLI"},
        { "Northern New Jersey", "NNJ"},
        { "Northern New York", "NNY"},
        { "Southern New Jersey", "SNJ"},
        { "Western New York", "WNY"},
        { "Delaware", "DE"},
        { "Eastern Pennsylvania", "EPA"},
        { "Maryland-DC", "MDC"},
        { "Western Pennsylvania", "WPA"},
        { "Alabama", "AL"},
        { "Georgia", "GA"},
        { "Kentucky", "KY"},
        { "North Carolina", "NC"},
        { "Northern Florida", "NFL"},
        { "Puerto Rico", "PR"},
        { "South Carolina", "SC"},
        { "Southern Florida", "SFL"},
        { "Tennessee", "TN"},
        { "Virginia", "VA"},
        { "US Virgin Islands", "VI"},
        { "West Central Florida", "WCF"},
        { "Arkansas", "AR"},
        { "Louisiana", "LA"},
        { "Mississippi", "MS"},
        { "New Mexico", "NM"},
        { "North Texas", "NTX"},
        { "Oklahoma", "OK"},
        { "South Texas", "STX"},
        { "West Texas", "WTX"},
        { "East Bay", "EB"},
        { "Los Angeles", "LAX"},
        { "Orange", "ORG"},
        { "Pacific", "PAC"},
        { "Santa Barbara", "SB"},
        { "Santa Clara Valley", "SCV"},
        { "San Diego", "SDG"},
        { "San Francisco", "SF"},
        { "San Joaquin Valley", "SJV"},
        { "Sacramento Valley", "SV"},
        { "Alaska", "AK"},
        { "Arizona", "AZ"},
        { "Eastern Washington", "EWA"},
        { "Idaho", "ID"},
        { "Montana", "MT"},
        { "Nevada", "NV"},
        { "Oregon", "OR"},
        { "Utah", "UT"},
        { "Western Washington", "WWA"},
        { "Wyoming", "WY"},
        { "Michigan", "MI"},
        { "Ohio", "OH"},
        { "West Virginia", "WV"},
        { "Illinois", "IL"},
        { "Indiana", "IN"},
        { "Wisconsin", "WI"},
        { "Alberta", "AB"},
        { "British Columbia", "BC"},
        { "Ontario Golden Horseshoe", "GH"},
        { "Manitoba", "MB"},
        { "New Brunswick", "NB"},
        { "Newfoundland and Labrador", "NL"},
        { "Nova Scotia", "NS"},
        { "Ontario East", "ONE"},
        { "Ontario North", "ONN"},
        { "Ontario South", "ONS"},
        { "Prince Edward Island", "PE"},
        { "Quebec", "QC"},
        { "Saskatchewan", "SK"},
        { "Territories", "TER"}
    };

public:
    // Source: https://www.contestcalendar.com/cabnames.php
    const QList<struct cabrillo_contest_names_t> cabrillo_contest_data {
        {156, "10-10 Int. 10-10 Day Sprint", "10-10-SPRINT"},
        {173, "10-10 Int. Fall Contest, CW", "10-10-FALL-CW"},
        {464, "10-10 Int. Fall Contest, Digital", "10-10-FALL-DIGITAL"},
        {516, "10-10 Int. Open Season PSK Contest", "10-10-OPEN-SEASON"},
        {5, "10-10 Int. Spring Contest, CW", "10-10-SPRING-CW"},
        {453, "10-10 Int. Spring Contest$ Digital", "10-10-SPRING-DIGITAL"},
        {81, "10-10 Int. Summer Contest, SSB", "10-10-SUMMER-PHONE"},
        {237, "10-10 Int. Winter Contest, SSB", "10-10-WINTER-PHONE"},
        {404, "7th Call Area QSO Party", "7QP"},
        {715, "A1Club AWT", "A1CLUB-AWT"},
        {578, "Africa All Mode International DX Contest", "ALL-AFRICA"},
        {7, "AGCW QRP/QRP Party", "AGCW-QRP"},
        {134, "Alabama QSO Party", "AL-QSO-PARTY"},
        {47, "All Asian DX Contest, CW", "AADX-CW"},
        {102, "All Asian DX Contest, Phone", "AADX-SSB"},
        {719, "ANZAC Day Contest", "ANZAC"},
        {686, "ARI 40/80 Contest", "40-80"},
        {9, "ARI International DX Contest", "ARI-DX"},
        {482, "Arizona QSO Party", "AZ-QSO-PARTY"},
        {132, "Arkansas QSO Party", "AR-QSO-PARTY"},
        {88, "ARRL 10 GHz and Up Contest", "ARRL-10-GHZ"},
        {199, "ARRL 10-Meter Contest", "ARRL-10"},
        {194, "ARRL 160-Meter Contest", "ARRL-160"},
        {615, "ARRL 222 MHz and Up Distance Contest", "ARRL-222"},
        {417, "ARRL EME Contest", "ARRL-EME"},
        {57, "ARRL Field Day", "ARRL-FD"},
        {716, "ARRL Inter. Digital Contest", "ARRL-DIGI"},
        {256, "ARRL Inter. DX Contest, CW", "ARRL-DX-CW"},
        {289, "ARRL Inter. DX Contest, SSB", "ARRL-DX-SSB"},
        {231, "ARRL January VHF Contest", "ARRL-VHF-JAN"},
        {43, "ARRL June VHF Contest", "ARRL-VHF-JUN"},
        {502, "ARRL Rookie Roundup, CW", "ARRL-RR-CW"},
        {501, "ARRL Rookie Roundup, RTTY", "ARRL-RR-DIG"},
        {500, "ARRL Rookie Roundup, SSB", "ARRL-RR-PH"},
        {217, "ARRL RTTY Roundup", "ARRL-RTTY"},
        {254, "ARRL School Club Roundup", "ARRL-SCR"},
        {113, "ARRL September VHF Contest", "ARRL-VHF-SEP"},
        {177, "ARRL Sweepstakes Contest, CW", "ARRL-SS-CW"},
        {178, "ARRL Sweepstakes Contest, SSB", "ARRL-SS-SSB"},
        {164, "Asia-Pacific Fall Sprint, CW", "AP-SPRINT"},
        {42, "Asia-Pacific Sprint, SSB", "AP-SPRINT"},
        {714, "Australia Day Contest", "WIA-AUSTRALIADAY"},
        {599, "Balkan HF Contest", "BALKAN-HF"},
        {28, "Baltic Contest", "BALTIC-CONTEST"},
        {309, "BARTG HF RTTY Contest", "BARTG-RTTY"},
        {230, "BARTG RTTY Sprint", "BARTG-SPRINT"},
        {513, "BARTG Sprint 75", "BARTG-SPRINT"},
        {703, "BARTG Sprint PSK63 Contest", "BARTG-SPRINT"},
        {754, "Batavia DX Contest", "BATAVIA"},
        {677, "Batavia FT8 Contest", "BATAVIA"},
        {269, "British Columbia QSO Party", "BC-QSO-PARTY"},
        {427, "Bucharest Digital Contest", "BUCURESTI-DIGITAL"},
        {140, "California QSO Party", "CA-QSO-PARTY"},
        {717, "Canadian Prairies QSO Party", "CP-QSO-PARTY"},
        {122, "Collegiate QSO Party", "CQP"},
        {431, "Colorado QSO Party", "COQP"},
        {673, "COVID-19 Radio Communication Event", "STAYHOME"},
        {232, "CQ 160-Meter Contest, CW", "CQ-160-CW"},
        {259, "CQ 160-Meter Contest, SSB", "CQ-160-SSB"},
        {192, "CQ Worldwide DX Contest, CW", "CQ-WW-CW"},
        {130, "CQ Worldwide DX Contest, RTTY", "CQ-WW-RTTY"},
        {172, "CQ Worldwide DX Contest, SSB", "CQ-WW-SSB"},
        {73, "CQ Worldwide VHF Contest", "CQ-VHF"},
        {245, "CQ WW RTTY WPX Contest", "CQ-WPX-RTTY"},
        {29, "CQ WW WPX Contest, CW", "CQ-WPX-CW"},
        {291, "CQ WW WPX Contest, SSB", "CQ-WPX-SSB"},
        {14, "CQ-M International DX Contest", "CQ-M"},
        {21, "CQMM DX Contest", "CQMMDX"},
        {206, "Croatian DX Contest", "9A-DX"},
        {534, "CVA DX Contest, CW", "CVA-DX-CW"},
        {535, "CVA DX Contest, SSB", "CVA-DX-SSB"},
        {532, "CWOps CW Open", "CW-OPEN"},
        {498, "CWops Test (CWT)", "CW-OPS"},
        {223, "DARC 10-Meter Contest", "DARC-10"},
        {210, "DARC Christmas Contest", "XMAS"},
        {668, "DARC Easter Contest", "EASTER"},
        {745, "DARC FT4 Contest", "FT4"},
        {744, "DARC RTTY Sprint", "ShortRY"},
        {240, "Delaware QSO Party", "DE-QSO-PARTY"},
        {691, "DIG QSO Party, CW", "DIG-QSO-PARTY"},
        {690, "DIG QSO Party, SSB", "DIG-QSO-PARTY"},
        {568, "DRCG WW RTTY Contest", "DRCG-WW-RTTY"},
        {249, "Dutch PACC Contest", "PACC"},
        {693, "Dutch PACCdigi Contest", "PACCDIGI"},
        {403, "EA PSK63 Contest", "EA-PSK"},
        {313, "EA RTTY Contest", "EARTTY"},
        {550, "EANET Sprint", "EANET-SPRINT"},
        {27, "EU PSK DX Contest", "EU-PSK-DX"},
        {219, "EUCW 160m Contest", "EUCW-160"},
        {660, "EurAsia HF Championship", "EURASIA-CHAMP"},
        {82, "European HF Championship", "EUHFC"},
        {687, "European Union DX Contest", "EUDXC"},
        {488, "F9AA Cup, CW", "F9AA-CW"},
        {577, "F9AA Cup, PSK", "F9AA-DIGI"},
        {489, "F9AA Cup, SSB", "F9AA-SSB"},
        {692, "FIRAC HF Contest", "FIRAC-CONTEST"},
        {325, "Florida QSO Party", "FCG-FQP"},
        {636, "FT Roundup", "FT8-RU"},
        {45, "GACW WWSA CW DX Contest", "WWSA"},
        {328, "Georgia QSO Party", "GA-QSO-PARTY"},
        {732, "Gunung Jati DX Contest", "GUNUNGJATI"},
        {449, "HA3NS Sprint Memorial Contest", "HA3NS-SPRINT"},
        {729, "Ham Spirit Contest, CW", "HAM-SPIRIT-CW"},
        {730, "Ham Spirit Contest, SSB", "HAM-SPIRIT-SSB"},
        {742, "HARC Anniversary QSO Party", "HARC-QSOP"},
        {96, "Hawaii QSO Party", "HI-QSO-PARTY"},
        {326, "Helvetia Contest", "HELVETIA"},
        {651, "Hiram Percy Maxim Birthday Celebration", "ARRL-HPM-150"},
        {23, "His Maj. King of Spain Contest, CW", "EA-MAJESTAD-CW"},
        {59, "His Maj. King of Spain Contest, SSB", "EA-MAJESTAD-SSB"},
        {319, "Holyland DX Contest", "WWHC"},
        {228, "Hungarian DX Contest", "HA-DX"},
        {67, "IARU HF World Championship", "IARU-HF"},
        {720, "ICWC Medium Speed Test", "ICWC-MST"},
        {305, "Idaho QSO Party", "ID-QSO-PARTY"},
        {688, "IG-RY World Wide RTTY Contest", "IG-WW-RY"},
        {167, "Illinois QSO Party", "IL-QSO-PARTY"},
        {743, "INDEXA Worldwide QSO Party", "INDEXA-QSOP"},
        {8, "Indiana QSO Party", "IN-QSO-PARTY"},
        {484, "Iowa QSO Party", "IAQP"},
        {758, "IRTS 2m Counties Contest", "IRTS-CONTEST"},
        {757, "IRTS 70cm Counties Contest", "IRTS-CONTEST"},
        {756, "IRTS 80m Evening Counties Contest", "IRTS-80-CONTEST"},
        {161, "JARTS WW RTTY Contest", "JARTS-WW-RTTY"},
        {314, "JIDX CW Contest", "JIDX-CW"},
        {184, "JIDX Phone Contest", "JIDX-SSB"},
        {734, "Kalbar Contest", "KALBAR CONTEST"},
        {675, "KANHAM Contest", "KANHAM"},
        {483, "Kansas QSO Party", "KS-QSO-PARTY"},
        {244, "KCJ Topband Contest", "KCJ-TOPBAND"},
        {371, "Kentucky QSO Party", "KYQP"},
        {759, "LABRE-RS Digi Contest", "CQWW-DIGI"},
        {250, "Louisiana QSO Party", "LA-QSO-PARTY"},
        {187, "LZ DX Contest", "LZ-DX"},
        {736, "Maidenhead Mayhem Contest", "MAIDENHEAD-MAYHEM"},
        {735, "Maidenhead Mayhem Sprint", "MAIDENHEAD-MAYHEM"},
        {592, "Maine QSO Party", "ME-QSO-PARTY"},
        {159, "Makrothen RTTY Contest", "MAKROTHEN-RTTY"},
        {658, "Malaysia DX Contest", "MYDX-SSB-CONTEST"},
        {710, "Marconi Club ARI Loano QSO Party Day", "MCD-QSO-PARTY"},
        {56, "Marconi Memorial HF Contest", "MMC-HF-CW"},
        {86, "Maryland-DC QSO Party", "MDC-QSO-PARTY"},
        {241, "Mexico RTTY International Contest", "XE-RTTY"},
        {323, "Michigan QSO Party", "MI-QSO-PARTY"},
        {711, "Mini-Test 40", "MINITEST-40"},
        {712, "Mini-Test 80", "MINITEST-80"},
        {238, "Minnesota QSO Party", "MN-QSO-PARTY"},
        {308, "Mississippi QSO Party", "MS-QSO-PARTY"},
        {327, "Missouri QSO Party", "MO-QSO-PARTY"},
        {741, "NCCC FT4 Sprint", "NCCC-SPRINT-FT4"},
        {540, "NCCC RTTY Sprint", "NCCC-SPRINT-RTTY"},
        {44, "NCCC Sprint", "NCCC-SPRINT-CW"},
        {336, "Nebraska QSO Party", "NE-QSO-PARTY"},
        {12, "Nevada QSO Party", "NV-QSO-PARTY"},
        {10, "New England QSO Party", "NEQP"},
        {71, "New Hampshire QSO Party", "NH-QSO-PARTY"},
        {92, "New Jersey QSO Party", "NJQP"},
        {286, "New Mexico QSO Party", "NM-QSO-PARTY"},
        {473, "New York QSO Party", "NY-QSO-PARTY"},
        {727, "North American Meteor Scatter Sprint", "VHF-NAMSS"},
        {218, "North American QSO Party, CW", "NAQP-CW"},
        {263, "North American QSO Party, RTTY", "NAQP-RTTY"},
        {229, "North American QSO Party, SSB", "NAQP-SSB"},
        {253, "North American Sprint, CW", "NA-SPRINT-CW"},
        {155, "North American Sprint, RTTY", "NA-SPRINT-RTTY"},
        {242, "North American SSB Sprint Contest", "NA-SPRINT-SSB"},
        {265, "North Carolina QSO Party", "NC-QSO-PARTY"},
        {468, "North Dakota QSO Party", "ND-QSO-PARTY"},
        {562, "NRAU 10m Activity Contest", "NRAU-10"},
        {706, "NRRL MGM Weekend Contest", "NRRL-MGM"},
        {721, "NZART Memorial Contest", "MEMORIAL-CONTEST"},
        {151, "Oceania DX Contest, CW", "OCEANIA-DX-CW"},
        {142, "Oceania DX Contest, Phone", "OCEANIA-DX-SSB"},
        {100, "Ohio QSO Party", "MRRC-OHQP"},
        {204, "OK DX RTTY Contest", "OK-DX-RTTY"},
        {185, "OK/OM DX Contest, CW", "OK-OM-DX"},
        {567, "OK/OM DX Contest, SSB", "OK-OM-DX"},
        {293, "Oklahoma QSO Party", "OK-QSO-PARTY"},
        {357, "Ontario QSO Party", "ON-QSO-PARTY"},
        {334, "Open Ukraine RTTY Championship", "UKR-CHAMP-RTTY"},
        {722, "Pajajaran Bogor DX Contest", "PBDX-CONTEST"},
        {153, "Pennsylvania QSO Party", "PA-QSO-PARTY"},
        {679, "Portable Operations Challenge", "POC"},
        {41, "Portugal Day Contest", "PORTUGAL-DAY"},
        {723, "PRG CQ Pride Contest", "TBD"},
        {53, "Quebec QSO Party", "QC-QSO-PARTY"},
        {60, "RAC Canada Day Contest", "CANADA-DAY"},
        {205, "RAC Winter Contest", "CANADA-WINTER"},
        {209, "RAEM Contest", "RAEM"},
        {645, "RCC Cup", "RCC-CUP"},
        {233, "REF Contest, CW", "REF-CW"},
        {260, "REF Contest, SSB", "REF-SSB"},
        {252, "RSGB 1.8 MHz Contest", "RSGB-160"},
        {458, "RSGB 80m Autumn Series, CW", "RSGB-80M-AUT"},
        {617, "RSGB 80m Autumn Series, Data", "RSGB-80M-AUT"},
        {459, "RSGB 80m Autumn Series, SSB", "RSGB-80M-AUT"},
        {275, "RSGB 80m Club Championship, CW", "RSGB-80M-CC"},
        {274, "RSGB 80m Club Championship, Data", "RSGB-80M-CC"},
        {273, "RSGB 80m Club Championship, SSB", "RSGB-80M-CC"},
        {627, "RSGB AFS Contest, CW", "RSGB-AFS-CW"},
        {282, "RSGB Commonwealth (BERU) Contest", "RSGB-COMMONWEALTH"},
        {653, "RSGB FT4 Contest", "RSGB-FT4"},
        {718, "RSGB FT4 International Activity Day", "RSGB-FT4"},
        {662, "RSGB Hope QSO Party", "RSGB-HQP"},
        {74, "RSGB International Low Power Contest", "RSGB-LOW-POWER"},
        {75, "RSGB IOTA Contest", "RSGB-IOTA"},
        {37, "RSGB National Field Day", "RSGB-NFD"},
        {106, "RSGB SSB Field Day", "RSGB-NFD"},
        {669, "RTTYOPS Weekend Sprint", "RTTYOPS-WEEKEND-SPRINT"},
        {670, "RTTYOPS Weeksprint", "RTTYOPS-WEEKSPRINT"},
        {202, "Russian 160-Meter Contest", "RADIO-160"},
        {94, "Russian District Award Contest", "RDAC"},
        {310, "Russian DX Contest", "RDXC"},
        {258, "Russian PSK WW Contest", "RUS-WW-PSK"},
        {103, "Russian RTTY WW Contest", "RADIO-WW-RTTY"},
        {570, "Russian WW Digital Contest", "RUS-WW-DIGI"},
        {574, "Russian WW MultiMode Contest", "RUS-WW-MM"},
        {101, "SARL HF CW Contest", "SARL-HF-CW"},
        {545, "SARL HF Digital Contest", "SARL-HF-DIGI"},
        {78, "SARL HF Phone Contest", "SARL-HF-SSB"},
        {211, "SARTG New Year RTTY Contest", "SARTG-NY-RTTY"},
        {87, "SARTG WW RTTY Contest", "SARTG-RTTY"},
        {121, "Scandinavian Activity Contest, CW", "SAC-CW"},
        {131, "Scandinavian Activity Contest, SSB", "SAC-SSB"},
        {680, "SCRY/RTTYOps WW RTTY Contest", "RTTYOPS-WW-RTTY"},
        {749, "SEC QSO Party", "SEC-QSO-PARTY"},
        {616, "Solar Eclipse QSO Party", "ECLIPSE-QSO"},
        {598, "South America 10 Meter Contest", "SA10M"},
        {728, "South American Integration Contest CW", "SACW"},
        {123, "South Carolina QSO Party", "SC-QSO-PARTY"},
        {492, "South Dakota QSO Party", "SDQSOP"},
        {312, "SP DX Contest", "SPDX"},
        {207, "Stew Perry Topband Challenge", "STEW-PERRY"},
        {115, "Tennessee QSO Party", "TN-QSO-PARTY"},
        {725, "Tennessee State Parks on the Air", "TNPOTA"},
        {566, "TESLA Memorial HF CW Contest", "TESLA-HF"},
        {133, "Texas QSO Party", "TXQP"},
        {559, "Texas State Parks on the Air", "TSPOTA"},
        {678, "Tisza Cup CW Contest", "TISZACUP"},
        {637, "TRC Digi Contest", "TRC-DIGI"},
        {536, "TRC DX Contest", "TRC-DX"},
        {697, "UA1DZ Memorial Cup", "ALRS-UA1DZ-CUP"},
        {261, "UBA DX Contest, CW", "UBA-DX-CW"},
        {235, "UBA DX Contest, SSB", "UBA-DX-SSB"},
        {165, "UBA ON Contest, 2m", "UBA-ON-2M"},
        {139, "UBA ON Contest, 6m", "UBA-ON-6M"},
        {157, "UBA ON Contest, CW", "UBA-ON-CW"},
        {144, "UBA ON Contest, SSB", "UBA-ON-SSB"},
        {576, "UBA PSK63 Prefix Contest", "UBA-PSK63-PREFIX"},
        {297, "UBA Spring Contest, 2m", "UBA-SPRING-CONTEST"},
        {298, "UBA Spring Contest, 6m", "UBA-SPRING-CONTEST"},
        {295, "UBA Spring Contest, CW", "UBA-SPRING-CONTEST"},
        {296, "UBA Spring Contest, SSB", "UBA-SPRING-CONTEST"},
        {603, "UFT QRP Contest", "UFT-QRP"},
        {597, "UK/EI DX Contest, CW", "UKEI-DX"},
        {596, "UK/EI DX Contest, SSB", "UKEI-DX"},
        {583, "UKEICC 80m Contest", "UKEICC-80M"},
        {542, "Ukrainian DX Classic RTTY Contest", "UR-DX-RTTY"},
        {176, "Ukrainian DX Contest", "UKRAINIAN-DX"},
        {55, "Ukrainian DX DIGI Contest", "UR-DX-DIGI"},
        {480, "UN DX Contest", "UN-DX"},
        {236, "Vermont QSO Party", "VT-QSO-PARTY"},
        {302, "Virginia QSO Party", "VA-QSO-PARTY"},
        {481, "VK Shires Contest", "VKSHIRES"},
        {13, "VOLTA WW RTTY Contest", "VOLTA-RTTY"},
        {169, "W/VE Islands QSO Party", "ISLAND-QSO-PARTY"},
        {85, "WAE DX Contest, CW", "DARC-WAEDC-CW"},
        {183, "WAE DX Contest, RTTY", "DARC-WAEDC-RTTY"},
        {111, "WAE DX Contest, SSB", "DARC-WAEDC-SSB"},
        {126, "Washington State Salmon Run", "WA-SALMON-RUN"},
        {753, "Weekly RTTY Test", "WRT"},
        {49, "West Virginia QSO Party", "WVQP"},
        {421, "Winter Field Day", "WFD"},
        {330, "Wisconsin QSO Party", "WIQP"},
        {163, "Worked All Germany Contest", "WAG"},
        {546, "Worked All Provinces of China DX Contest", "WAPC-DX"},
        {650, "World Wide Digi DX Contest", "WW-DIGI"},
        {755, "World Wide Patagonia DX Contest", "WWPDX"},
        {672, "Worldwide Sideband Activity Contest", "WWSAC"},
        {471, "WW PMC Contest", "WW-PMC"},
        {630, "YARC QSO Party", "YARC-QSO-PARTY"},
        {747, "YB Banggai DX Contest", "BANGGAI-DX"},
        {746, "YB Bekasi Merdeka Contest", "BEKASI-MERDEKA-CONTEST"},
        {621, "YB DX Contest", "YB DX CONTEST"},
        {642, "YB DX RTTY Contest", "YB-DX-CONTEST"},
        {353, "YLRL YL-OM Contest", "YL-OM"},
        {98, "YO DX HF Contest", "YO-DX-HF"},
        {696, "YOTA Contest", "YOTA"},
        {322, "YU DX Contest", "YUDX"},
        {367, "Yuri Gagarin International DX Contest", "GC"},
        {737, "ZL Sprint", "ZL-SPRINT"}
    };

signals:
};

#endif // CONTESTCONFIGURATION_H
