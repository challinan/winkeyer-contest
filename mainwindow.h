#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QSerialPortInfo>
#include "serialcomms.h"
#include "ui_station_data.h"
#include "sqlite3-connector.h"
#include "tabbed-config-dialog.h"
#include "transmitwindow.h"
#include "ledwidget.h"
#include "county_list.h"

#include <QTextCursor>
#include <QPushButton>
#include <QAbstractButton>
#include <QEvent>
#include <QDebug>
#include <QWidget>
#include <QThread>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
namespace Ui { class stationDialog; }
QT_END_NAMESPACE

class TransmitWindow;

#include <QWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool initialize_mainwindow();
    bool initSucceeded();
    void setSerialStatusLedColor(QColor color);

private:
    void set_dummy_station_data(Ui::stationDialog sd_ui);
    // void populateSerialPortComboBox(Ui::stationDialog sd_ui);

private:
    Ui::MainWindow *ui;
    SerialComms *serial_comms_p;
    Sqlite3_connector *db;
    QMetaObject::Connection c_invoke_config_dialog;
    QMetaObject::Connection c_speed_timer;
    bool initialization_succeeded;
    bool init_called_once;
    QTimer *blinkTimer;
    TopLevelTabContainerDialog *pTabbedDialogPtr;
    QTimer *speed_spinbox_timer;
    bool speed_timer_active;
    TransmitWindow *pTxWindow;
    CountyList *pCountyList;

protected:
    void showEvent(QShowEvent *event) override;

public slots:
    void serial_port_detected(QString &s);

private slots:
    void on_exitPushButton_clicked();
    void changeConfigButtonTextColor();
    void launchConfigDialog();
    void waitForVisible();
    void UpdateSpeed();

    void speedSpinBox_valueChanged(int arg1);

    void on_comboBox_currentTextChanged(const QString &arg1);

signals:
    void waitVisibleSignal();
};

#endif // MAINWINDOW_H
