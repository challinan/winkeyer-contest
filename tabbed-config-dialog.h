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
class ContestTab;
class SystemConfigTab;

// The top level container holding the tab Widgets
class TopLevelTabContainerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TopLevelTabContainerDialog(Sqlite3_connector *db, QWidget *parent = nullptr);
    ~TopLevelTabContainerDialog();

    void connectTextChangedSignals(bool doConnect=true);
    void setFieldText(QString s, QString t);

private:
    void save_tabbed_data_to_database();
    bool get_local_station_data_into_dialog();

private:
    // QTabWidget holds the stack of tabbed wigets
    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;
    Sqlite3_connector *db;
    StationDataTab *pStationDataTab;
    ContestTab *pContestTab;
    SystemConfigTab *pSystemConfigTab;

private slots:
    void user_pressed_save();
    void station_data_changed();
    void user_pressed_cancel();
};

// This is the QWidget holding stataion data
class StationDataTab : public QWidget
{
    Q_OBJECT

public:
    explicit StationDataTab(Sqlite3_connector *p, QWidget *parent = nullptr);
    ~StationDataTab();

private:
    Sqlite3_connector *db;

public:
    QList<QLineEdit *> editBoxes;    // These are the QLineEdit boxes in the tab

signals:
    void selectionChanged();

};

// This is the QWidget holding contest configuration data
class ContestTab : public QWidget
{
    Q_OBJECT

public:
    explicit ContestTab(Sqlite3_connector *p, QWidget *parent = nullptr);

private:
    Sqlite3_connector *db;
};

// This is the QWidget holding system configuration data
class SystemConfigTab : public QWidget
{
    Q_OBJECT

public:
    explicit SystemConfigTab(Sqlite3_connector *p, QWidget *parent = nullptr);

private:
    Sqlite3_connector *db;
};

#endif // TABBEDDIALOG_H
