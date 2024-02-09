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
    tabWidget->addTab(pStationDataTab, "StationData");
    tabWidget->addTab(pContestTab, "Contest Config");
    tabWidget->addTab(pSysconfigTab, "System Config");
    qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Tabs Added";

    // const QMap<int, QString>
    pStationKeyFields = &db->db_station_fields;
    pSysconfigKeyFields = &db->db_sysconfig_fields;
    pContestKeyFields = &db->db_contest_fields;

    // Create two standard push buttons, and connect each of them to the appropriate slots in the dialog
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

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
    get_local_station_data_into_dialog();
    get_local_sysconfig_data_into_dialog();
    get_local_contest_data_into_dialog();
    connectStationTabTextChangedSignals();
}

bool TopLevelTabContainerDialog::get_local_station_data_into_dialog() {

    QList<QString> keys = db->get_xxx_table_keys(db->db_station_fields);

    // For debug only
    // db->dump_local_station_data();

    QList<QString>::iterator e;
    for (e = keys.begin(); e != keys.end(); ++e) {
        //  {0, "callsign", "opname", "gridsquare", "city", "state", "county", "country", "section"}

        if ( *e == QString("callsign") )
            setFieldText<class StationDataTab>(pStationDataTab, "callsign", db->get_station_data_table_value_by_key(*e));
        else if ( *e == QString("opname") )
            setFieldText<class StationDataTab>(pStationDataTab, "opname", db->get_station_data_table_value_by_key(*e));
        else if ( *e == QString("gridsquare") )
            setFieldText<class StationDataTab>(pStationDataTab, "gridsquare", db->get_station_data_table_value_by_key(*e));
        else if ( *e == QString("city") )
            setFieldText<class StationDataTab>(pStationDataTab, "city", db->get_station_data_table_value_by_key(*e));
        else if ( *e == QString("state") )
            setFieldText<class StationDataTab>(pStationDataTab, "state", db->get_station_data_table_value_by_key(*e));
        else if ( *e == QString("county") )
            setFieldText<class StationDataTab>(pStationDataTab, "county", db->get_station_data_table_value_by_key(*e));
        else if ( *e == QString("country") )
            setFieldText<class StationDataTab>(pStationDataTab, "country", db->get_station_data_table_value_by_key(*e));
        else if ( *e == QString("section") )
            setFieldText<class StationDataTab>(pStationDataTab, "section", db->get_station_data_table_value_by_key(*e));
        else {
            qDebug() << "MainWindow::get_local_station_data(): Invalid station table key:" << *e;
            return false;
        }
    }

    return true;
}

bool TopLevelTabContainerDialog::get_local_sysconfig_data_into_dialog() {

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

    return true;
}

bool TopLevelTabContainerDialog::get_local_contest_data_into_dialog() {

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

    return true;
}

void TopLevelTabContainerDialog::connectStationTabTextChangedSignals(bool doConnect) {

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
}

void TopLevelTabContainerDialog::user_pressed_save() {

    qDebug() << "TopLevelTabContainerDialog::user_pressed_save(): Entered";
    // save_tabbed_data_to_database();
    save_tabbed_data_to_database_template<StationDataTab>(pStationDataTab);
    save_tabbed_data_to_database_template<SystemConfigTab>(pSysconfigTab);
    save_tabbed_data_to_database_template<ContestTab>(pContestTab);
    db->syncGeneric_write();

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


template <typename T>void TopLevelTabContainerDialog::setFieldText(T *tabPtr, QString key, QString value) {

    QLineEdit *p;
    T *pTab = tabPtr;
    QString childName = key;
    childName.append("EditBox");
    qDebug() << "TopLevelTabContainerDialog::setFieldText(): childName =" << childName;

    p = pTab->template findChild<QLineEdit *>(childName);
    Q_ASSERT(p);
    p->setText(value);
    // db->set_station_data_table_value_by_key("callsign", p->text());
}

void TopLevelTabContainerDialog::station_data_changed() {
    qDebug() << "TopLevelTabContainerDialog::station_data_changed(): **********>>> Entered";
}

#define USE_TEMPLATE
#ifdef USE_TEMPLATE
#else
StationDataTab *TopLevelTabContainerDialog::getStationDataTabPtr() {
    return pStationDataTab;
}
#endif

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

}


// ******************  StationDataTab  **************************** //
StationDataTab::StationDataTab(Sqlite3_connector *p, QWidget *parent)
    : DataTab(p, parent)
{
    db = p;     // Pointer to sqlite3_connector database engine
    formLayout = new QFormLayout(this);

    QList<QString> keys = db->get_xxx_table_keys(db->db_station_fields);

    // {"callsign", "opname", "gridsquare", "city"}, "state", "county", "country", "section", "serialport"}
    QListIterator<QString> e(keys);
    while (e.hasNext() ) {
        QString key = e.next();

        // Create an appropriate QLineEdit, put it in the List, give it an objname, and add it to layout
        QLineEdit *qle = new QLineEdit;
        dataEditBoxes.append(qle); // Add this one to the QList<QLineEdit> list
        QString objname = key;
        objname.append("EditBox");
        qle->setObjectName(objname);
        formLayout->addRow(db->text_labels_for_station_data_keys[key], qle);
        qDebug() << "StationDataTab::StationDataTab():" << key << db->text_labels_for_station_data_keys[key];
    }
    setLayout(formLayout);
}

void StationDataTab::set_xxx_table_value_by_key(QString objname, QString value) {
    db->set_station_data_table_value_by_key(objname, value);
}


// TODO: Combine this code with other tab destructors - make body static in top level tab?
StationDataTab::~StationDataTab() {

    // Delete all the QLineEdit objects
    QListIterator<QLineEdit *> e(dataEditBoxes);
    while (e.hasNext() ) {
        // QLineEdit *p = e.next();
        // qDebug () << "Deleteing: ******* " << p->objectName();
        // delete p;
        delete e.next();
    }
    delete formLayout;
}

ContestTab::ContestTab(Sqlite3_connector *p, QWidget *parent)
    : DataTab(p, parent)
{
    db = p;     // Pointer to sqlite3_connector database engine

    formLayout = new QFormLayout(this);
    QList<QString> keys = db->get_xxx_table_keys(db->db_contest_fields);

    // {"sequence", "section", "rst"}

    QListIterator<QString> e(keys);
    while (e.hasNext() ) {
        QString key = e.next();

        // Create an appropriate QLineEdit, put it in the List, give it an objname, and add it to layout
        QLineEdit *qle = new QLineEdit;
        dataEditBoxes.append(qle); // Add this one to the QList<QLineEdit> list
        QString objname = key;
        objname.append("EditBox");
        qle->setObjectName(objname);

        formLayout->addRow(db->text_labels_for_contest_keys[key], qle);
        qDebug() << "ContestTab::ContestTab():" << key << db->text_labels_for_sysconfig_keys[key];
    }
    setLayout(formLayout);
}

void ContestTab::set_xxx_table_value_by_key(QString key, QString value) {
    db->set_contest_table_value_by_key(key, value);
}

ContestTab::~ContestTab() {

    // Delete all the QLineEdit objects
    QListIterator<QLineEdit *> e(dataEditBoxes);
    while (e.hasNext() ) {
        // QLineEdit *p = e.next();
        // qDebug () << "Deleteing: ******* " << p->objectName();
        // delete p;
        delete e.next();
    }
    delete formLayout;
}

SystemConfigTab::SystemConfigTab(Sqlite3_connector *p, QWidget *parent)
    : DataTab(p, parent)
{
    db = p;     // Pointer to sqlite3_connector database engine

    formLayout = new QFormLayout(this);

    QList<QString> keys = db->get_xxx_table_keys(db->db_sysconfig_fields);

    // {"serialport", "audiooutput", "audioinput"}

    QListIterator<QString> e(keys);
    while (e.hasNext() ) {
        QString key = e.next();

        // Create an appropriate QLineEdit, put it in the List, give it an objname, and add it to layout
        QLineEdit *qle = new QLineEdit;
        dataEditBoxes.append(qle); // Add this one to the QList<QLineEdit> list
        QString objname = key;
        objname.append("EditBox");
        qle->setObjectName(objname);
        // These fields could be long strings
        qle->setMinimumWidth(275);
        formLayout->addRow(db->text_labels_for_sysconfig_keys[key], qle);
        qDebug() << "SystemConfigTab::SystemConfigTab():" << key << db->text_labels_for_sysconfig_keys[key];
    }
    setLayout(formLayout);
}

void SystemConfigTab::set_xxx_table_value_by_key(QString key, QString value) {
    db->set_sysconfig_table_value_by_key(key, value);
}

// TODO: Combine this code with other tab destructors - make body static in top level tab?
SystemConfigTab::~SystemConfigTab() {

    // Delete all the QLineEdit objects
    QListIterator<QLineEdit *> e(dataEditBoxes);
    while (e.hasNext() ) {
        // QLineEdit *p = e.next();
        // qDebug () << "Deleteing: ******* " << p->objectName();
        // delete p;
        delete e.next();
    }
    delete formLayout;
}
