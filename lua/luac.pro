TEMPLATE = app
CONFIG += console
DESTDIR = bin
TARGET = luac
SOURCES += src/luac.c
INCLUDEPATH += src
win32 {
  DEFINES += LUA_BUILD_AS_DLL
}
