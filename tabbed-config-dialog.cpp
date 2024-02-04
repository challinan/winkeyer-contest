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

    // Create two standard push buttons, and connect each of them to the appropriate slots in the dialog
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &TopLevelTabContainerDialog::user_pressed_save);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TopLevelTabContainerDialog::user_pressed_cancel);

    // Arrange the tab widget above the buttons in the dialog
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    // Set the dialog's title
    setWindowTitle(tr("Configuration Dialog"));

    // If data is available, populate it here
    get_local_station_data_into_dialog();
    get_local_sysconfig_data_into_dialog();
    connectStationTabTextChangedSignals();
}

bool TopLevelTabContainerDialog::get_local_station_data_into_dialog() {

    QList<QString> keys = db->get_station_data_table_keys();
    db->dump_local_station_data();

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

    QList<QString> keys = db->get_sysconfig_table_keys();
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

void TopLevelTabContainerDialog::connectStationTabTextChangedSignals(bool doConnect) {

    // Connect signals from all the QLineEdit items that indicate
    //    data changed so we know to save the data
    QList<QString> keys = db->get_station_data_table_keys();
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
    save_tabbed_data_to_database();

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

void TopLevelTabContainerDialog::save_tabbed_data_to_database() {

    qDebug() << "TopLevelTabContainerDialog::save_tabbed_data_to_database()";

    // Store away the station_data values from the station data tab
    // The QLineEdit objects in the Station Data tab are enumerated in the editBoxes list
    QListIterator<QLineEdit *> e(pStationDataTab->stationDataEditBoxes);
    while (e.hasNext() ) {
        QLineEdit *lep = e.next();
        QString objname = lep->objectName();
        lep = pStationDataTab->findChild<QLineEdit *>(objname);
        objname.remove("EditBox");
        db->set_station_data_table_value_by_key(objname, lep->text());
    }
    db->syncStationData_write();

    // Store away the sysconfig values from the system config tab
    // The QLineEdit objects in the System Config tab are enumerated in the editBoxes list
    QListIterator<QLineEdit *> s(pSysconfigTab->sysconfigEditBoxes);
    while (s.hasNext() ) {
        QLineEdit *lep = s.next();
        QString objname = lep->objectName();
        lep = pStationDataTab->findChild<QLineEdit *>(objname);
        objname.remove("EditBox");
        db->set_station_data_table_value_by_key(objname, lep->text());
    }
    db->syncSysconfigData_write();
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
    qDebug() << "TopLevelTabContainerDialog::station_data_changed(): *******************************>>> Entered";
}


TopLevelTabContainerDialog::~TopLevelTabContainerDialog() {
    delete pStationDataTab;
    delete pContestTab;
    delete pSysconfigTab;
    delete tabWidget;
}

// ******************  StationDataTab  **************************** //
StationDataTab::StationDataTab(Sqlite3_connector *p, QWidget *parent)
    : QWidget(parent)
{
    db = p;     // Pointer to sqlite3_connector database engine
    QFormLayout *formLayout = new QFormLayout(this);

    QList<QString> keys = db->get_station_data_table_keys();

    // {"callsign", "opname", "gridsquare", "city"}, "state", "county", "country", "section", "serialport"}
    QListIterator<QString> e(keys);
    while (e.hasNext() ) {
        QString key = e.next();

        // Create an appropriate QLineEdit, put it in the List, give it an objname, and add it to layout
        QLineEdit *qle = new QLineEdit;
        stationDataEditBoxes.append(qle); // Add this one to the QList<QLineEdit> list
        QString objname = key;
        objname.append("EditBox");
        qle->setObjectName(objname);
        formLayout->addRow(db->text_labels_for_station_data_keys[key], qle);
        qDebug() << "StationDataTab::StationDataTab():" << key << db->text_labels_for_station_data_keys[key];
    }
    setLayout(formLayout);

    // My screen is 2560 x 1664  (640*4) x 1664
    // QRect r = callsignEditBox->frameGeometry();
    // qDebug() << "StationDataTab::StationDataTab(): Geo: x="<< r.x() << "y=" << r.y() << "h=" << r.height() << "w=" << r.width();
    // r = this->frameGeometry();
    // qDebug() << "StationDataTab::StationDataTab(): Geo: x="<< r.x() << "y=" << r.y() << "h=" << r.height() << "w=" << r.width();
}

// TODO: Combine this code with other tab destructors - make body static in top level tab?
StationDataTab::~StationDataTab() {

    // Delete all the QLineEdit objects
    QListIterator<QLineEdit *> e(stationDataEditBoxes);
    while (e.hasNext() ) {
        // QLineEdit *p = e.next();
        // qDebug () << "Deleteing: ******* " << p->objectName();
        // delete p;
        delete e.next();
    }
}

ContestTab::ContestTab(Sqlite3_connector *p, QWidget *parent)
    : QWidget(parent)
{
    db = p;     // Pointer to sqlite3_connector database engine
    QGroupBox *permissionsGroup = new QGroupBox(tr("Permissions"));

    QCheckBox *readable = new QCheckBox(tr("Readable"));

    QCheckBox *writable = new QCheckBox(tr("Writable"));

    QCheckBox *executable = new QCheckBox(tr("Executable"));

    QGroupBox *ownerGroup = new QGroupBox(tr("Ownership"));

    QVBoxLayout *permissionsLayout = new QVBoxLayout;
    permissionsLayout->addWidget(readable);
    permissionsLayout->addWidget(writable);
    permissionsLayout->addWidget(executable);
    permissionsGroup->setLayout(permissionsLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(permissionsGroup);
    mainLayout->addWidget(ownerGroup);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

SystemConfigTab::SystemConfigTab(Sqlite3_connector *p, QWidget *parent)
    : QWidget(parent)
{
    db = p;     // Pointer to sqlite3_connector database engine

    QFormLayout *formLayout = new QFormLayout(this);

    QList<QString> keys = db->get_sysconfig_table_keys();

    // {"serialport", "audiooutput", "audioinput"}

    QListIterator<QString> e(keys);
    while (e.hasNext() ) {
        QString key = e.next();

        // Create an appropriate QLineEdit, put it in the List, give it an objname, and add it to layout
        QLineEdit *qle = new QLineEdit;
        sysconfigEditBoxes.append(qle); // Add this one to the QList<QLineEdit> list
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

// TODO: Combine this code with other tab destructors - make body static in top level tab?
SystemConfigTab::~SystemConfigTab() {

    // Delete all the QLineEdit objects
    QListIterator<QLineEdit *> e(sysconfigEditBoxes);
    while (e.hasNext() ) {
        // QLineEdit *p = e.next();
        // qDebug () << "Deleteing: ******* " << p->objectName();
        // delete p;
        delete e.next();
    }
}
