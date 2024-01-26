#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include "serialcomms.h"
#include "ui_mainwindow.h"
#include "sqlite3-connector.h"

#include <QTextCursor>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
namespace Ui { class stationDialog; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QTextCursor textCursor;

private:
    void set_dummy_data(Ui::stationDialog sd_ui);

private:
    Ui::MainWindow *ui;
    SerialComms *serial_comms_p;
    Sqlite3_connector *db;

public slots:
    void serial_port_detected(QString &s);

private slots:
    void on_plainTextEdit_textChanged();
    void on_configPushButton_clicked();
    void on_exitPushButton_clicked();
};

#endif // MAINWINDOW_H
