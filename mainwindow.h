#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QTextCursor>
#include <QPushButton>
#include <QAbstractButton>
#include <QEvent>
#include <QDebug>
#include <QWidget>
#include <QThread>
#include <QTimer>
#include <QMainWindow>
#include <QDir>
#include <QSerialPortInfo>

#include "serialcomms.h"
#include "ui_station_data.h"
#include "sqlite3-connector.h"
#include "tabbed-config-dialog.h"
#include "transmitwindow.h"
#include "county_list.h"
#include <contest_configuration.h>
#include "country_file_parser.h"
#include "callsign_lookup.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
namespace Ui { class stationDialog; }
QT_END_NAMESPACE

class TransmitWindow;

enum qso_state {
    UNK_STATE,              // State not defined
    RUNMODE_CQ_START,       // No QSO in progress.  Calling CQ
    RUNMODE_PREFIXVALID,   // User entered callsign prefix into Callsign TextEdit

};

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
    void setupContextEditBoxes();
    // void populateSerialPortComboBox(Ui::stationDialog sd_ui);
    void createFunctionKeys(state_e mode);
    void resetFunctionButtonLabels(state_e mode);
    void disableEsmHighlighting();

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
    CountyList *pCountyList;
    QRect call_sign_box_pos;
    bool allow_screen_moves;
    ContestConfiguration *pContestConfiguration;
    QList<QPushButton *> function_key_buttons;
    bool esm_mode;
    CountryFileParser *pCountryFileParser;
    CallSignLookup *callsign_window_p;


protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *sender, QEvent *event) override;

public slots:
    void serial_port_detected(QString &s);
    void processKeyEventFromTxWindow(int key);

private slots:
    void on_exitPushButton_clicked();
    void changeConfigButtonTextColor();
    void launchConfigDialog();
    void waitForVisible();
    void UpdateSpeed();

    void speedSpinBox_valueChanged(int arg1);
    void on_moveCheckBox_stateChanged(int arg1);
    void on_contestComboBox_activated(int index);
    void on_runRadioButton_toggled(bool checked);
    void on_snpRadioButton_toggled(bool checked);
    void on_esmCheckBox_stateChanged(int arg1);

    void callSignLineEdit_textChanged(const QString &arg1);

signals:
    void waitVisibleSignal();
    void callsignTextEditEmpty();
};

#endif // MAINWINDOW_H
