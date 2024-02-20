#include "mainwindow.h"
#include "ui_mainwindow.h"

#define SKIP_SERIAL_PORT_INIT
// #define DBCONFIG_DEBUG  // When enabled, erases the db file on every run

void MainWindow::waitForVisible() {
    int count = 0;
    while ( isVisible() == false ) {
        count++;
    }
    // qDebug() << "MainWindow::waitForVisible(void): count =" << count;
    if ( !initialize_mainwindow() ) {
        qDebug() << "MainWindow::waitForVisible(): Exiting app:" << count;
        QCoreApplication::exit(-3);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    initialization_succeeded = true;
    init_called_once = false;
    qDebug() << "MainWindow::MainWindow(): Ctor Entered";
    ui->setupUi(this);
    connect(this, &MainWindow::waitVisibleSignal, this, &MainWindow::waitForVisible, Qt::QueuedConnection);
    connect(ui->configPushButton, &QPushButton::clicked, this, &MainWindow::launchConfigDialog, Qt::QueuedConnection);

    // Entry to main event loop starts when this constructor returns
    qDebug() << "MainWindow::MainWindow(): Ctor Exiting complete";
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

    // Initialize and sync internal and external station database
    db = new Sqlite3_connector;

    c_invoke_config_dialog =
    connect(db, &Sqlite3_connector::do_config_dialog, this, &MainWindow::launchConfigDialog, Qt::QueuedConnection);

    qDebug() << "MainWindow::initialize_mainwindow(): Called connect: c_invoke_config_dialog = " << c_invoke_config_dialog;

    db->initContinue();

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
    pTabbedDialogPtr = new TopLevelTabContainerDialog(db);

    // get_local_station_data_into_dialog(tabbedDialogPtr);
    pTabbedDialogPtr->show();
    pTabbedDialogPtr->exec();

    // Store the data here
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
#ifndef SKIP_SERIAL_PORT_INIT
   serial_comms_p->close_serial_port();
   delete serial_comms_p;
   delete ui;
#endif
   disconnect(c_invoke_config_dialog);
   delete db;
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
