QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    databasemanager.cpp \
    keycoordinates.cpp \
    keylogger.cpp \
    main.cpp \
    mainwindow.cpp \
    screentaker.cpp

HEADERS += \
    databasemanager.h \
    keycoordinates.h \
    keylogger.h \
    mainwindow.h \
    screentaker.h

FORMS += \
    mainwindow.ui

win32:LIBS += -lgdi32 -luser32

LIBS += -lGdiplus

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
