#ifndef LEDWIDGET_H
#define LEDWIDGET_H

#include <QObject>
#include <QLabel>
#include <QWidget>
#include <QPainter>
#include <QColor>


class LedWidget : public QWidget
{
    friend class MainWindow;

    Q_OBJECT
public:
    explicit LedWidget(QWidget *parent = nullptr);

public slots:
    void setText(const QString &);  // Fool mainwindow.ui
    void setLedColor(QColor color);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor ledColor;

signals:

};

#endif // LEDWIDGET_H
