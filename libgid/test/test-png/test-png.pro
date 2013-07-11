#-------------------------------------------------
#
# Project created by QtCreator 2012-04-12T00:08:35
#
#-------------------------------------------------

QT       += core gui

TARGET = test-png
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui


INCLUDEPATH += ../../include

LIBS += -L"C:\myprojects\gidlib\debug" -lgidlib
