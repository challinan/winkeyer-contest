#include <QString>
#include <QPushButton>
#include <QSerialPortInfo>

#include "tabbed-config-dialog.h"

TopLevelTabContainerDialog::TopLevelTabContainerDialog(Sqlite3_connector *p, QWidget *parent)
    : QDialog(parent)
{
    db = p;
    qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Entered";
    tabWidget = new QTabWidget;

    pStationDataTab = new StationDataTab(db);
    pSysconfigTab = new SystemConfigTab(db);
    pContestTab = new ContestTab(db);
    pContestConfigTab = new ContestConfigTab(db);

    // Get pointers to Database Sub-classes - yup, this is super ugly
    pStationDataClassPtr = db->getStationDbClassPtr();
    pSysconfigDataClassPtr = db->getSysconfigDbClassPtr();
    pContestDataClassPtr = db->getContestDbClassPtr();
    pContestConfigDataClassPtr = db->getContestConfigDbClassPtr();

    tabWidget->addTab(pStationDataTab, "StationData");
    tabWidget->addTab(pSysconfigTab, "System Config");
    tabWidget->addTab(pContestTab, "Contest Data");
    tabWidget->addTab(pContestConfigTab, "Contest Config");
    qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Tabs Added";

    // Create two standard push buttons, and connect each of them to the appropriate slots in the dialog
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    // Set the default button's label to "Save"
    QPushButton *pOK = buttonBox->button(QDialogButtonBox::Ok);
    pOK->setText("Save");

    connect(buttonBox, &QDialogButtonBox::accepted, this, &TopLevelTabContainerDialog::user_pressed_save);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TopLevelTabContainerDialog::user_pressed_cancel);

    // Arrange the tab widget above the buttons in the dialog
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    pMainLayout->addWidget(tabWidget);
    pMainLayout->addWidget(buttonBox);
    setLayout(pMainLayout);

    // Set the dialog's title
    setWindowTitle("Configuration Dialog");

    // {"station_data", "sysconfig_data", "contest_data", "contest_config_data"}
    // Get a simple list of QStrings containing our table names
    // TODO: Check if this is valid.  Doesn't "Register" save a table list?
    QList<QString> items = db->GetTableNameList();

    // If data is available, populate it here
    for (const QString &item : items ) {
        database_state db_state = db->getDatabaseState(item);
        if ( db_state != DB_NOEXIST && db_state != DB_NOTABLES ) {
            qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Reading database data into tabs **********************************";
            // TODO: This is ugly.  Fix it.
            if ( item == "station_data" )
                get_local_data_into_dialog_T(pStationDataClassPtr);
            if ( item == "sysconfig_data" )
                get_local_data_into_dialog_T(pSysconfigDataClassPtr);
            if ( item == "contest_data" )
                get_local_data_into_dialog_T(pContestDataClassPtr);
            if ( item == "contest_config_data" )
                get_local_data_into_dialog_T(pContestConfigDataClassPtr);
        } else {
            qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Skipping database read into tabs";
            // TODO
            // Debug code - prepopulate station data
            qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): LOADING DUMMY DATA INTO TABS";
            if ( item == "station_data" ) {
                pStationDataTab->set_dummy_station_data(pStationDataClassPtr);
                get_local_data_into_dialog_T(pStationDataClassPtr);
            }
        }

    }
}

template<typename T>
bool TopLevelTabContainerDialog::get_local_data_into_dialog_T(T *pDataClassPtr) {

    // Get pointer to StationData db fields
    const QMap<int, dbfields_values_t> &db_fields = pDataClassPtr->getDbFields();
    QMap<QString, QString> &pMap = pDataClassPtr->getLocalDataMap();

    QMapIterator<int, dbfields_values_t> e(db_fields);
    if ( e.hasNext() ) {
        e.next();   // Skip over the table name
    } else {
        qDebug() << "TopLevelTabContainerDialog::get_local_data_into_dialog_T(): Failed - expected more rows in db_fields";
        return false;
    }

    while ( e.hasNext() ) {
        e.next();
        //  For example: {"callsign", "opname", "gridsquare", "city", "state", "county", "country", "arrlsection"}

        QLineEdit *p;
        QString childName = e.value().fieldname;    // Might be "callsign", or "opname", etc
        if ( childName == "serialport" ) {
            // Process the comboBox
            childName.append("ComboBox");
            qDebug() << "TopLevelTabContainerDialog::get_local_data_into_dialog_T(): childName =" << childName;  // will be "callsignEditBox", etc.
            QComboBox *pBox = findChild<QComboBox *>(childName);
            if ( pBox == nullptr ) {
                qDebug() << "TopLevelTabContainerDialog::get_local_data_into_dialog_T(): child" << childName << "ComboBox not found";
                return false;
            }
            QString tmps = pMap.value(e.value().fieldname);
            pBox->addItem(tmps);
        } else {
            childName.append("EditBox");
            qDebug() << "TopLevelTabContainerDialog::get_local_data_into_dialog_T(): childName =" << childName;  // will be "callsignEditBox", etc.

            p = findChild<QLineEdit *>(childName);
            if ( p == nullptr ) {
                qDebug() << "TopLevelTabContainerDialog::get_local_data_into_dialog_T(): child" << childName << "Edit Box not found";
                return false;
            }

            QString e_dbfields_value = e.value().fieldname;
            QString str_tmp = pMap.value(e_dbfields_value);
            p->setText(str_tmp);
        }
    }
    return true;
}

// Avoid link errors by predefining all instances of this template function
// template void TopLevelTabContainerDialog::saveTabbedDataToLocalMap(StationDataTab *);
// template void TopLevelTabContainerDialog::saveTabbedDataToLocalMap(SystemConfigTab *);
// template void TopLevelTabContainerDialog::saveTabbedDataToLocalMap(ContestTab*);

template <typename T>
void TopLevelTabContainerDialog::saveTabbedDataToLocalMap(T *tabPtr) { // Pointer to our config Dialog Tabs

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

void TopLevelTabContainerDialog::user_pressed_save() {

    qDebug() << "TopLevelTabContainerDialog::user_pressed_save(): Entered";
    pStationDataClassPtr = db->getStationDbClassPtr();
    pSysconfigDataClassPtr = db->getSysconfigDbClassPtr();
    pContestDataClassPtr = db->getContestDbClassPtr();
    pContestConfigDataClassPtr = db->getContestConfigDbClassPtr();

    // TODO There must be a better way to do this
    saveTabbedDataToLocalMap<StationDataTab>(pStationDataTab);
    db->dump_local_station_data();
    saveTabbedDataToLocalMap<SystemConfigTab>(pSysconfigTab);
    db->dump_local_sysconfig_data();
    saveTabbedDataToLocalMap<ContestTab>(pContestTab);
    db->dump_local_contest_data();
    saveTabbedDataToLocalMap<ContestConfigTab>(pContestConfigTab);
    db->dump_local_contest_config_data();


    db->syncGenericWriteToDatabase_T(pStationDataClassPtr);
    db->syncGenericWriteToDatabase_T(pSysconfigDataClassPtr);
    db->syncGenericWriteToDatabase_T(pContestDataClassPtr);
    db->syncGenericWriteToDatabase_T(pContestConfigDataClassPtr);

    QDialog::accept();
}

void TopLevelTabContainerDialog::user_pressed_cancel() {
    qDebug() << "TopLevelTabContainerDialog::user_pressed_cancel(): Entered";
    QDialog::reject();
}

void TopLevelTabContainerDialog::station_data_changed() {
    qDebug() << "TopLevelTabContainerDialog::station_data_changed(): **********>>> Entered";
}

TopLevelTabContainerDialog::~TopLevelTabContainerDialog() {

    qDebug() << "TopLevelTabContainerDialog::~TopLevelTabContainerDialog(): Destructor entered";
    delete pMainLayout;
    delete buttonBox;
    delete pSysconfigTab;
    delete pContestTab;
    delete pStationDataTab;
    delete pContestConfigTab;
    delete tabWidget;
}

// ******************  Data Tab Base Class ************************ //
DataTab::DataTab(Sqlite3_connector *p, QWidget *parent)
    : QWidget(parent)
{
    db = p; // Unused
}

DataTab::~DataTab() {

    // Delete all the QLineEdit objects
    QListIterator<QLineEdit *> e(dataEditBoxes);
    while (e.hasNext() ) {
        QLineEdit *p = e.next();
        // qDebug () << "Deleting: ******* " << p->objectName();
        delete p;
        // delete e.next();
    }

    // Delete all the comboBoxes
    QListIterator<QComboBox *> m(comboBoxesList);
    while (m.hasNext() ) {
        QComboBox *p = m.next();
        // qDebug () << "Deleting: ******* " << p->objectName();
        delete p;
        // delete e.next();
    }


    delete formLayout;
}


// ******************  StationDataTab  **************************** //
StationDataTab::StationDataTab(Sqlite3_connector *p, QWidget *parent)
    : DataTab(p, parent)
{
    db = p;     // Pointer to sqlite3_connector database engine

    StationData *pStationDataClassPtr = db->getStationDbClassPtr();

    // Get pointer to StationData db fields
    QMap<int, dbfields_values_t> db_fields = pStationDataClassPtr->getDbFields();

    formLayout = new QFormLayout(this);

    // {"callsign", "opname", "gridsquare", "city"}, "state", "county", "country", "arrlsection", "serialport"}
    QMapIterator<int, dbfields_values_t> e(db_fields);

    // First entry is the tablename
    if (e.hasNext() ) {
        e.next();   // Skip over table name
    } else {
        qDebug() << "StationDataTab::StationDataTab(): Failed - expected more rows in db_fields";
        return;
    }

    while ( e.hasNext() ) {
        e.next();
        QString field_name = e.value().fieldname;

        // Create an appropriate QLineEdit, put it in the List, give it an objname, and add it to layout
        QLineEdit *qle = new QLineEdit;

        // Give the LineEdit boxes unique names so we can easily look them up later
        QString objname = field_name;
        objname.append("EditBox");
        qle->setObjectName(objname);
        dataEditBoxes.append(qle); // Add this one to the QList<QLineEdit> list
        qDebug() << "StationDataTab::StationDataTab(): Adding editBox to station data tab" << qle << qle->objectName();


        QString editBoxLabel = e.value().field_label;
        formLayout->addRow(editBoxLabel, qle);
        // qDebug() << "StationDataTab::StationDataTab(): EditBox Object Name:" << objname << "key =" << key << "editBoxLabel = " << editBoxLabel;
    }
    setLayout(formLayout);
}

void StationDataTab::setLocalMapValueByKey(QString key, QString value) {
    // This works
    QMap<QString, QString> &p = db->getStationDbClassPtr()->getLocalDataMap();
    p.insert(key, value);
}

StationDataTab::~StationDataTab() {
    // Delete code found in base class
}

void StationDataTab::set_dummy_station_data(StationData *pStationData) {

    QMap<QString, QString> &pMap = pStationData->getLocalDataMap();

    pMap.insert("callsign", "K1AY");
    pMap.insert("opname", "Chris");
    pMap.insert("gridsquare", "EL96av");
    pMap.insert("city", "Punta Gorda");
    pMap.insert("state", "FL");
    pMap.insert("county", "Charlotte");
    pMap.insert("country", "US");
    pMap.insert("cqzone", "5");
    pMap.insert("arrlsection", "WCF");
    pMap.insert("ituzone", "2");
}

// ******************  SystemConfigTab  **************************** //
SystemConfigTab::SystemConfigTab(Sqlite3_connector *p, QWidget *parent)
    : DataTab(p, parent)
{
    db = p;     // Pointer to sqlite3_connector database engine

    SysconfigData *pSysconfigDataClassPtr = db->getSysconfigDbClassPtr();

    // Get pointer to StationData db fields
    QMap<int, dbfields_values_t> db_fields = pSysconfigDataClassPtr->getDbFields();

    formLayout = new QFormLayout(this);

    // {"serialport", "audioinport", audiooutport"}
    QMapIterator<int, dbfields_values_t> e(db_fields);

    // First entry is the tablename
    if (e.hasNext() ) {
        // Skip over table name - we don't need it here
        e.next();
    } else {
        qDebug() << "SystemConfigTab::SystemConfigTab(): Failed - expected more rows in db_fields";
        return;
    }

    while ( e.hasNext() ) {
        e.next();
        QString key = e.value().fieldname;

        // Serial port needs a ComboBox
        if ( key == "serialport" ) {
            // Do we have a configured value? Check our local map
            QMap<QString, QString> pMap = db->getSysconfigDbClassPtr()->getLocalDataMap();
            QString configuredSerialPort = pMap.value("serialport");

            QList<QSerialPortInfo> serialPortsList = QSerialPortInfo::availablePorts();

            QComboBox *box = new QComboBox;
            box->setInsertPolicy(QComboBox::InsertAtTop);

            QListIterator m(serialPortsList);
            while (m.hasNext()) {
                QString s = m.next().portName();
                if ( s.startsWith("cu.usbserial") ) {
                    box->addItem(s);
                }
            }

            if ( !configuredSerialPort.isEmpty() )
                box->addItem(configuredSerialPort);

            if ( box->count() == 0 ) {
                box->addItem("No Port Found");
            }

            comboBoxesList.append(box);     // So we can delete it later
            QString objname = key;
            objname.append("ComboBox");
            box->setObjectName(objname);

            QString comboBoxLabel = e.value().field_label;
            box->setMinimumWidth(270);
            formLayout->addRow(comboBoxLabel, box);

        } else {

        // Create an appropriate QLineEdit, put it in the List, give it an objname, and add it to layout
        QLineEdit *qle = new QLineEdit;
        dataEditBoxes.append(qle); // Add this one to the QList<QLineEdit> list

        QString objname = key;
        objname.append("EditBox");
        qle->setObjectName(objname);

        QString editBoxLabel = e.value().field_label;
        formLayout->addRow(editBoxLabel, qle);
        }
        // qDebug() << "SystemConfigTab::SystemConfigTab(): EditBox Object Name:" << objname << "key =" << key << "editBoxLabel = " << editBoxLabel;
    }
    setLayout(formLayout);
}

void SystemConfigTab::setLocalMapValueByKey(QString key, QString value) {
    QMap<QString, QString> &p = db->getSysconfigDbClassPtr()->getLocalDataMap();
    p.insert(key, value);
}

SystemConfigTab::~SystemConfigTab() {
    // Delete code found in base class
}

// ******************  ContestTab  **************************** //
ContestTab::ContestTab(Sqlite3_connector *p, QWidget *parent)
    : DataTab(p, parent)
{
    db = p;     // Pointer to sqlite3_connector database engine

    ContestData *pContestDataClassPtr = db->getContestDbClassPtr();

    // Get pointer to Contest Data db fields
    QMap<int, dbfields_values_t> db_fields = pContestDataClassPtr->getDbFields();

    // TOOO: Get these from the new table

    formLayout = new QFormLayout(this);

    // {"sequence", "rst"}

    QMapIterator<int, dbfields_values_t> e(db_fields);
    // First entry is the tablename
    if (e.hasNext() ) {
        // Skip over table name - we don't need it here
        e.next();
    } else {
        qDebug() << "ContestTab::ContestTab(): Failed - expected more rows in db_fields";
        return;
    }


    while (e.hasNext() ) {
        QString key = e.next().value().fieldname;

        // Create an appropriate QLineEdit, put it in the List, give it an objname, and add it to layout
        QLineEdit *qle = new QLineEdit;
        dataEditBoxes.append(qle); // Add this one to the QList<QLineEdit> list
        QString objname = key;
        objname.append("EditBox");
        qle->setObjectName(objname);

        QString editBoxLabel = e.value().field_label;
        formLayout->addRow(editBoxLabel, qle);
        qDebug() << "ContestTab::ContestTab(): EditBox Object Name:" << objname << "key =" << key << "editBoxLabel = " << editBoxLabel;
    }
    setLayout(formLayout);
}

void ContestTab::setLocalMapValueByKey(QString key, QString value) {
    QMap<QString, QString> &p = db->getContestDbClassPtr()->getLocalDataMap();
    p.insert(key, value);
}

ContestTab::~ContestTab() {
    // Delete code found in base class
}

// ******************  ContestConfigTab  **************************** //
ContestConfigTab::ContestConfigTab(Sqlite3_connector *p, QWidget *parent)
    : DataTab(p, parent)
{
    db = p;     // Pointer to sqlite3_connector database engine

    ContestConfigData *pContestConfigDataClassPtr = db->getContestConfigDbClassPtr();

    // Get pointer to Contest Data db fields
    QMap<int, dbfields_values_t> db_fields = pContestConfigDataClassPtr->getDbFields();

    // TOOO: Get these from the new table

    formLayout = new QFormLayout(this);

    // {"sequence", "rst"}

    QMapIterator<int, dbfields_values_t> e(db_fields);
    // First entry is the tablename
    if (e.hasNext() ) {
        // Skip over table name - we don't need it here
        e.next();
    } else {
        qDebug() << "ContestConfigTab::ContestConfigTab(): Failed - expected more rows in db_fields";
        return;
    }


    while (e.hasNext() ) {
        QString key = e.next().value().fieldname;

        // Create an appropriate QLineEdit, put it in the List, give it an objname, and add it to layout
        QLineEdit *qle = new QLineEdit;
        dataEditBoxes.append(qle); // Add this one to the QList<QLineEdit> list
        QString objname = key;
        objname.append("EditBox");
        qle->setObjectName(objname);

        QString editBoxLabel = e.value().field_label;
        formLayout->addRow(editBoxLabel, qle);
        qDebug() << "ContestConfigTab::ContestConfigTab(): EditBox Object Name:" << objname << "key =" << key << "editBoxLabel = " << editBoxLabel;
    }
    setLayout(formLayout);
}

void ContestConfigTab::setLocalMapValueByKey(QString key, QString value) {
    QMap<QString, QString> &p = db->getContestConfigDbClassPtr()->getLocalDataMap();
    p.insert(key, value);
}

ContestConfigTab::~ContestConfigTab() {
    // Delete code found in base class
}

