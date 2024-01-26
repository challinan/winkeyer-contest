#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    bool rcb;

    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    // QDir::homePath() gives users $HOME
    rcb = QDir::setCurrent("/Users/chris/sandbox");
    if ( rcb == false ) a.exit(1);

    return a.exec();
}
