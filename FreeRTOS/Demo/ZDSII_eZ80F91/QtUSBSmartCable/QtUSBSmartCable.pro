#-------------------------------------------------
#
# Project created by QtCreator 2016-07-22T17:35:56
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtUSBSmartCable
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    smartcableserver.cpp

HEADERS  += mainwindow.h \
    smartcableserver.h

FORMS    += mainwindow.ui

unix: LIBS += -lusb-1.0

RESOURCES += \
    resources.qrc
