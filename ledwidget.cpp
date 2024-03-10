#include "ledwidget.h"

LedWidget::LedWidget(QWidget *parent)
     : QWidget{parent}
{
    setFixedSize(50, 40);
    ledColor = Qt::red;
}

void LedWidget::paintEvent(QPaintEvent *event) {

    Q_UNUSED(event);
    QPainter painter(this);
    painter.setBrush(ledColor);

    // This will draw an object at x,y with respect to the geometry of the enclosing Widget, not the main window
    painter.drawEllipse(0, 0, 15, 15);

}

void LedWidget::setText(const QString &) {
    // Dummy function to satisfy the "Promote" logic in Qt Designer ui form
}

void LedWidget::setLedColor(QColor color) {
    ledColor = color;
}
