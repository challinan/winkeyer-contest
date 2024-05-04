#ifndef COUNTYLIST_H
#define COUNTYLIST_H

#include <QObject>
#include <QDebug>
#include <QFile>
#include <QRect>

struct state_t {
    QString state;
    QList<QString> counties;
};

class CountyList : public QObject
{
    Q_OBJECT
public:
    explicit CountyList(QObject *parent = nullptr);
    ~CountyList();

    void process_line(QByteArray &line, QFile &file);
    bool isNumber(char c);
    int findCountyCount(QByteArray line);
    QString findState(QByteArray line);
    QString findCountyName(QByteArray &line);
    void displayTable();

private:
    QList<struct state_t> us_state_county_table;

signals:
};

#endif // COUNTYLIST_H
