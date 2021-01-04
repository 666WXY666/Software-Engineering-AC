QT       += core gui network sql charts
RC_ICONS = server.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dao.cpp \
    log.cpp \
    main.cpp \
    managerwidget.cpp \
    monitorwidget.cpp \
    receptionwidget.cpp \
    report.cpp \
    room.cpp \
    scheduler.cpp \
    server.cpp \
    serverqueue.cpp \
    serverwidget.cpp \
    setwidget.cpp \
    statistic.cpp \
    waitqueue.cpp

HEADERS += \
    dao.h \
    log.h \
    managerwidget.h \
    monitorwidget.h \
    receptionwidget.h \
    report.h \
    room.h \
    scheduler.h \
    server.h \
    serverqueue.h \
    serverwidget.h \
    setwidget.h \
    statistic.h \
    waitqueue.h

FORMS += \
    managerwidget.ui \
    monitorwidget.ui \
    receptionwidget.ui \
    serverwidget.ui \
    setwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
