#include <QWidget>
// **********************  StationDataTab  *************************** //

// This is the QWidget holding station data
// template <typename T>
class StationDataTab : public QWidget
{
    Q_OBJECT

public:
    explicit StationDataTab(Sqlite3_connector *p, QWidget *parent = nullptr);
    ~StationDataTab();

private:
    Sqlite3_connector *db;
    QFormLayout *formLayout;
    QMap<QString, QString> data_list_local_map;

public:
    QList<QLineEdit *> dataEditBoxes;    // These are the QLineEdit boxes in the tab

signals:
    void selectionChanged();

};

// **********************  SystemConfigTab  *************************** //

// This is the QWidget holding system configuration data

class SystemConfigTab : public QWidget
{
    Q_OBJECT

public:
    explicit SystemConfigTab(Sqlite3_connector *p, QWidget *parent = nullptr);
    ~SystemConfigTab();

private:
    Sqlite3_connector *db;
    QMap<QString, QString> data_list_local_map;

public:
    QList<QLineEdit *> dataEditBoxes;
    QFormLayout *formLayout;

};

// **********************  ContestTab  *************************** //

// This is the QWidget holding contest configuration data
class ContestTab : public QWidget
{
    Q_OBJECT

public:
    explicit ContestTab(Sqlite3_connector *p, QWidget *parent = nullptr);
    ~ContestTab();

private:
    Sqlite3_connector *db;
    QFormLayout *formLayout;
    QMap<QString, QString> data_list_local_map;

public:
    QList<QLineEdit *> dataEditBoxes;
};


