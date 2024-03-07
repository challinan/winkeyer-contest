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

    // Get pointers to Database Sub-classes - yup, this is super ugly
    pStationDataClassPtr = db->getStationDbClassPtr();
    pSysconfigDataClassPtr = db->getSysconfigDbClassPtr();
    pContestDataClassPtr = db->getContestDbClassPtr();

    tabWidget->addTab(pStationDataTab, "StationData");
    tabWidget->addTab(pSysconfigTab, "System Config");
    tabWidget->addTab(pContestTab, "Contest Config");
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

    // {"station_data", "sysconfig_data", "contest_data"}
    // Get a simple list of QStrings containing our table names
    // TODO: Check if this is valid.  Doesn't "Register" save a table list?
    QList<QString> items = db->GetTableNameList();

    // If data is available, populate it here
    for (const QString &item : items ) {
        database_state db_state = db->getDatabaseState(item);
        if ( db_state != DB_NOEXIST &&  db_state != DB_NOTABLES ) {
            qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Reading database data into tabs";
            // TODO: This is ugly.  Fix it.
            if ( item == "station_data" )
                get_local_data_into_dialog_T(pStationDataClassPtr);
            if ( item == "sysconfig_data" )
                get_local_data_into_dialog_T(pSysconfigDataClassPtr);
            if ( item == "contest_data" )
                get_local_data_into_dialog_T(pContestDataClassPtr);
        } else {
            qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Skipping database read into tabs";
        }

    }

    // Connect potential signals here
    connectStationTabTextChangedSignals();

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
        //  For example: {"callsign", "opname", "gridsquare", "city", "state", "county", "country", "section"}

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
            pBox->addItem(pMap.value(e.value().fieldname));
        } else {
            childName.append("EditBox");
            qDebug() << "TopLevelTabContainerDialog::get_local_data_into_dialog_T(): childName =" << childName;  // will be "callsignEditBox", etc.

            p = findChild<QLineEdit *>(childName);
            if ( p == nullptr ) {
                qDebug() << "TopLevelTabContainerDialog::get_local_data_into_dialog_T(): child" << childName << "Edit Box not found";
                return false;
            }

            p->setText(pMap.value(e.value().fieldname));
        }
    }
    return true;
}

void TopLevelTabContainerDialog::connectStationTabTextChangedSignals(bool doConnect) {

    qDebug() << "TopLevelTabContainerDialog::connectStationTabTextChangedSignals: Entered" << doConnect;

#if 0
    // Connect signals from all the QLineEdit items that indicate
    //    data changed so we know to save the data
    QList<QString> keys = db->get_xxx_table_keys(db->db_station_fields);
    QList<QString>::iterator e;
    for (e = keys.begin(); e != keys.end(); ++e) {
        QLineEdit *pLineEdit;
        QString findKey = *e;
        findKey.append("EditBox");
        pLineEdit = pStationDataTab->findChild<QLineEdit *>(findKey);
        Q_ASSERT(pLineEdit);
        if ( doConnect ) {
            if ( connect(pLineEdit, &QLineEdit::textChanged, this,
                        &TopLevelTabContainerDialog::station_data_changed) )
                qDebug() << "TopLevelTabContainerDialog::connectStationTabTextChangedSignals(): Connection valid:" << findKey;
        }
        else {
            if ( disconnect(pLineEdit, &QLineEdit::textChanged, this,
                        &TopLevelTabContainerDialog::station_data_changed) )
                qDebug() << "TopLevelTabContainerDialog::connectStationTabTextChangedSignals(): Connection valid:" << findKey;
        }
    }
#endif
}

void TopLevelTabContainerDialog::user_pressed_save() {

    qDebug() << "TopLevelTabContainerDialog::user_pressed_save(): Entered";
    pStationDataClassPtr = db->getStationDbClassPtr();
    pSysconfigDataClassPtr = db->getSysconfigDbClassPtr();
    pContestDataClassPtr = db->getContestDbClassPtr();

    saveTabbedDataToLocalMap<StationDataTab>(pStationDataTab);
    db->dump_local_station_data();
    saveTabbedDataToLocalMap<SystemConfigTab>(pSysconfigTab);
    db->dump_local_sysconfig_data();
    saveTabbedDataToLocalMap<ContestTab>(pContestTab);
    db->dump_local_contest_data();

    db->syncGeneric_write_to_database_T(pStationDataClassPtr);
    db->syncGeneric_write_to_database_T(pSysconfigDataClassPtr);
    db->syncGeneric_write_to_database_T(pContestDataClassPtr);

    // Disconnect all the QLineEdit signals
    connectStationTabTextChangedSignals(false);
    QDialog::accept();
}

void TopLevelTabContainerDialog::user_pressed_cancel() {
    qDebug() << "TopLevelTabContainerDialog::user_pressed_cancel(): Entered";
    // Disconnect all the QLineEdit signals
    connectStationTabTextChangedSignals(false);
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

    // {"callsign", "opname", "gridsquare", "city"}, "state", "county", "country", "section", "serialport"}
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
    // Delete code in base class
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
    // Delete code in base class
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
    // Delete code in base class
}
