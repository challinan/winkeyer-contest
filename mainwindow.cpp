#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_station_data.h"

#define DB_DEBUG

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    db = new Sqlite3_connector;
    db->initDatabase();
    // Q_ASSERT(false);


    // Initialize serial port object
#ifndef DB_DEBUG
    serial_comms_p = new SerialComms();
    db->setSerialPtr(serial_comms_p);
    serial_comms_p->openSerialPort();
    connect(serial_comms_p, &SerialComms::on_serial_port_detected, this, &MainWindow::serial_port_detected);

   textCursor = QTextCursor(ui->plainTextEdit->textCursor());
   ui->plainTextEdit->setFocusPolicy(Qt::StrongFocus);
#endif
}

MainWindow::~MainWindow()
{
#ifndef DB_DEBUG
   serial_comms_p->close_serial_port();
   delete serial_comms_p;
   delete ui;
#endif
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

void MainWindow::on_configPushButton_clicked()
{
    qDebug() << "MainWindow::on_configPushButton_clicked(): entered";
    Ui::stationDialog sd_ui;
    QDialog d;

    sd_ui.setupUi(&d);
    // Set the "OK" button label to "Save"
    sd_ui.saveButtonBox->button(QDialogButtonBox::Ok)->setText("Save");
    // sd_ui.serialPortComboBox->setPlaceholderText("Select Serial Port");

    // connect(sd_ui.callSignPlainTextEdit, &QPlainTextEdit::textChanged, this, &MainWindow::on_callSignPlainTextEdit_TextChanged);
// #error "Check database, if table exists, pre-populate fields before showing dialog"

    // For testing - insert test values into dialog box
    set_dummy_data(sd_ui);

    int dialog_code = d.exec();
    // "callsign, ", "opname, ", "gridsquare, ", "city, ", "state, ",
    //     "county, ", "country, ", "section, ", "serialport"}

    if ( dialog_code == QDialog::Accepted ) {
        qDebug() << "MainWindow::on_configPushButton_clicked(): Storing values";
        // Store all values in the database
        db->setCallSign(sd_ui.callSignLineEdit->text());
        db->setName(sd_ui.nameLineEdit->text());
        db->setGridSquare(sd_ui.gridSquareLineEdit->text());
        db->setCity(sd_ui.cityLineEdit->text());
        db->setState(sd_ui.stateLineEdit->text());
        db->setCounty(sd_ui.countyLineEdit->text());
        db->setCountry(sd_ui.countryLineEdit->text());
        db->setArrlSection(sd_ui.sectionLineEdit->text());
        db->setSerialPort(sd_ui.serialPortComboBox->currentText());

        db->syncStationData();
    }
    else {
        qDebug() << "MainWindow::on_configPushButton_clicked(): Aborting";
    }

    qDebug() << "MainWindow::on_configPushButton_clicked(): exiting";


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
