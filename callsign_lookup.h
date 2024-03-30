#ifndef CALLSIGNLOOKUP_H
#define CALLSIGNLOOKUP_H

#include <QObject>
#include <QGroupBox>
#include <QTextEdit>
#include <QRect>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDebug>

class CallSignLookup : public QGroupBox
{
    Q_OBJECT
public:
    explicit CallSignLookup(const QString &title, QGroupBox *parent = nullptr);
    ~CallSignLookup();

    QLineEdit *call_sign_line_edit_p;
    QLabel *call_sign_label_p;

    QTextEdit *country_text_edit_p;
    QLabel *country_label_p;
    QTextEdit *prefix_text_edit_p;
    QLabel *prefix_text_edit_label_p;
    QLineEdit *unique_line_edit_p;
    QLabel *unique_line_edit_label_p;

private:
    QRect r {1,1,1,1};

signals:
};

#endif // CALLSIGNLOOKUP_H
