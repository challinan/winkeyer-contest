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
#include <QComboBox>

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

private:
    void save_tabbed_data_to_database();

    template <typename T>
    void saveTabbedDataToLocalMap(T *tabPtr) { // Pointer to our config Dialog Tabs

        // Store away the values from the associated tab's edit boxes
        // The QLineEdit objects are enumerated in the editBoxes list in the individual classes
        qDebug() << "saveTabbedDataToLocalMap:" << tabPtr;
        QListIterator<QLineEdit *> s = tabPtr->dataEditBoxes;
        while (s.hasNext() ) {
            QLineEdit *lep_tmp = s.next();
            QString objname = lep_tmp->objectName();
            qDebug() << "saveTabbedDataToLocalMap(): objname:" << objname;

            lep_tmp = tabPtr->template findChild<QLineEdit *>(objname);
            objname.remove("EditBox");

            QString value = lep_tmp->text();
            tabPtr->setLocalMapValueByKey(objname, value);
            qDebug() << "TopLevelTabContainerDialog::saveTabbedDataToLocalMap(): objname, lep_tmp->text()"
                     << objname << lep_tmp->text();
        }

        QListIterator<QComboBox *> c = tabPtr->comboBoxesList;
        while ( c.hasNext() ) {
            QComboBox *cb_tmp = c.next();
            QString objname = cb_tmp->objectName();
            qDebug() << "saveTabbedDataToLocalMap(): objname:" << objname;

            cb_tmp = tabPtr->template findChild<QComboBox *>(objname);
            objname.remove("ComboBox");

            QString value = cb_tmp->currentText();
            tabPtr->setLocalMapValueByKey(objname, value);
            qDebug() << "TopLevelTabContainerDialog::saveTabbedDataToLocalMap(): objname, cb_tmp->text()"
                     << objname << cb_tmp->currentText();
        }
};

private:
    // TODO: Consolidate these into a single generic routine

    // Read local database map from DataClass objects into tabbed dialog
    template<typename T>
    bool get_local_data_into_dialog_T(T *pDataClassPtr);

private:
    // QTabWidget holds the stack of tabbed wigets
    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;
    Sqlite3_connector *db;
    QVBoxLayout *pMainLayout;

    // Pointers to individual tab objects
    StationDataTab *pStationDataTab;
    SystemConfigTab *pSysconfigTab;
    ContestTab *pContestTab;

    // Pointers to db Classes
    StationData *pStationDataClassPtr;
    SysconfigData *pSysconfigDataClassPtr;
    ContestData *pContestDataClassPtr;

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

    virtual void setLocalMapValueByKey(QString key, QString value) = 0;

private:
    Sqlite3_connector *db;
    QFormLayout *formLayout;

public:
    // These are the QLineEdit boxes in the tab
    QList<QLineEdit *> dataEditBoxes;
    QList<QComboBox *> comboBoxesList;

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

    void setLocalMapValueByKey(QString key, QString value);

};

// **********************  SystemConfigTab  *************************** //

// This is the QWidget holding system configuration data
class SystemConfigTab : public DataTab
{
    Q_OBJECT

public:
    explicit SystemConfigTab(Sqlite3_connector *p, QWidget *parent = nullptr);
    ~SystemConfigTab();

    void setLocalMapValueByKey(QString key, QString value);

};

// **********************  ContestTab  *************************** //

// This is the QWidget holding contest configuration data
class ContestTab : public DataTab
{
    Q_OBJECT

public:
    explicit ContestTab(Sqlite3_connector *p, QWidget *parent = nullptr);
    ~ContestTab();

    void setLocalMapValueByKey(QString key, QString value);

};

#endif // TABBEDDIALOG_H
