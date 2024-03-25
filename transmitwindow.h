#ifndef TRANSMITWINDOW_H
#define TRANSMITWINDOW_H

#include <QObject>
#include <QApplication>
#include <QDebug>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QString>
#include <QThread>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QMutex>
#include <QDateTime>
#include <QTimer>
// #include "serialcomms.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define CBUFF_SIZE 32

class CWTX_Thread;
class MainWindow;

struct transmitChar {
    char c; // The character typed and reported in keyPressEvent
    int position;   // Returns the absolute position of the cursor within the document
    int block;
    bool complete;
};

class CBuffer {
public:
    CBuffer();
    ~CBuffer();
    bool put(char c);
    char get();
    void clear();
    bool isEmpty();
    bool isFull();
    bool setSize(int size);
    bool deleteLast();
    int getNumCharsQueued();

private:
    // char cbuff[CBUFF_SIZE];
    char *cbuff;

    // We write to "tail" and read from "head"
    int size;
    int head;
    int tail;
};

class TransmitWindow : public QTextEdit
{
    friend class CWTX_Thread;
    Q_OBJECT
public:
    TransmitWindow(QWidget *parent = nullptr);
    ~TransmitWindow();

private:
    void txReset();

public slots:
    void processTextChanged();
    void strikeoutCharAsSent();
    void reportTxSpeed(int speed);

protected:
     void keyPressEvent(QKeyEvent *event) override;
     void keyReleaseEvent(QKeyEvent *event) override;

 public:
     CWTX_Thread *tx_thread_p;
     struct transmitChar txCharStamp;

 private:
     int tx_position;
     int highlight_position;
     bool is_transmitting;
     QTextCursor cursor;
     QTextCharFormat strikethrough_f;
     QTextCharFormat normal_f;
     int dit_timing_factor;
     int txSpeed;

     CBuffer ccbuf;
     CBuffer txWindowBuffer;

     // For debug only
     int key_down_count;
     int key_release_count;
     int strikeout_count;

 signals:
     void startTx();
     void notifyMainWindowCR(int key);

};

// *********   CWTX_Thread Class *****************

class CWTX_Thread : public QThread
{
    Q_OBJECT;

public:
    CWTX_Thread(TransmitWindow *p);
    void run() override;
    CBuffer *cbuf_p;

private:
    int calculateDelay(char c);

private:
    bool transmitNow;
    bool paused;
    TransmitWindow *txwinObj_p;
    bool txAvailable;
    char currentTxChar;
    bool strikeoutTimerRunning;
    int ms_delay;
    QTimer *pStrikeoutTimer;
    QMetaObject::Connection c_strikeout_timerConnx;

public slots:
    void sendToSerialPortTx();
    void pauseTx(bool pause);
    void serialPortTxCharComplete();
    void strikeoutTimerTimeout();

signals:
    void deQueueChar();
    void sendTxChar(uchar c);

};

#endif // TRANSMITWINDOW_H
