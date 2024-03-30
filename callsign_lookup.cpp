#include "callsign_lookup.h"

#include <QDebug>

CallSignLookup::CallSignLookup(const QString &title, QGroupBox *parent)
    : QGroupBox{title, parent}
{
    qDebug() << "CallSignLookup::CallSignLookup(): Constructor called";
    r = this->geometry();
    // r.setWidth(r.width()+75);
    // r.setX(r.x());
    // this->setGeometry(r);
    qDebug() << "CallSignLookup::CallSignLookup(): r =:" << r;

    prefix_text_edit_p = new QTextEdit(this);
    country_text_edit_p = new QTextEdit(this);

    country_label_p = new QLabel("Country");
    country_label_p->setObjectName("country_label");

    call_sign_line_edit_p = new (QLineEdit);
    call_sign_line_edit_p->setObjectName("callsign_line_edit");

    call_sign_label_p = new QLabel("callsign_label");
    call_sign_label_p->setText("Callsign");

    prefix_text_edit_label_p = new QLabel("Prefix");

    unique_line_edit_p = new QLineEdit;
    unique_line_edit_label_p = new QLabel("Unique");

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(call_sign_label_p);
    vbox->addWidget(call_sign_line_edit_p);
    vbox->addWidget(country_label_p);
    vbox->addWidget(country_text_edit_p);
    vbox->addWidget(prefix_text_edit_label_p);
    vbox->addWidget(prefix_text_edit_p);
    vbox->addWidget(unique_line_edit_label_p);
    vbox->addWidget(unique_line_edit_p);
    vbox->addStretch(10);
    this->setLayout(vbox);

}

CallSignLookup::~CallSignLookup() {

    delete country_text_edit_p;
    delete country_label_p;
    delete call_sign_line_edit_p;
    delete prefix_text_edit_p;
}
