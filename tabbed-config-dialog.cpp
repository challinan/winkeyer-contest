#include "tabbed-config-dialog.h"

TopLevelTabContainerDialog::TopLevelTabContainerDialog(Sqlite3_connector *p, QWidget *parent)
    : QDialog(parent)
{

    db = p;
    qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Entered";
    tabWidget = new QTabWidget;
    tabWidget->addTab(new StationDataTab(p), "StationData");
    tabWidget->addTab(new ContestTab(p), "Contest Config");
    tabWidget->addTab(new SystemConfigTab(p), "System Config");
    qDebug() << "TopLevelTabContainerDialog::TopLevelTabContainerDialog(): Tabs Added";

    // Create two standard push buttons, and connect each of them to the appropriate slots in the dialog
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);


    // Arrange the tab widget above the buttons in the dialog
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    // Set the dialog's title
    setWindowTitle(tr("Configuration Dialog"));
}

TopLevelTabContainerDialog::~TopLevelTabContainerDialog() {
    delete tabWidget;
}

StationDataTab::StationDataTab(Sqlite3_connector *p, QWidget *parent)
    : QWidget(parent)
{
    db = p;     // Pointer to sqlite3_connector database engine
    QList<QString> keys = db->get_station_data_table_keys();

    QListIterator<QString> e(keys);
    while (e.hasNext() ) {
        QString item = e.next();
        qDebug() << "StationDataTab::StationDataTab():" << item << db->text_labels_for_keys[item];
    }

    QFormLayout *formLayout = new QFormLayout(this);

    // Callsign
    QLineEdit *callSignEditBox = new QLineEdit(this);
    formLayout->addRow("Call Sign", callSignEditBox);
    // callSignEditBox->setGeometry(QRect(10, 60, 128, 21));

    // Name
    QLineEdit *nameEditBox = new QLineEdit(this);
    formLayout->addRow("Name", nameEditBox);

    // City
    QLineEdit *cityEditBox = new QLineEdit(this);
    formLayout->addRow("City", cityEditBox);

    // State
    QLineEdit *stateEditBox = new QLineEdit(this);
    formLayout->addRow("State", stateEditBox);

    // GridSquare
    QLineEdit *gridSquareEditBox = new QLineEdit(this);
    formLayout->addRow("Grid Sqaure", gridSquareEditBox);

    // County
    QLineEdit *countyEditBox = new QLineEdit(this);
    formLayout->addRow("County", countyEditBox);

    // Country
    QLineEdit *countryEditBox = new QLineEdit(this);
    formLayout->addRow("Country", countryEditBox);

    // ARRL Section
    QLineEdit *sectionEditBox = new QLineEdit(this);
    formLayout->addRow("Arrl Section", sectionEditBox);

    setLayout(formLayout);

#if 0
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(callSignLabel);
    mainLayout->addWidget(callSignEditBox);
    mainLayout->addWidget(cityLabel);
    mainLayout->addWidget(cityEditBox);
    mainLayout->addWidget(stateLabel);
    mainLayout->addWidget(stateEditBox);
    mainLayout->addWidget(gridSquareLabel);
    mainLayout->addWidget(gridSquareEditBox);
    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(nameEditBox);
    mainLayout->addWidget(countyLabel);
    mainLayout->addWidget(countyEditBox);
    mainLayout->addWidget(countryLabel);
    mainLayout->addWidget(countryEditBox);
    mainLayout->addWidget(sectionLabel);
    mainLayout->addWidget(sectionEditBox);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
#endif

    // My screen is 2560 x 1664  (640*4) x 1664
    QRect r = callSignEditBox->frameGeometry();
    qDebug() << "StationDataTab::StationDataTab(): Geo: x="<< r.x() << "y=" << r.y() << "h=" << r.height() << "w=" << r.width();
    r = this->frameGeometry();
    qDebug() << "StationDataTab::StationDataTab(): Geo: x="<< r.x() << "y=" << r.y() << "h=" << r.height() << "w=" << r.width();

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
