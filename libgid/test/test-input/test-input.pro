#-------------------------------------------------
#
# Project created by QtCreator 2012-09-12T18:45:59
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = test-input
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

INCLUDEPATH += ../../include ../../include/qt

LIBS += -L"C:/myprojects/gideros/libgid/debug" -lgid
