#-------------------------------------------------
#
# Project created by QtCreator 2011-12-13T16:19:54
#
#-------------------------------------------------

QT       -= core gui

TARGET = gfile
TEMPLATE = lib

DEFINES += GIDEROS_LIBRARY

INCLUDEPATH += \
	../libgideros \
    ../pystring \
    ../libgid/include \

SOURCES += \
    gfile.cpp

HEADERS +=\
    gfile.h \
    gfile_p.h

win32 {
LIBS += -L"../libgid/release" -lgid
}

macx {
LIBS += -L"../libgid" -lgid
}








