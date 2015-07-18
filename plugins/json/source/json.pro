QT -= core gui

TARGET = json
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include

SOURCES += \
    strbuf.c \
    fpconv.c \
    lua_cjson.c \
    lua_cjson_stub.cpp

HEADERS += \
    strbuf.h \
    fpconv.h

LIBS += -L"../../../Sdk/lib/desktop" -llua

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
}
