#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_station_data.h"

#define SKIP_SERIAL_PORT_INIT
// #define DBCONFIG_DEBUG  // When enabled, erases the db file on every run

void MainWindow::waitForVisible() {
    int count = 0;
    while ( isVisible() == false ) {
        count++;
    }
    qDebug() << "MainWindow::waitForVisible(void): count =" << count;
    if ( !initialize_mainwindow() ) {
        qDebug() << "MainWindow::waitForVisible(): Exiting app";
        QCoreApplication::exit(-3);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    initialization_succeeded = true;
    init_called_once = false;
    qDebug() << "MainWindow::MainWindow(): Ctor Entered";
    ui->setupUi(this);
    connect(this, &MainWindow::waitVisibleSignal, this, &MainWindow::waitForVisible, Qt::QueuedConnection);
    // ((object).*(ptrToMember))
    // typedef double (CA::*CAGetter)(const double x);
    // typedef int (MyClass::*MyFuncPtr)(int, int);
    // MyFuncPtr my_func_ptr = &MyClass::add;
    typedef void (QPushButton::*myFnPtr)(bool);
    // myFnPtr p = &ui->configPushButton->clicked;
    // connect(ui, p, this, &MainWindow::launchConfigDialog, Qt::QueuedConnection);

    // Entry to main event loop starts when this constructor returns
    qDebug() << "MainWindow::MainWindow(): Ctor Exiting complete";
}

bool MainWindow::initSucceeded() {
    return initialization_succeeded;
}

bool MainWindow::initialize_mainwindow() {

#ifdef DBCONFIG_DEBUG
    // For debug only - delete when done
    QString dbFile = "/Users/chris/.macrr/winkeyer_db";
    QFile file (dbFile);
    file.remove();
    qDebug() << "MainWindow::initialize_mainwindow(): DATABASE FILE REMOVED";
#endif

    // Initialize and sync internal and external station database
    db = new Sqlite3_connector;
    c_invoke_config_dialog =
        connect(db, &Sqlite3_connector::do_config_dialog, this, &MainWindow::launchConfigDialog, Qt::QueuedConnection);
    qDebug() << "MainWindow::initialize_mainwindow(): Called connect: c_invoke_config_dialog = " << c_invoke_config_dialog;

    if ( !db->initDatabase() ) {
        qDebug() << "MainWindow::initialize_mainwindow(): database init failed in Sqlite3_connector:: constructor";
        db->setInitStatus(false);
        return false;
    }

    if ( !db->dbInitSucceeded() ) {
        initialization_succeeded = false;
        QCoreApplication::exit(2);
    }

// Initialize serial port object
#ifndef SKIP_SERIAL_PORT_INIT
    serial_comms_p = new SerialComms();
    db->setSerialPtr(serial_comms_p);
    serial_comms_p->openSerialPort();
    connect(serial_comms_p, &SerialComms::on_serial_port_detected, this, &MainWindow::serial_port_detected);

    // textCursor = QTextCursor(ui->plainTextEdit->textCursor());
    // ui->plainTextEdit->setFocusPolicy(Qt::StrongFocus);
#endif

    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::changeConfigButtonTextColor));
    blinkTimer->start(350);

    return true;
}

void MainWindow::launchConfigDialog() {

    // Bring up our tabbed config dialog box
    qDebug() << "MainWindow::launchConfigDialog(): Entered";
    tabbedDialog = new TopLevelTabContainerDialog(db);
    tabbedDialog->show();
    tabbedDialog->exec();
    // Store the data here
    delete tabbedDialog;
}

void MainWindow::changeConfigButtonTextColor() {

    // TEST TEST TEST TEST
    // stylesheet = "QPushButton {color: red;}";

    static bool toggle = false;
    QString stylesheet = ui->configPushButton->styleSheet();
    QString s = "QPushButton ";
    if ( toggle )
        s.append("{color: red;}");
    else
        s.append("{color: black;}");

    toggle = !toggle;
    stylesheet = s;
    ui->configPushButton->setStyleSheet(s);
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow::~MainWindow(): dtor entered";
#ifndef SKIP_SERIAL_PORT_INIT
   serial_comms_p->close_serial_port();
   delete serial_comms_p;
   delete ui;
#endif
   disconnect(c_invoke_config_dialog);
   delete db;
   delete tabbedDialog;
}

void MainWindow::serial_port_detected(QString &s) {

    qDebug() << "MainWindow::serial_port_detected()" << s;
}


void MainWindow::on_plainTextEdit_textChanged()
{
    // qDebug() << "MainWindow::on_plainTextEdit_textChanged() entered" << textCursor.position();
    QTextCursor cursor = ui->plainTextEdit->textCursor();
    int position = cursor.position()-1;
    QChar character = ui->plainTextEdit->document()->characterAt(position);
    char c = character.toLatin1();
    if ( c == 'T' ) {
        serial_comms_p->sendEcho = true;
        serial_comms_p->doEchoTest();
        return;
    }
    if ( c == 'V' ) {
        serial_comms_p->readVCC();
        return;

    }

    // If fall through - send character
    qDebug() << "Character at current cursor position is" << c << "Unicode:" << character.unicode();
    std::printf("Character is %02x", c);
    if ( c == '\r' || c == '\n' ) {
        qDebug() << "MainWindow::on_plainTextEdit_textChanged(): Discarding CR or LF";
        return;
    }
    c = toupper(c);
    serial_comms_p->add_byte(c);
    serial_comms_p->write_serial_data();
}

void MainWindow::configPushButton_clicked()
{
    qDebug() << "MainWindow::configPushButton_clicked(): entered";
    Ui::stationDialog sd_ui;
    QDialog d;

    sd_ui.setupUi(&d);
    // Set the "OK" button label to "Save"
    sd_ui.saveButtonBox->button(QDialogButtonBox::Ok)->setText("Save");

// #error "Check database, if table exists, pre-populate fields before showing dialog"

    // For testing - insert dummy values into dialog box
    // set_dummy_data(sd_ui);
    get_local_station_data(sd_ui);

    int dialog_code = d.exec();

    // User clicked Save - store data into local database
    if ( dialog_code == QDialog::Accepted ) {
        qDebug() << "MainWindow::configPushButton_clicked(): Storing values";
        // Store all values in the local database
        db->set_station_data_table_value_by_key("callsign", sd_ui.callSignLineEdit->text());
        db->set_station_data_table_value_by_key("opname", sd_ui.nameLineEdit->text());
        db->set_station_data_table_value_by_key("gridsquare", sd_ui.gridSquareLineEdit->text());
        db->set_station_data_table_value_by_key("city", sd_ui.cityLineEdit->text());
        db->set_station_data_table_value_by_key("state", sd_ui.stateLineEdit->text());
        db->set_station_data_table_value_by_key("county", sd_ui.countyLineEdit->text());
        db->set_station_data_table_value_by_key("country", sd_ui.countryLineEdit->text());
        db->set_station_data_table_value_by_key("section", sd_ui.sectionLineEdit->text());
        db->set_station_data_table_value_by_key("serialport", sd_ui.serialPortComboBox->currentText());

        db->syncStationData_write();
    }
    else {
        qDebug() << "MainWindow::configPushButton_clicked(): Aborting";
    }

    qDebug() << "MainWindow::configPushButton_clicked(): exiting";

    // Don't forget to disconnect signals before station dialog is destroyed
    // disconnect(sd_ui.callSignPlainTextEdit, &QPlainTextEdit::textChanged, this, &MainWindow::on_callSignPlainTextEdit_TextChanged);
}

void MainWindow::on_exitPushButton_clicked()
{
    // Exit application
    QApplication::quit();
}

void MainWindow::set_dummy_data(Ui::stationDialog sd_ui) {
    sd_ui.callSignLineEdit->setText("K1AY");
    sd_ui.nameLineEdit->setText("Chris");
    sd_ui.gridSquareLineEdit->setText("EL96av");
    sd_ui.cityLineEdit->setText("Punta Gorda");
    sd_ui.stateLineEdit->setText("FL");
    sd_ui.countyLineEdit->setText("Charlotte");
    sd_ui.countryLineEdit->setText("USA");
    sd_ui.sectionLineEdit->setText("WCF");
    sd_ui.serialPortComboBox->insertItem(0, "/dev/cu.usbserial-xxxyyyzzz");

}

bool MainWindow::get_local_station_data(Ui::stationDialog sd_ui) {

    QList<QString> keys = db->get_station_data_table_keys();
    db->dump_local_station_data();

    QList<QString>::iterator e;
    for (e = keys.begin(); e != keys.end(); ++e) {

        if ( *e == QString("callsign") )
            sd_ui.callSignLineEdit->setText(db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("opname") )
            sd_ui.nameLineEdit->setText(db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("gridsquare") )
            sd_ui.gridSquareLineEdit->setText(db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("city") )
            sd_ui.cityLineEdit->setText(db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("state") )
            sd_ui.stateLineEdit->setText(db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("county") )
            sd_ui.countyLineEdit->setText(db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("country") )
            sd_ui.countryLineEdit->setText(db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("section") )
            sd_ui.sectionLineEdit->setText(db->get_stataion_data_table_value_by_key(*e));
        else if ( *e == QString("serialport") ) {
            QString stmp = db->get_stataion_data_table_value_by_key(*e);
            if ( !stmp.isEmpty() ) {
                qDebug() << "MainWindow::get_local_station_data(): *e is NOT EMPTY ********" << *e;
                sd_ui.serialPortComboBox->insertItem(0, db->get_stataion_data_table_value_by_key(*e));
            }
        }
        else {
            qDebug() << "MainWindow::get_local_station_data(): Invalid station table key:" << *e;
            return false;
        }
    }
    populateSerialPortComboBox(sd_ui);

    return true;
}

void MainWindow::populateSerialPortComboBox(Ui::stationDialog sd_ui) {

    // Populate the ComboBox with all available serial ports - current was set above
    int index = sd_ui.serialPortComboBox->currentIndex();
    qDebug() << "MainWindow::populateSerialPortComboBox(): comboBox index:" << index;

    QString serialport = db->get_stataion_data_table_value_by_key("serialport");
    if ( serialport.isEmpty() ) {
        qDebug() << "MainWindow::populateSerialPortComboBox(): serialport field empty, setting placeholder text";
        sd_ui.serialPortComboBox->setPlaceholderText("Select Serial Port");
    }
    else
        sd_ui.serialPortComboBox->setPlaceholderText(serialport);

    QList<QSerialPortInfo>::iterator i;
    for (i = db->serial_port_list.begin(); i != db->serial_port_list.end(); ++i) {
        qDebug() << "MainWindow::populateSerialPortComboBox(): i->portName():" << i->portName();
        if ( i->portName().contains("cu.") )
            sd_ui.serialPortComboBox->addItem(i->portName());
    }
}

void MainWindow::showEvent(QShowEvent *event) {

    qDebug() << "MainWindow::showEvent(): Override Entered";
    if ( init_called_once == false ) {
        init_called_once = true;
        Q_EMIT waitVisibleSignal();
    }
}

bool MainWindow::event(QEvent* ev) {

#if 0
    if (ev->type() == QEvent::UpdateRequest )
        // This is a dirty hack - need to find a better way call init after mainwindow
        //   becomes visible for the first time
        if ( init_called_once == false ) {
            init_called_once = true;
            if ( !initialize_mainwindow() )
                QCoreApplication::exit(-3);
        }
#endif

    if ( ev->type() == QEvent::Show ) {
        qDebug() << "MainWindow::event(): for Event Qevent::Show";
    }

    return QWidget::event(ev);
}
