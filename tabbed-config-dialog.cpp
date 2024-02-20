#include <QString>
#include <QPushButton>

#include "tabbed-config-dialog.h"

TopLevelTabContainerDialog::TopLevelTabContainerDialog(Sqlite3_connector *p, QWidget *parent)
    : QDialog(parent)
{
    db = p;
    qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Entered";
    tabWidget = new QTabWidget;

    pStationDataTab = new StationDataTab(db);
    pContestTab = new ContestTab(db);
    pSysconfigTab = new SystemConfigTab(db);

    // Get pointers to Database Sub-classes - yup, this is super ugly
    pStationDataClassPtr = db->getStationDbClassPtr();
    pSysconfigDataClassPtr = db->getSysconfigDbClassPtr();
    pContestDataClassPtr = db->getContestDbClassPtr();

    tabWidget->addTab(pStationDataTab, "StationData");
    tabWidget->addTab(pContestTab, "Contest Config");
    tabWidget->addTab(pSysconfigTab, "System Config");
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
#if 0
    if ( db_state != DB_NOEXIST &&  db_state != DB_NOTABLES ) {
        get_local_data_into_dialog_T(pStationDataClassPtr);
        get_local_data_into_dialog_T(pSysconfigDataClassPtr);
        get_local_data_into_dialog_T(pContestDataClassPtr);
    } else {
        qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Skipping database read into tabs";
    }
#endif

    // Connect potential signals here
    connectStationTabTextChangedSignals();

}

template<typename T>
bool TopLevelTabContainerDialog::get_local_data_into_dialog_T(T *pDataClassPtr) {

    // Get pointer to StationData db fields
    const QMap<int, QString> &db_fields = pDataClassPtr->getDbFields();
    QMap<int, QString> &pMap = pDataClassPtr->getLocalDataMap();

    // For debug only
    // db->dump_local_station_data();

    QMapIterator<int, QString> e(db_fields);
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
        QString childName = e.value();    // Might be "callsign", or "opname", etc
        childName.append("EditBox");
        qDebug() << "TopLevelTabContainerDialog::get_local_data_into_dialog_T(): childName =" << childName;  // will be "callsignEditBox", etc.

        p = findChild<QLineEdit *>(childName);
        if ( p == nullptr ) {
            qDebug() << "TopLevelTabContainerDialog::get_local_data_into_dialog_T(): child" << childName << "not found";
            return false;
        }
        // TODO: This is as ugly as it gets.  What I really need is a 3-ple table {key, fieldname, value}
        p->setText(pMap.value(e.key()-1));
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

    save_tabbed_data_to_local_database_map_T<StationDataTab>(pStationDataTab);
    db->dump_local_station_data();
    save_tabbed_data_to_local_database_map_T<SystemConfigTab>(pSysconfigTab);
    db->dump_local_sysconfig_data();
    save_tabbed_data_to_local_database_map_T<ContestTab>(pContestTab);
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
    delete formLayout;


}


// ******************  StationDataTab  **************************** //
StationDataTab::StationDataTab(Sqlite3_connector *p, QWidget *parent)
    : DataTab(p, parent)
{
    db = p;     // Pointer to sqlite3_connector database engine

    StationData *pStationDataClassPtr = db->getStationDbClassPtr();

    // Get pointer to StationData db fields
    QMap<int, QString> db_fields = pStationDataClassPtr->getDbFields();
    QMap<QString, QString> text_label_fields = pStationDataClassPtr->getTextLabelFields();

    formLayout = new QFormLayout(this);

    // {"callsign", "opname", "gridsquare", "city"}, "state", "county", "country", "section", "serialport"}
    QMapIterator<int, QString> e(db_fields);

    // First entry is the tablename
    if (e.hasNext() )
        e.next();   // Skip over table name

    while ( e.hasNext() ) {
        e.next();
        QString field_name = e.value();

        // Create an appropriate QLineEdit, put it in the List, give it an objname, and add it to layout
        QLineEdit *qle = new QLineEdit;

        QString objname = field_name;
        objname.append("EditBox");
        qle->setObjectName(objname);
        dataEditBoxes.append(qle); // Add this one to the QList<QLineEdit> list
        qDebug() << "StationDataTab::StationDataTab(): Adding editBox to station data tab" << qle << qle->objectName();


        QString editBoxLabel = text_label_fields.value(field_name);
        formLayout->addRow(editBoxLabel, qle);
        // qDebug() << "StationDataTab::StationDataTab(): EditBox Object Name:" << objname << "key =" << key << "editBoxLabel = " << editBoxLabel;
    }
    setLayout(formLayout);
}

void StationDataTab::set_xxx_table_value_by_key(int key, QString &value) {
    db->set_station_data_table_value_by_key(key, value);
}


StationDataTab::~StationDataTab() {
    // Delete code in base class
}

ContestTab::ContestTab(Sqlite3_connector *p, QWidget *parent)
    : DataTab(p, parent)
{
    db = p;     // Pointer to sqlite3_connector database engine

    ContestData *pContestDataClassPtr = db->getContestDbClassPtr();

    // Get pointer to StationData db fields
    QMap<int, QString> db_fields = pContestDataClassPtr->getDbFields();
    QMap<QString, QString> text_label_fields = pContestDataClassPtr->getTextLabelFields();

    formLayout = new QFormLayout(this);

    // {"sequence", "section", "rst"}

    QMapIterator<int, QString> e(db_fields);
    // First entry is the tablename
    if (e.hasNext() ) {
        // Skip over table name - we don't need it here
        e.next();
    }

    while (e.hasNext() ) {
        QString key = e.next().value();

        // Create an appropriate QLineEdit, put it in the List, give it an objname, and add it to layout
        QLineEdit *qle = new QLineEdit;
        dataEditBoxes.append(qle); // Add this one to the QList<QLineEdit> list
        QString objname = key;
        objname.append("EditBox");
        qle->setObjectName(objname);

        QString editBoxLabel = text_label_fields.value(key);
        formLayout->addRow(editBoxLabel, qle);
        // qDebug() << "ContestTab::ContestTab(): EditBox Object Name:" << objname << "key =" << key << "editBoxLabel = " << editBoxLabel;
    }
    setLayout(formLayout);
}

void ContestTab::set_xxx_table_value_by_key(int key, QString &value) {
    db->set_contest_table_value_by_key(key, value);
}

ContestTab::~ContestTab() {
    // Delete code in base class
}

SystemConfigTab::SystemConfigTab(Sqlite3_connector *p, QWidget *parent)
    : DataTab(p, parent)
{
    db = p;     // Pointer to sqlite3_connector database engine

    SysconfigData *pSysconfigDataClassPtr = db->getSysconfigDbClassPtr();

    // Get pointer to StationData db fields
    QMap<int, QString> db_fields = pSysconfigDataClassPtr->getDbFields();
    QMap<QString, QString> text_label_fields = pSysconfigDataClassPtr->getTextLabelFields();

    formLayout = new QFormLayout(this);

    // {"serialport", "audioinport", audiooutport"}
    QMapIterator<int, QString> e(db_fields);

    // First entry is the tablename
    if (e.hasNext() ) {
        // Skip over table name - we don't need it here
        e.next();
    }

    while ( e.hasNext() ) {
        e.next();
        QString key = e.value();

        // Create an appropriate QLineEdit, put it in the List, give it an objname, and add it to layout
        QLineEdit *qle = new QLineEdit;
        dataEditBoxes.append(qle); // Add this one to the QList<QLineEdit> list

        QString objname = key;
        objname.append("EditBox");
        qle->setObjectName(objname);

        QString editBoxLabel = text_label_fields.value(key);
        formLayout->addRow(editBoxLabel, qle);
        // qDebug() << "SystemConfigTab::SystemConfigTab(): EditBox Object Name:" << objname << "key =" << key << "editBoxLabel = " << editBoxLabel;
    }
    setLayout(formLayout);
}

void SystemConfigTab::set_xxx_table_value_by_key(int key, QString &value) {
    db->set_sysconfig_table_value_by_key(key, value);
}

SystemConfigTab::~SystemConfigTab() {
    // Delete code in base class
}
