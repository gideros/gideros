#-------------------------------------------------
#
# Project created by QtCreator 2011-04-15T11:18:26
#
#-------------------------------------------------

QT       += core gui network

TARGET = macclient
TEMPLATE = app

INCLUDEPATH += "C:/OpenSSL-Win32/include"

SOURCES += main.cpp\
        dialog.cpp \
    startpage.cpp \
    signplayerpage.cpp \
    signplayerrunpage.cpp

HEADERS  += dialog.h \
    startpage.h \
    signplayerpage.h \
    signplayerrunpage.h

FORMS    += dialog.ui \
    startpage.ui \
    signplayerpage.ui \
    signplayerrunpage.ui

LIBS += -L"C:/OpenSSL-Win32/lib/MinGW" -leay32
