QT -= core gui

TARGET = lualanes
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include

SOURCES += \
    custom_alloc.c \
    lanes.c \
    compat.c \
    threading.c \
    tools.c \
    deep.c \
    keeper.c \
    universe.c

SOURCES += lualanes_stub.cpp

HEADERS += \
    macros_and_utils.h

DEFINES += CUSTOM_ALLOC

LIBS += -L"../../../Sdk/lib/desktop" -llua

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
}
