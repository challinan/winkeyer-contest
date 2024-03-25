#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;   // This calls the MainWindow constructor

    if ( !w.initSucceeded() )   // This checks the initialization_succeeded flag
        return(-1);
    w.show();

    return a.exec();
}
