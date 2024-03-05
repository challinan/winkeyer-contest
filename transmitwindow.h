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

#define CBUFF_SIZE 8

class CWTX_Thread;
class MainWindow;

struct transmitChar {
    char c; // The character typed and reported in keyPressEvent
    int position;   // Returns the absolute position of the cursor within the document
    bool complete;
};

class CBuffer {
public:
    CBuffer();
    bool put(char c);
    char get();
    void clear();
    bool isEmpty();
    bool isFull();
    bool deleteLast();
    int getNumCharsQueued();

private:
    char cbuff[CBUFF_SIZE];
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
    explicit TransmitWindow(MainWindow *parent = nullptr);
    ~TransmitWindow();

private:
    void txReset();

public slots:
    void processTextChanged();
    void markCharAsSent();
    void CursorPositionChangedSlot();

protected:
     void keyPressEvent(QKeyEvent *event) override;
     void mousePressEvent(QMouseEvent *event) override;
     void keyReleaseEvent(QKeyEvent *event) override;

 public:
     CWTX_Thread *tx_thread_p;
     struct transmitChar txChar;

private:
     int tx_position;
     char key_count;
     int last_size;
     bool is_transmitting;
     QTextCursor cursor;
     QTextCharFormat strikethrough_f;
     QTextCharFormat normal_f;

     struct {
         int position;  // Transmit character position
         int block;  // Transmit block
     } tpos;

     CBuffer ccbuf;

     // For debug only
     int key_release_count;
     int key_down_count;
     QMutex highlightTextMutex;

signals:
     void startTx();

};

// *********   QThread Class *****************

class CWTX_Thread : public QThread
{
    Q_OBJECT;

public:
    CWTX_Thread(TransmitWindow *p);
    void run() override;
    CBuffer *cbuf_p;

private:
    int calculate_delay(char c);

private:
    bool transmitNow;
    bool paused;
    TransmitWindow *txwinObj_p;
    int dit_timing_factor;
    bool txAvailable;
    char currentTxChar;

public slots:
    void sendToSerialPortTx();
    void pauseTx(bool pause);
    void serialPortTxCharComplete();

signals:
    void deQueueChar();
    void sendTxChar(uchar c);

};

#endif // TRANSMITWINDOW_H
