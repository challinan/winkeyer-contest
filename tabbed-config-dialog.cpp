#include "tabbed-config-dialog.h"

TopLevelTabContainerDialog::TopLevelTabContainerDialog(Sqlite3_connector *p, QWidget *parent)
    : QDialog(parent)
{

    db = p;
    qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Entered";
    tabWidget = new QTabWidget;
    pStationDataTab = new StationDataTab(db);
    pContestTab = new ContestTab(db);
    pSystemConfigTab = new SystemConfigTab(db);
    tabWidget->addTab(pStationDataTab, "StationData");
    tabWidget->addTab(pContestTab, "Contest Config");
    tabWidget->addTab(pSystemConfigTab, "System Config");
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
    connectTextChangedSignals();
}

bool TopLevelTabContainerDialog::get_local_station_data_into_dialog() {

    QList<QString> keys = db->get_station_data_table_keys();
    db->dump_local_station_data();

    QList<QString>::iterator e;
    for (e = keys.begin(); e != keys.end(); ++e) {
        //  {0, "callsign", "opname", "gridsquare", "city", "state", "county", "country", "section"}

        if ( *e == QString("callsign") )
            setFieldText("callsign", db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("opname") )
            setFieldText("opname", db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("gridsquare") )
            setFieldText("gridsquare", db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("city") )
            setFieldText("city", db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("state") )
            setFieldText("state", db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("county") )
            setFieldText("county", db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("country") )
            setFieldText("country", db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("section") )
            setFieldText("section", db->get_stataion_data_table_value_by_key(*e));
        else {
            qDebug() << "MainWindow::get_local_station_data(): Invalid station table key:" << *e;
            return false;
        }
    }

    return true;
}

void TopLevelTabContainerDialog::connectTextChangedSignals(bool doConnect) {

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
    connectTextChangedSignals(false);
    QDialog::accept();
}

void TopLevelTabContainerDialog::user_pressed_cancel() {
    qDebug() << "TopLevelTabContainerDialog::user_pressed_cancel(): Entered";
    // Disconnect all the QLineEdit signals
    connectTextChangedSignals(false);
    QDialog::reject();
}

void TopLevelTabContainerDialog::save_tabbed_data_to_database() {

    qDebug() << "TopLevelTabContainerDialog::save_tabbed_data_to_database()";
    // QLineEdit *p;

    QListIterator<QLineEdit *> e(pStationDataTab->editBoxes);
    while (e.hasNext() ) {
        QLineEdit *lep = e.next();
        QString objname = lep->objectName();
        lep = pStationDataTab->findChild<QLineEdit *>(objname);
        objname.remove("EditBox");
        db->set_station_data_table_value_by_key(objname, lep->text());
    }

#if 0
    p = pStationDataTab->findChild<QLineEdit *>("callsignEditBox");
    db->set_station_data_table_value_by_key("callsign", p->text());

    p = pStationDataTab->findChild<QLineEdit *>("opnameEditBox");
    db->set_station_data_table_value_by_key("opname", p->text());

    p = pStationDataTab->findChild<QLineEdit *>("gridsquareEditBox");
    db->set_station_data_table_value_by_key("gridsquare", p->text());

    p = pStationDataTab->findChild<QLineEdit *>("cityEditBox");
    db->set_station_data_table_value_by_key("city", p->text());

    p = pStationDataTab->findChild<QLineEdit *>("stateEditBox");
    db->set_station_data_table_value_by_key("state", p->text());

    p = pStationDataTab->findChild<QLineEdit *>("countyEditBox");
    db->set_station_data_table_value_by_key("county", p->text());

    p = pStationDataTab->findChild<QLineEdit *>("countryEditBox");
    db->set_station_data_table_value_by_key("country", p->text());

    p = pStationDataTab->findChild<QLineEdit *>("sectionEditBox");
    db->set_station_data_table_value_by_key("section", p->text());
#endif

    db->syncStationData_write();
}

void TopLevelTabContainerDialog::setFieldText(QString key, QString value) {

    QLineEdit *p;
    QString childName = key;
    childName.append("EditBox");
    // qDebug() << "TopLevelTabContainerDialog::setFieldText(): childName =" << childName;

    p = pStationDataTab->findChild<QLineEdit *>(childName);
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
    delete pSystemConfigTab;
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
        editBoxes.append(qle); // Add this one to the QList<QLineEdit> list
        QString objname = key;
        objname.append("EditBox");
        qle->setObjectName(objname);
        formLayout->addRow(db->text_labels_for_keys[key], qle);
        qDebug() << "StationDataTab::StationDataTab():" << key << db->text_labels_for_keys[key];
    }
    setLayout(formLayout);

    // My screen is 2560 x 1664  (640*4) x 1664
    // QRect r = callsignEditBox->frameGeometry();
    // qDebug() << "StationDataTab::StationDataTab(): Geo: x="<< r.x() << "y=" << r.y() << "h=" << r.height() << "w=" << r.width();
    // r = this->frameGeometry();
    // qDebug() << "StationDataTab::StationDataTab(): Geo: x="<< r.x() << "y=" << r.y() << "h=" << r.height() << "w=" << r.width();
}

StationDataTab::~StationDataTab() {

    // Delete all the QLineEdit objects
    QListIterator<QLineEdit *> e(editBoxes);
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
    QLabel *topLabel = new QLabel(tr("Open with:"));

    QListWidget *applicationsListBox = new QListWidget;
    QStringList applications;

    for (int i = 1; i <= 30; ++i)
        applications.append(tr("Application %1").arg(i));
    applicationsListBox->insertItems(0, applications);

    QCheckBox *alwaysCheckBox;
    alwaysCheckBox = new QCheckBox(tr("My Check Box"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(topLabel);
    layout->addWidget(applicationsListBox);
    layout->addWidget(alwaysCheckBox);
    setLayout(layout);
}
