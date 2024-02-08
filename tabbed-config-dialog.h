#ifndef TABBEDDIALOG_H
#define TABBEDDIALOG_H

#include <QWidget>
#include <QTabWidget>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QListWidget>
#include <QFormLayout>

#include "sqlite3-connector.h"

// Forward declarations
class StationDataTab;
class SystemConfigTab;
class ContestTab;

// The top level container holding the tab Widgets
class TopLevelTabContainerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TopLevelTabContainerDialog(Sqlite3_connector *db, QWidget *parent = nullptr);
    ~TopLevelTabContainerDialog();

    void connectStationTabTextChangedSignals(bool doConnect=true);
    template <typename T>void setFieldText(T *tabPtr, QString s, QString t);

     template <typename T>
    T *getStationDataTabPtr() { return pStationDataTab; }


private:
    void save_tabbed_data_to_database();

    template <typename T>
    void save_tabbed_data_to_database_template(T *tabPtr);

    bool get_local_station_data_into_dialog();
    bool get_local_sysconfig_data_into_dialog();

private:
    // QTabWidget holds the stack of tabbed wigets
    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;
    Sqlite3_connector *db;
    StationDataTab *pStationDataTab;
    ContestTab *pContestTab;
    SystemConfigTab *pSysconfigTab;
    QVBoxLayout *pMainLayout;

private slots:
    void user_pressed_save();
    void station_data_changed();
    void user_pressed_cancel();
};

// **********************  StationDataTab  *************************** //

// This is the QWidget holding station data
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
    QFormLayout *formLayout;
    QMap<QString, QString> data_list_local_map;

public:
    QList<QLineEdit *> dataEditBoxes;

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

#endif // TABBEDDIALOG_H
