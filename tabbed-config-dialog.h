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
    void save_tabbed_data_to_database_template(T *tabPtr) {

        // Store away the values from the associated tab
        // The QLineEdit objects are enumerated in the editBoxes list in the individual classes
        qDebug() << "save_tabbed_data_to_database_template:" << tabPtr;
        QListIterator<QLineEdit *> s = tabPtr->dataEditBoxes;
        while (s.hasNext() ) {
            QLineEdit *lep_tmp = s.next();
            QString objname = lep_tmp->objectName();
            qDebug() << "save_tabbed_data_to_database_template(): objname:" << objname;

            lep_tmp = tabPtr->template findChild<QLineEdit *>(objname);
            objname.remove("EditBox");

            QString value = lep_tmp->text();
            tabPtr->set_xxx_table_value_by_key(objname, value);
            qDebug() << "TopLevelTabContainerDialog::save_tabbed_data_to_database_template(): objname, leptmp->text()"
                     << objname << lep_tmp->text();
        }

        // Save the data from the Tabbed Dialog into the local db
//        db->syncGeneric_write(*pSysconfigKeyFields);
//        db->syncGeneric_write(*pContestKeyFields);
};

private:
    // TODO: Consolidate these into a single generic routine
    bool get_local_station_data_into_dialog();
    bool get_local_sysconfig_data_into_dialog();
    bool get_local_contest_data_into_dialog();

private:
    // QTabWidget holds the stack of tabbed wigets
    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;
    Sqlite3_connector *db;
    StationDataTab *pStationDataTab;
    SystemConfigTab *pSysconfigTab;
    ContestTab *pContestTab;
    QVBoxLayout *pMainLayout;

    // Pointers to db key maps
    const QMap<int, QString> *pStationKeyFields;
    const QMap<int, QString> *pSysconfigKeyFields;
    const QMap<int, QString> *pContestKeyFields;

private slots:
    void user_pressed_save();
    void station_data_changed();
    void user_pressed_cancel();
};

// **********************  Tabbed Dialog Base Class ****************** //
class DataTab : public QWidget
{
    Q_OBJECT

public:
    explicit DataTab(Sqlite3_connector *p, QWidget *parent = nullptr);
    ~DataTab();

    // db->set_sysconfig_table_value_by_key(objname, lep_tmp->text())
    virtual void set_xxx_table_value_by_key(QString objname, QString value) = 0;

private:
    Sqlite3_connector *db;
    QFormLayout *formLayout;

public:
    QList<QLineEdit *> dataEditBoxes;    // These are the QLineEdit boxes in the tab
    QMap<QString, QString> data_list_local_map;

signals:
    void selectionChanged();

    friend StationDataTab;
    friend SystemConfigTab;
    friend ContestTab;
};

// **********************  StationDataTab  *************************** //

// This is the QWidget holding station data
class StationDataTab : public DataTab
{
    Q_OBJECT

public:
    explicit StationDataTab(Sqlite3_connector *p, QWidget *parent = nullptr);
    ~StationDataTab();

    void set_xxx_table_value_by_key(QString objname, QString value);
};

// **********************  SystemConfigTab  *************************** //

// This is the QWidget holding system configuration data
class SystemConfigTab : public DataTab
{
    Q_OBJECT

public:
    explicit SystemConfigTab(Sqlite3_connector *p, QWidget *parent = nullptr);
    ~SystemConfigTab();

    void set_xxx_table_value_by_key(QString key, QString value);
};

// **********************  ContestTab  *************************** //

// This is the QWidget holding contest configuration data
class ContestTab : public DataTab
{
    Q_OBJECT

public:
    explicit ContestTab(Sqlite3_connector *p, QWidget *parent = nullptr);
    ~ContestTab();

    void set_xxx_table_value_by_key(QString key, QString value);
};

#endif // TABBEDDIALOG_H
