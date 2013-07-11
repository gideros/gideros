#-------------------------------------------------
#
# Project created by QtCreator 2013-06-04T09:29:10
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test-http
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui


INCLUDEPATH += ../../include ../../include/qt

LIBS += -L"../../debug" -lgid
