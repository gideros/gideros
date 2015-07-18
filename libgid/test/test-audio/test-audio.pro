#-------------------------------------------------
#
# Project created by QtCreator 2012-11-20T11:12:23
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = test-audio
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

INCLUDEPATH += ../../include ../../include/qt

LIBS += -L"../../debug" -lgid
