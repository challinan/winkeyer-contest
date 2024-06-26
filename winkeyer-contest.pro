QT       += core widgets serialport sql gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS += -O0

SOURCES += \
    callsign_lookup.cpp \
    contest_configuration.cpp \
    contest_type.cpp \
    country_file_parser.cpp \
    database-tables-base.cpp \
    ledwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    serialcomms.cpp \
    sqlite3-connector.cpp \
    transmitwindow.cpp \
    county_list.cpp \
    tabbed-config-dialog.cpp

HEADERS += \
    callsign_lookup.h \
    contest_configuration.h \
    contest_type.h \
    country_file_parser.h \
    database-tables-base.h \
    ledwidget.h \
    mainwindow.h \
    serialcomms.h \
    sqlite3-connector.h \
    transmitwindow.h \
    morse_table.h \
    county_list.h \
    tabbed-config-dialog.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    notes.txt
