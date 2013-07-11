#-------------------------------------------------
#
# Project created by QtCreator 2011-06-25T13:00:29
#
#-------------------------------------------------

QT       += core gui

TARGET = libcurl
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

HEADERS += requester.h
SOURCES += requester.cpp

LIBS += -lpthread

LIBS +=	-L"C:/myprojects/gideros/external/curl-7.21.6/mingw/lib" \
		-L"D:/myprojects/gideros/external/curl-7.21.6/mingw/lib" \
		-lcurldll

INCLUDEPATH += "C:/myprojects/gideros/external/curl-7.21.6/mingw/include"
INCLUDEPATH += "D:/myprojects/gideros/external/curl-7.21.6/mingw/include"

