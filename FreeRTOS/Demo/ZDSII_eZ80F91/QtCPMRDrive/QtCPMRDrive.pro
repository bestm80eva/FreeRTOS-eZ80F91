#-------------------------------------------------
#
# Project created by QtCreator 2016-07-23T00:45:48
#
#-------------------------------------------------

QT       += core gui network


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtCPMRDrive
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    cpmdrive.cpp \
    cpmclient.cpp \
    clientgui.cpp \
    cdrivegui.cpp

HEADERS  += mainwindow.h \
    cpmrdsk.h \
    cpmdrive.h \
    cpmclient.h \
    clientgui.h \
    cdrivegui.h

FORMS    += mainwindow.ui \
    clientgui.ui \
    cdrivegui.ui
