#-------------------------------------------------
#
# Project created by QtCreator 2011-12-14T00:11:16
#
#-------------------------------------------------

QT       -= core gui

TARGET = gideros
TEMPLATE = lib

LUA_ENGINE=$$(LUA_ENGINE)
isEmpty(LUA_ENGINE) {
    LUA_ENGINE=lua
}
equals(LUA_ENGINE,luau) {
    LUA_INCLUDE=../luau/VM/include
}
else
{
    LUA_INCLUDE=../lua/src
}

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
        $$LUA_INCLUDE \
	../libpystring

win32 {
LIBS += -L"../libgid/release" -lgid
LIBS += -L"../$$LUA_ENGINE/release" -llua
LIBS += -L"../libpystring/release" -lpystring
}

macx {
LIBS += -L"../libgid" -lgid
LIBS += -L"../$$LUA_ENGINE" -llua
LIBS += -L"../libpystring" -lpystring
} else {
unix:!macx {
LIBS += ../libgid/libgid.so
LIBS += ../$$LUA_ENGINE/liblua.so
LIBS += ../libpystring/libpystring.so
}
}
