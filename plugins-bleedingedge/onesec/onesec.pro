#-------------------------------------------------
#
# Project created by QtCreator 2011-12-13T23:23:24
#
#-------------------------------------------------

QT       -= core gui

TARGET = onesec
TEMPLATE = lib

SOURCES += \
    onesec.cpp

HEADERS +=


INCLUDEPATH += ../../lua/src
INCLUDEPATH += ../../libgideros

LIBS += -L"C:/myprojects/gideros/libgfile/release" -lgfile
LIBS += -L"C:/myprojects/gideros/lua/release" -llua
LIBS += -L"C:/myprojects/gideros/libgideros/release" -lgideros
