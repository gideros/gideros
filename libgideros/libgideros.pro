#-------------------------------------------------
#
# Project created by QtCreator 2011-12-14T00:11:16
#
#-------------------------------------------------

QT       -= core gui

TARGET = gideros
TEMPLATE = lib

DEFINES += GIDEROS_LIBRARY

SOURCES += \
    binderutil.cpp \
    stringid.cpp \
    eventdispatcher.cpp \
    event.cpp \
    refptr.cpp \
    eventvisitor.cpp \
    pluginmanager.cpp \
    luautil.cpp

HEADERS += gexport.h

HEADERS += \
	gplugin.h \
    stringid.h \
    eventtype.h \
    eventdispatcher.h \
    event.h \
    refptr.h \
    eventvisitor.h \
    gideros_p.h \
    gproxy.h \
    gideros.h \
    pluginmanager.h \
    luautil.h

INCLUDEPATH += \
	. \
	../lua/src \
	../libpystring

win32 {
LIBS += -L"../libgid/release" -lgid
LIBS += -L"../lua/release" -llua
LIBS += -L"../libpystring/release" -lpystring
}

macx {
LIBS += -L"../libgid" -lgid
LIBS += -L"../lua" -llua
LIBS += -L"../libpystring" -lpystring
}

unix {
LIBS += ../libgid/libgid.so
LIBS += ../lua/liblua.so
LIBS += ../libpystring/libpystring.so
}
