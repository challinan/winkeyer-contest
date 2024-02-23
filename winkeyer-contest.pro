QT       += core widgets serialport sql gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS += -O0

SOURCES += \
    database-tables-base.cpp \
    main.cpp \
    mainwindow.cpp \
    serialcomms.cpp \
    sqlite3-connector.cpp \
    tabbed-config-dialog.cpp

HEADERS += \
    database-tables-base.h \
    mainwindow.h \
    serialcomms.h \
    sqlite3-connector.h \
    tabbed-config-dialog.h

FORMS += \
    mainwindow.ui \
    station_data.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    notes.txt
