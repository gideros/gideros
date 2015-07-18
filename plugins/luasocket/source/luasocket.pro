QT -= core gui

TARGET = luasocket
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include

SOURCES += \
    auxiliar.c \
    buffer.c \
    except.c \
    inet.c \
    io.c \
    luasocket.c \
    options.c \
    select.c \
    tcp.c \
    timeout.c \
    udp.c

SOURCES += mime.c

SOURCES += luasocket_stub.cpp

win32 {
SOURCES += wsocket.c
}

macx {
SOURCES += usocket.c
}


HEADERS +=

win32 {
LIBS += -lws2_32
}

LIBS += -L"../../../Sdk/lib/desktop" -llua

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
}
