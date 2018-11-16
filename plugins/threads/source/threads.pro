QT -= core gui

TARGET = threads
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include
INCLUDEPATH += ../../../luabinding
INCLUDEPATH +="../../../libgid/external/zlib-1.2.8"

SOURCES += \
    threads_entry.cpp \
    luathread.cpp \
    StateToState.cpp \
    ../../lfs/source/lfs.c \
    ../../../luabinding/binder.cpp \
    ../../../luabinding/zlibbinder.cpp \
    threadtimedluahook.cpp

SOURCES+= \
    ../../luasocket/source/auxiliar.c \
    ../../luasocket/source/buffer.c \
    ../../luasocket/source/except.c \
    ../../luasocket/source/inet.c \
    ../../luasocket/source/io.c \
    ../../luasocket/source/luasocket.c \
    ../../luasocket/source/options.c \
    ../../luasocket/source/select.c \
    ../../luasocket/source/tcp.c \
    ../../luasocket/source/timeout.c \
    ../../luasocket/source/udp.c \
    ../../luasocket/source/mime.c

SOURCES += \
    ../../json/source/strbuf.c \
    ../../json/source/fpconv.c \
    ../../json/source/lua_cjson.c

HEADERS += \
    ../../json/source/strbuf.h \
    ../../json/source/fpconv.h \
    threadtimedluahook.h


HEADERS += \
    ../../luasocket/source/luasocket.h \
    luathread.h \
    StateToState.h \
    macros.h

# luasocket
DEFINES += LUA_NOCOMPAT_MODULE

win32{
    SOURCES += ../../luasocket/source/wsocket.c
    DEFINES += LUASOCKET_INET_PTON

    LIBS += -lws2_32
    LIBS += -L"../../../libgid/external/zlib-1.2.8/build/mingw48_32" -lzlibx

    debug_in_place{
        message("Build Target: Debug In Place")
        DESTDIR = ../../../Build.Win/Plugins
        LIBS += -L"../../../Build.Win" -llua -lgideros -lgvfs
    } else {
        message("Build Target: release or debug")
        LIBS += -L"../../../Sdk/lib/desktop" -llua -lgideros -lgvfs -lgid
    }
} else {
    LIBS += -L"../../../Sdk/lib/desktop" -llua -lgideros -lgvfs -lgid
}

#Linux + macOS + pi
unix {
    SOURCES += ../../luasocket/source/usocket.c
}

#Linux + pi
unix:!macx {
    LIBS += -L"../../../libgid/external/zlib-1.2.8/build/gcc484_64" -lzlibx
}

macx {
    LIBS += -L"../../../libgid/external/zlib-1.2.8/build/clang_64" -lzlibx
    QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
    QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
    QMAKE_POST_LINK += install_name_tool -change libgvfs.1.dylib "@executable_path/../Frameworks/libgvfs.1.dylib" $(TARGET);
    QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
}



