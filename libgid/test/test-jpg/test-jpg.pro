#-------------------------------------------------
#
# Project created by QtCreator 2012-04-12T13:09:21
#
#-------------------------------------------------

QT       += core gui

TARGET = test-jpg
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

INCLUDEPATH += ../../include

LIBS += -L"C:\myprojects\gidlib\debug" -lgidlib
