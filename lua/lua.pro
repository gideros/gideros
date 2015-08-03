#-------------------------------------------------
#
# Project created by QtCreator 2011-12-13T12:06:49
#
#-------------------------------------------------

QT       -= core gui

TARGET = lua
TEMPLATE = lib

win32 {
DEFINES += LUA_BUILD_AS_DLL
}
INCLUDEPATH += \
	src \
        ../libgvfs

SOURCES += etc/all_lua.c

HEADERS += \
    src/lzio.h \
    src/lvm.h \
    src/lundump.h \
    src/lualib.h \
    src/luaconf.h \
    src/lua.hpp \
    src/lua.h \
    src/ltm.h \
    src/ltable.h \
    src/lstring.h \
    src/lstate.h \
    src/lparser.h \
    src/lopcodes.h \
    src/lobject.h \
    src/lmem.h \
    src/llimits.h \
    src/llex.h \
    src/lgc.h \
    src/lfunc.h \
    src/ldo.h \
    src/ldebug.h \
    src/lcode.h \
    src/lauxlib.h \
    src/lapi.h

win32 {
LIBS += -L"../libgvfs/release" -lgvfs
}

macx {
LIBS += -L"../libgvfs" -lgvfs
}

unix {
LIBS += ../libgvfs/libgvfs.so
}
