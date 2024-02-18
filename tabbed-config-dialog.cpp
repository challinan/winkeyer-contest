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

    // If data is available, populate it here
    database_state db_state = db->getDatabaseState();
    if ( db_state != DB_NOEXIST &&  db_state != DB_NOTABLES ) {
        qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Reading database data into tabs";
        get_local_data_into_dialog_T(pStationDataClassPtr);
        get_local_data_into_dialog_T(pSysconfigDataClassPtr);
        get_local_data_into_dialog_T(pContestDataClassPtr);
    } else {
        qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Skipping database read into tabs";
    }

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
        //  {"callsign", "opname", "gridsquare", "city", "state", "county", "country", "section"}

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

bool TopLevelTabContainerDialog::get_local_station_data_into_dialog() {

    // Get pointer to StationData db fields
    const QMap<int, QString> &db_fields = pStationDataClassPtr->getDbFields();
    QMap<int, QString> &pMap = pStationDataClassPtr->getLocalDataMap();

    // For debug only
    // db->dump_local_station_data();

    QMapIterator<int, QString> e(db_fields);
    if ( e.hasNext() ) {
        e.next();   // Skip over the table name
    } else {
        qDebug() << "TopLevelTabContainerDialog::get_local_station_data_into_dialog(): Failed - expected more rows in db_fields";
        return false;
    }

    while ( e.hasNext() ) {
        e.next();
        //  {"callsign", "opname", "gridsquare", "city", "state", "county", "country", "section"}

        QLineEdit *p;
        QString childName = e.value();    // Might be "callsign", or "opname", etc
        childName.append("EditBox");
        qDebug() << "TopLevelTabContainerDialog::get_local_station_data_into_dialog(): childName =" << childName;  // will be "callsignEditBox", etc.

        p = findChild<QLineEdit *>(childName);
        if ( p == nullptr ) {
            qDebug() << "TopLevelTabContainerDialog::get_local_station_data_into_dialog(): child" << childName << "not found";
            return false;
        }
        // TODO: This is as ugly as it gets.  What I really need is a 3-ple table {key, fieldname, value}
        p->setText(pMap.value(e.key()-1));
    }
    return true;
}

bool TopLevelTabContainerDialog::get_local_sysconfig_data_into_dialog() {
#if 0

    QList<QString> keys = db->get_xxx_table_keys(db->db_sysconfig_fields);
    db->dump_local_sysconfig_data();

    QList<QString>::iterator e;
    for (e = keys.begin(); e != keys.end(); ++e) {
        // {"serialport", "audiooutput", "audioinput"}

        if ( *e == QString("serialport") )
            setFieldText<class SystemConfigTab>(pSysconfigTab, "serialport", db->get_sysconfig_table_value_by_key(*e));
        else if ( *e == QString("audiooutput") )
            setFieldText<class SystemConfigTab>(pSysconfigTab, "audiooutput", db->get_sysconfig_table_value_by_key(*e));
        else if ( *e == QString("audioinput") )
            setFieldText<class SystemConfigTab>(pSysconfigTab, "audioinput", db->get_sysconfig_table_value_by_key(*e));
        else {
            qDebug() << "TopLevelTabContainerDialog::get_local_sysconfig_data_into_dialog(): Invalid station table key:" << *e;
            return false;
        }
    }

#endif
    return true;
}

bool TopLevelTabContainerDialog::get_local_contest_data_into_dialog() {
#if 0

    QList<QString> keys = db->get_xxx_table_keys(db->db_contest_fields);
    // db->dump_local_contest_data();

    QList<QString>::iterator e;
    for (e = keys.begin(); e != keys.end(); ++e) {
        // {"sequence", "section", "rst"}

        if ( *e == QString("sequence") )
            setFieldText<class ContestTab>(pContestTab, "sequence", db->get_contest_table_value_by_key(*e));
        else if ( *e == QString("section") )
            setFieldText<class ContestTab>(pContestTab, "section", db->get_contest_table_value_by_key(*e));
        else if ( *e == QString("rst") )
            setFieldText<class ContestTab>(pContestTab, "rst", db->get_contest_table_value_by_key(*e));
        else {
            qDebug() << "TopLevelTabContainerDialog::get_local_contest_data_into_dialog(): Invalid station table key:" << *e;
            return false;
        }
    }
#endif
    return true;
}

void TopLevelTabContainerDialog::connectStationTabTextChangedSignals(bool doConnect) {

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
                qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Connection valid:" << findKey;
        }
        else {
            if ( disconnect(pLineEdit, &QLineEdit::textChanged, this,
                        &TopLevelTabContainerDialog::station_data_changed) )
                qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Connection valid:" << findKey;
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

// Sets a value into each EditBox in the tabbed dialog
template <typename T>
void TopLevelTabContainerDialog::setFieldText(T *tabPtr, int key, QString value) {

    qDebug() << "TopLevelTabContainerDialog::setFieldText(): Entered: key" << key << "value" << value;
#if 0
    QList<QMap<int, QString>> &table_list = db->getTableList();

    QLineEdit *p;
    T *pTab = tabPtr;
    // QString childName = key;    // Might be "callsign", or "opname"
    childName.append("EditBox");
    qDebug() << "TopLevelTabContainerDialog::setFieldText(): childName =" << childName;  // will be "callsignEditBox", etc.

    p = pTab->template findChild<sQLineEdit *>(childName);
    p->setText(value);
    db->set_station_data_table_value_by_key(key, p->text());
#endif
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
        qDebug () << "Deleteing: ******* " << p->objectName();
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


// TODO: Combine this code with other tab destructors - make body static in top level tab?
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
