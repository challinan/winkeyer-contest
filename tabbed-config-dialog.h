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

// The top level container holding the tab Widgets
class TopLevelTabContainerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TopLevelTabContainerDialog(Sqlite3_connector *db, QWidget *parent = nullptr);
    ~TopLevelTabContainerDialog();

private:
    // QTabWidget holds the stack of tabbed wigets
    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;
    Sqlite3_connector *db;
};

// This is the QWidget holding stataion data
class StationDataTab : public QWidget
{
    Q_OBJECT

public:
    explicit StationDataTab(Sqlite3_connector *p, QWidget *parent = nullptr);

private:
    Sqlite3_connector *db;

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
