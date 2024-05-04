#include <QPainter>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ledwidget.h"

#include <QPointF>

// #define DBCONFIG_DEBUG  // When enabled, erases the db file on every run

// The purpose of this function is to allow the main window to become visible
//   before anything else.  For example, if there is no database, the warning
//   message is displayed before the main window.  Confusing and not good.  I
//   want the MainWindow to appear before any warning dialogs.
//
//   Don't know if there is a better way

// Important note: this function will only be called once show() has been called
void MainWindow::waitForVisible() {

    int count = 0;
    while ( isVisible() == false ) {
        count++;
    }
    qDebug() << "MainWindow::waitForVisible(void): count =" << count;
    initialization_succeeded = initialize_mainwindow();
    if ( !initialization_succeeded ) {
        qDebug() << "MainWindow::waitForVisible(): Exiting app:" << count;
        QApplication::exit(-3);
    }
    ui->virtualStatusLed->update();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    // Entry to main event loop starts when this constructor returns
    initialization_succeeded = true;    // If false, the application will quit before showing main window
    init_called_once = false;           // This helps implement our window becoming visible early
    speed_timer_active = false;
    allow_screen_moves = false;
    esm_mode = false;

    serial_comms_p = nullptr;
    db = nullptr;
    pTabbedDialogPtr = nullptr;
    pContestConfiguration = nullptr;

    // qDebug() << "MainWindow::MainWindow(): Ctor Entered";
    ui->setupUi(this);
    connect(this, &MainWindow::waitVisibleSignal, this, &MainWindow::waitForVisible, Qt::QueuedConnection);
    connect(ui->configPushButton, &QPushButton::clicked, this, &MainWindow::launchConfigDialog, Qt::QueuedConnection);
    // ui->CwTx_TextEdit->setPlaceholderText("This one will be replaced");
    ui->callSignLineEdit->installEventFilter(this);
    ui->callSignLineEdit->setMouseTracking(false);
    call_sign_box_pos = ui->callSignLineEdit->geometry();

    // Set input mask for callSignLineEdit
    //  ui->callSignLineEdit->setInputMask(">AAAAAA;");

    // Connect signal from call sign input QLineEdit to signal TextEdited
    // If we connect this signal automatically, or early, the setInputMask fires the signal and we crash
    connect(ui->callSignLineEdit, &QLineEdit::textEdited, this, &MainWindow::callSignLineEdit_textChanged, Qt::QueuedConnection);

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

    if ( !db->initContinue() )
        return false;

    // Read and parse the country data file
    pCountryFileParser = new CountryFileParser(this);
    connect(this, &MainWindow::callsignTextEditEmpty, pCountryFileParser, &CountryFileParser::callsignEditBoxReportEmpty);

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

    // Initialize contest configuration and Populate the contest dropdown with the contest names
    pContestConfiguration = new ContestConfiguration(this, db);
    connect(pContestConfiguration, &ContestConfiguration::esmStateChanged, this, &MainWindow::on_esmCheckBox_stateChanged);


    // Get the current contest name from the database
    QString configured_contest_name = db->getContestName();

    QListIterator<struct cabrillo_contest_names_t> m(pContestConfiguration->cabrillo_contest_data);
    while ( m.hasNext() ) {
        struct cabrillo_contest_names_t ccn = m.next();
        QString contest_name = ccn.cabrillo_name;
        ui->contestComboBox->addItem(contest_name);

        if ( configured_contest_name == ccn.cabrillo_name )
            ui->contest_name_label->setText( ccn.contest_full_name );
    }

    if ( !configured_contest_name.isEmpty() ) {
        ui->contestComboBox->setCurrentText(configured_contest_name);
        pContestConfiguration->setConfigContestName(configured_contest_name);
    }

    // Initialize ESM Mode indicator
    ui->esmModeLabel->setText("OFF");

    createFunctionKeys(pContestConfiguration->getRunMode());

    switch (pContestConfiguration->getRunMode()) {

        case RUN_MODE:
            ui->runRadioButton->setChecked(true);
            ui->snpRadioButton->setChecked(false);
            break;
        case SNP_MODE:
            ui->runRadioButton->setChecked(false);
            ui->snpRadioButton->setChecked(true);
            break;
        case UNKNOWN_MODE:
            break;
        }

    // CW Transmit window is now being created in the main ui form
    // Connect signals/slots
    // TODO - disconnect in destructor
    connect(ui->cwTextEdit->tx_thread_p, &CWTX_Thread::sendTxChar, serial_comms_p, &SerialComms::processTxChar, Qt::QueuedConnection);
    connect(serial_comms_p, &SerialComms::TxCharComplete,  ui->cwTextEdit->tx_thread_p, &CWTX_Thread::serialPortTxCharComplete, Qt::DirectConnection);
    connect(ui->cwTextEdit, &TransmitWindow::notifyMainWindowCR, this, &MainWindow::processKeyEventFromTxWindow);

    callsign_window_p = new CallSignLookup("Callsign Lookup");
    callsign_window_p->setAlignment(Qt::AlignHCenter);
    pCountryFileParser->setCallsignWindowP(callsign_window_p);

    callsign_window_p->show();

    QRect main_win_rect = this->geometry();
    QRect callsign_win_rect = callsign_window_p->geometry();
    callsign_window_p->move (main_win_rect.x()+900,callsign_win_rect.y());

    callsign_win_rect = callsign_window_p->geometry();
    qDebug() << "Callsign Window Geometry:" << callsign_win_rect << "pos:" << callsign_window_p->pos() << "Main Rect:" << main_win_rect;


    // TODO How, where and when do I kill this timer?
    // blinkTimer = new QTimer(this);
    // connect(blinkTimer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::changeConfigButtonTextColor));
    // blinkTimer->start(350);

    // Main Menu handler
    connect(ui->menubar, &QMenuBar::triggered, this, &MainWindow::menubarTriggered);
    ui->actionCallsign_Lookup->setChecked(true);

    return true;
}

void MainWindow::setupContextEditBoxes() {

}

void MainWindow::launchConfigDialog() {

    // Bring up our tabbed config dialog box
    qDebug() << "MainWindow::launchConfigDialog(): Entered";
    pTabbedDialogPtr = new TopLevelTabContainerDialog(db);

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
    delete pContestConfiguration;

    delete serial_comms_p;

    // Delete the function key buttons

    QListIterator<QPushButton *> i(function_key_buttons);
    while ( i.hasNext() ) {
        QPushButton *p = i.next();
        // qDebug() << "MainWindow::~MainWindow(): deleting:" << p->objectName();
        delete p;
    }

    delete ui;
    disconnect(c_invoke_config_dialog);
    delete pCountryFileParser;
    delete db;
}

void MainWindow::serial_port_detected(QString &s) {

    qDebug() << "MainWindow::serial_port_detected()" << s;
}

void MainWindow::on_exitPushButton_clicked()
{
    // Exit application
    QApplication::quit();
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
}

bool MainWindow::eventFilter(QObject *sender, QEvent *event)
{
    // This piece of code allows me to move the edit box where I want it with the mouse
    if (sender == ui->callSignLineEdit) {
        if(event->type()== QEvent::MouseMove && allow_screen_moves ) {
            // width = 141, height = 31
            QMouseEvent *m_event = (QMouseEvent*)(event);
            QPointF mouse_pos = m_event->position();
            call_sign_box_pos.setX(call_sign_box_pos.x()+mouse_pos.x());
            call_sign_box_pos.setY(call_sign_box_pos.y()+mouse_pos.y());
            call_sign_box_pos.setWidth(141);
            call_sign_box_pos.setHeight(31);
            ui->callSignLineEdit->setGeometry(call_sign_box_pos);
            ui->callSignLineEdit->show();
            qDebug() << "MainWindow::eventFilter(): position:" << m_event->position() << "call_sign_box_pos:" << call_sign_box_pos;
        }
        goto event_filter_done;
    }

    // This code intercepts the CR key for ESM mode (Enter Sends Message)
    if ( sender == this ) {
        QKeyEvent *p = (QKeyEvent *)event;
        qDebug() << "MainWindow::eventFilter(): This should be CR key:" << static_cast<char>(p->key());
    }

event_filter_done:
    return QWidget::eventFilter(sender, event);
}

void MainWindow::on_moveCheckBox_stateChanged(int checked)
{
    allow_screen_moves = checked == Qt::Checked ? true : false;
}


void MainWindow::on_esmCheckBox_stateChanged(int checked)
{
    esm_mode = checked == Qt::Checked ? true : false;

    if ( esm_mode ) {
        ui->esmModeLabel->setStyleSheet("QLabel { color : green; }");
        ui->esmModeLabel->setText("ON");

        // Configure the Function Keys to reflect ESM mode (highlighting)
        if ( pContestConfiguration->getRunMode() == RUN_MODE )
            on_runRadioButton_toggled(true);
        if ( pContestConfiguration->getRunMode() == SNP_MODE )
            on_snpRadioButton_toggled(true);
    }
    else {
        ui->esmModeLabel->setStyleSheet("QLabel { color : black; }");
        ui->esmModeLabel->setText("OFF");

        // Configure the Function Keys to reflect Normal mode (turn off highlighting)
        disableEsmHighlighting();
    }

}

void MainWindow::on_contestComboBox_activated(int index)
{
    Q_UNUSED(index);
    QString str = ui->contestComboBox->currentText();

    // Configure the main window for the contest chosen by the operator
    qDebug() << "MainWindow::on_contestComboBox_activated():" << str;
    pContestConfiguration->setCurrentContest(str);

    // Set the display string
    QListIterator<struct cabrillo_contest_names_t> m(pContestConfiguration->cabrillo_contest_data);
    while ( m.hasNext() ) {
        struct cabrillo_contest_names_t ccn = m.next();
        if ( str == ccn.cabrillo_name ) {
            ui->contest_name_label->setText( ccn.contest_full_name );
            break;
        }
    }

}

void MainWindow::createFunctionKeys(run_state_e run_mode) {

#define SPACE_BETWEEN_CELLS 10
#define FKEYS_PER_ROW 6

    QList<struct func_key_t> &pFKeys = pContestConfiguration->cwFuncKeyDefs;
    QList<struct func_key_t> modeList;      // Get only the labels, etc for our current mode

    QListIterator<struct func_key_t> m(pFKeys);
    while ( m.hasNext() ) {
        struct func_key_t t = m.next();
        if ( t.run_state == run_mode )
            modeList.append(t);
    }

    // Assumption is there are 12 function keys, 6 on each row, like n1mm
    QRect r1 = ui->callSignLineEdit->geometry();
    QRect mainWinGeometery = this->geometry();
    int total_window_width = mainWinGeometery.width();
    int available_width = total_window_width - (SPACE_BETWEEN_CELLS * (FKEYS_PER_ROW + 1)); // Seven spaces for 6 buttons

    // Make room for 6 boxes across the width
    int button_width = available_width / FKEYS_PER_ROW;
    int left_edge_reference = r1.x();

    // TODO this is duplicate code - see next for loop
    // Place first row of buttons
    int i;
    for ( i=0; i<6; i++) {
        QPushButton *p = new QPushButton(this);
        // Make sure 'i' is a valid index
        if ( i < 0 || i >= modeList.size() ) {
            delete p;
            return;
        }
        p->setObjectName(modeList.at(i).functionKey);

        // Calcaulate where it will be place in our main window
        QRect r;
        r.setX( (left_edge_reference  + ((SPACE_BETWEEN_CELLS + button_width) * i)) );
        r.setY(r1.y() + 70);
        r.setWidth(button_width);
        r.setHeight(r1.height());
        p->setGeometry(r);

        QString fkey_label = modeList.at(i).functionKey;
        QString label_text = modeList.at(i).label;

        fkey_label.append(" " + label_text);
        // qDebug () << "Label:" << label_text;
        p->setText(fkey_label);
        // p->show();
        function_key_buttons.append(p);
    }

    // Place second row of buttons
    for ( ; i<12; i++) {
        QPushButton *p = new QPushButton(this);
        p->setObjectName(modeList.at(i).functionKey);

        // Calcaulate where it will be place in our main window
        QRect r;
        r.setX( (left_edge_reference  + ((SPACE_BETWEEN_CELLS + button_width) * (i-6))) );
        r.setY(r1.y() + 100);
        r.setWidth(button_width);
        r.setHeight(r1.height());
        p->setGeometry(r);

        QString button_label = modeList.at(i).functionKey;
        button_label.append(" " + modeList.at(i).label);
        // qDebug () << "Label:" << modeList.at(i).label;
        p->setText(button_label);
        p->show();
        function_key_buttons.append(p);
    }
}

void MainWindow::disableEsmHighlighting() {

    QPushButton *p = findChild<QPushButton *>("F4");
    p->setStyleSheet("");
    p = findChild<QPushButton *>("F1");
    p->setStyleSheet("");

}

void MainWindow::on_runRadioButton_toggled(bool checked) {

    if ( checked ) {
        // Configure UI for Run Mode
        pContestConfiguration->setRunMode(RUN_MODE);
        ui->snpRadioButton->setChecked(false);
        resetFunctionButtonLabels(RUN_MODE);

        // If ESM mode is enabled, highlight the function that will be send on Enter
        if ( esm_mode ) {
            QPushButton *p = findChild<QPushButton *>("F4");
            p->setStyleSheet("");
            p = findChild<QPushButton *>("F1");
            p->setStyleSheet("QPushButton { background-color: yellow }");
        }
    }
}


void MainWindow::on_snpRadioButton_toggled(bool checked) {

    if ( checked ) {
        // Configure the UI for Search & Pounce mode
        pContestConfiguration->setRunMode(SNP_MODE);
        ui->runRadioButton->setChecked(false);
        resetFunctionButtonLabels(SNP_MODE);

        // If ESM mode is enabled, highlight the function that will be send on Enter
        if ( esm_mode ) {
            QPushButton *p = findChild<QPushButton *>("F1");
            p->setStyleSheet("");
            p = findChild<QPushButton *>("F4");
            p->setStyleSheet("QPushButton { background-color: yellow }");
        }
    }
}

void MainWindow::resetFunctionButtonLabels(run_state_e run_mode) {

    // Set the appropriate button label for the chosen operating mode, RUN or S&P
    QList<struct func_key_t> &pFKeys = pContestConfiguration->cwFuncKeyDefs;
    QList<struct func_key_t> modeList;      // Get only the labels, etc for our current mode

    QListIterator<struct func_key_t> m(pFKeys);
    while ( m.hasNext() ) {
        struct func_key_t t = m.next();
        if ( t.run_state == run_mode )
            modeList.append(t);
    }

    for ( int i=0; i<12; i++) {
        // Validate modeList index
        if ( i < 0 || i >= modeList.size() )
            return;
        QString button_label = modeList.at(i).functionKey;
        button_label.append(" " + modeList.at(i).label);
        function_key_buttons.at(i)->setText(button_label);
        function_key_buttons.at(i)->show();
    }
}

void MainWindow::processKeyEventFromTxWindow(int key) {

    if ( key == Qt::Key_Return ) {
        if (esm_mode )
        qDebug() << "MainWindow::processKeyEventFromTxWindow(): key:" << "CR";
        // Transmit highlighted function key!

    }
}

void MainWindow::callSignLineEdit_textChanged(const QString &arg1)
{
    if ( arg1.isEmpty() )
        emit callsignTextEditEmpty();
    QString s = arg1.toUpper();
    ui->callSignLineEdit->setText(s);
    // qDebug() << "MainWindow::on_callSignLineEdit_textChanged(): Entered - arg1" << s;
    // Debug Code
    if ( arg1 == "=" ) s = "A8O";
    pCountryFileParser->lookupPartial(s);
}

void MainWindow::menubarTriggered(QAction *a) {
    if ( a->text() == "Callsign Lookup") {
        if ( callsign_window_p->isVisible() )
            callsign_window_p->hide();
        else {
            callsign_window_p->show();
        }
    }
}
