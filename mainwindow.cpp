#include <QPainter>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ledwidget.h"

// #define SKIP_SERIAL_PORT_INIT
// #define DBCONFIG_DEBUG  // When enabled, erases the db file on every run

// The purpose of this function is to allow the main window to become visible
//   before anything else.  For example, if there is no database, the warning
//   message is displayed before the main window.  Confusing and not good.
//   Don't know if there is a better way
void MainWindow::waitForVisible() {
    int count = 0;
    while ( isVisible() == false ) {
        count++;
    }
    // qDebug() << "MainWindow::waitForVisible(void): count =" << count;
    if ( !initialize_mainwindow() ) {
        qDebug() << "MainWindow::waitForVisible(): Exiting app:" << count;
        QApplication::exit(-3);
    }
    ui->virtualStatusLed->update();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    initialization_succeeded = true;    // If false, the application will quit before showing main window
    init_called_once = false;           // This helps implement our window becoming visible early
    speed_timer_active = false;

    serial_comms_p = nullptr;

    // qDebug() << "MainWindow::MainWindow(): Ctor Entered";
    ui->setupUi(this);
    connect(this, &MainWindow::waitVisibleSignal, this, &MainWindow::waitForVisible, Qt::QueuedConnection);
    connect(ui->configPushButton, &QPushButton::clicked, this, &MainWindow::launchConfigDialog, Qt::QueuedConnection);
    // ui->CwTx_TextEdit->setPlaceholderText("This one will be replaced");

    // Entry to main event loop starts when this constructor returns
    QRect r;

    // QRect(0,0 852x425) - this was the default before setting it.
    // Place the window out of the way of my debug outout ;)
    r.setX(200);
    r.setY(0);
    r.setWidth(852);
    r.setHeight(425);
    this->setGeometry(r);

    ui->cwTextEdit->setFocus();

    ui->virtualStatusLed->update();
    qDebug() << "MainWindow::MainWindow(): Ctor Exiting complete" << r;
}

void MainWindow::setSerialStatusLedColor(QColor color) {
    qDebug() << "MainWindow::setSerialStatusLedColor(): color:" << color;
}

bool MainWindow::initSucceeded() {
    return initialization_succeeded;
}

bool MainWindow::initialize_mainwindow() {

#ifdef DBCONFIG_DEBUG
    // For debug only - delete db file for debug purposes
    QString dbFile = "/Users/chris/.macrr/winkeyer_db";
    QFile file (dbFile);
    file.remove();
    qDebug() << "MainWindow::initialize_mainwindow(): DATABASE FILE REMOVED";
#endif

    pCountyList = new CountyList;

    // Initialize and sync internal and external station database
    db = new Sqlite3_connector;

    c_invoke_config_dialog =
    connect(db, &Sqlite3_connector::do_config_dialog, this, &MainWindow::launchConfigDialog, Qt::QueuedConnection);

    qDebug() << "MainWindow::initialize_mainwindow(): Called connect(): returned =" << c_invoke_config_dialog;

    db->initContinue();

#ifndef SKIP_SERIAL_PORT_INIT
    // Initialize serial port object
    serial_comms_p = new SerialComms(this, db);

    db->setSerialPtr(serial_comms_p);

    if ( !serial_comms_p->openSerialPort() ) {
        qDebug() << "MainWindow::initialize_mainwindow(): Open Serial Port failed";
        ui->virtualStatusLed->setLedColor(Qt::red);
    } else {
        connect(serial_comms_p, &SerialComms::on_serial_port_detected, this, &MainWindow::serial_port_detected, Qt::QueuedConnection);
        ui->virtualStatusLed->setLedColor(Qt::green);
        ui->virtualStatusLed->update();

        // Set min/max ranges for our speed spinBox and Winkeyer Speed Pot
        ui->speedSpinBox->setMinimum(SPEEDPOT_MIN);
        ui->speedSpinBox->setMaximum(SPEEDPOT_MIN + SPEEDPOT_RANGE);
        serial_comms_p->setupSpeedPotRange(SPEEDPOT_MIN, SPEEDPOT_RANGE);

        // Set our initial keyer speed to 22 WPM
        ui->speedSpinBox->setValue(22);
        serial_comms_p->setSpeed(22);

        // Transmit window also needs to have an initial speed value for delay calculations
        ui->cwTextEdit->reportTxSpeed(22);

        connect(ui->speedSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::speedSpinBox_valueChanged);
    }
#endif

    // CW Transmit window is now being created in the main ui form
    // Connect signals/slots
    connect(ui->cwTextEdit->tx_thread_p, &CWTX_Thread::sendTxChar, serial_comms_p, &SerialComms::processTxChar, Qt::QueuedConnection);
    connect(serial_comms_p, &SerialComms::TxCharComplete,  ui->cwTextEdit->tx_thread_p, &CWTX_Thread::serialPortTxCharComplete, Qt::DirectConnection);

    // TODO How, where and when do I kill this timer?
    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::changeConfigButtonTextColor));
    blinkTimer->start(350);

    return true;
}

void MainWindow::launchConfigDialog() {

    // Bring up our tabbed config dialog box
    qDebug() << "MainWindow::launchConfigDialog(): Entered";
    pTabbedDialogPtr = new TopLevelTabContainerDialog(db);

    // get_local_station_data_into_dialog(tabbedDialogPtr);
    pTabbedDialogPtr->show();
    pTabbedDialogPtr->exec();

    // Store the data before here
    delete pTabbedDialogPtr;
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
    // delete pTxWindow;
#ifndef SKIP_SERIAL_PORT_INIT
   delete serial_comms_p;
#endif
   delete ui;
   disconnect(c_invoke_config_dialog);
   delete db;
}

void MainWindow::serial_port_detected(QString &s) {

    qDebug() << "MainWindow::serial_port_detected()" << s;
}

#if 0
void MainWindow::on_CwTx_TextEdit_textChanged()
{
    QTextCursor cursor = ui->CwTx_TextEdit->textCursor();
    // qDebug() << "MainWindow::on_CwTx_TextEdit_textChanged() entered" << cursor.position();

    int position = cursor.position()-1;
    QChar character = ui->CwTx_TextEdit->document()->characterAt(position);
    char c = character.toLatin1();

    if ( serial_comms_p == nullptr ) return;    // We don't have a valid serial comms object (mostly for debugging)

    if ( c == 'T' ) {
        serial_comms_p->doEchoTest();
        return;
    }
    if ( c == 'V' ) {
        serial_comms_p->readVCC();
        return;

    }

    // If fall through - send character
    c = toupper(c);
    serial_comms_p->add_byte(c);
    serial_comms_p->writeSerialData();
}
#endif

void MainWindow::on_exitPushButton_clicked()
{
    // Exit application
    QApplication::quit();
}

void MainWindow::set_dummy_station_data(Ui::stationDialog sd_ui) {
    sd_ui.callSignLineEdit->setText("K1AY");
    sd_ui.nameLineEdit->setText("Chris");
    sd_ui.gridSquareLineEdit->setText("EL96av");
    sd_ui.cityLineEdit->setText("Punta Gorda");
    sd_ui.stateLineEdit->setText("FL");
    sd_ui.countyLineEdit->setText("Charlotte");
    sd_ui.countryLineEdit->setText("USA");
    sd_ui.sectionLineEdit->setText("WCF");
}

void MainWindow::showEvent(QShowEvent *event) {

    Q_UNUSED(event);

    qDebug() << "MainWindow::showEvent(): Override Entered";
    if ( init_called_once == false ) {
        init_called_once = true;
        Q_EMIT waitVisibleSignal();
    }
}

void MainWindow::speedSpinBox_valueChanged(int arg1)
{
    if ( speed_timer_active == false ) {
        qDebug() << "MainWindow::speedSpinBox_valueChanged:" << arg1;
        speed_spinbox_timer = new QTimer(this);
        speed_spinbox_timer->setSingleShot(true);
        c_speed_timer = connect(speed_spinbox_timer, &QTimer::timeout, this, &MainWindow::UpdateSpeed);
        speed_timer_active = true;
        speed_spinbox_timer->start(750);
    } else  {
        speed_spinbox_timer->setInterval(750); // Bump timer
    }
}

void MainWindow::UpdateSpeed() {

    int speed = ui->speedSpinBox->value();
    speed_timer_active = false;
    disconnect(c_speed_timer);
    delete speed_spinbox_timer;

    qDebug() << "MainWindow::UpdateSpeed(): Timer timedout - slot entered - speed now" << speed;

    serial_comms_p->setSpeed(speed);
    pTxWindow->reportTxSpeed(speed);
}

void MainWindow::on_comboBox_currentTextChanged(const QString &arg1)
{
    qDebug() << "MainWindow::on_comboBox_currentTextChanged(): Entered:" << arg1;
}

