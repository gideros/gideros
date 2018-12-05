QT -= core gui

TARGET = luamidi
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include

SOURCES += \
    luamidi.cpp \
    RtMidi.cpp


HEADERS +=

win32 {
DEFINES+=__WINDOWS_MM__
LIBS += -lwinmm
}

LIBS += -L"../../../Sdk/lib/desktop" -llua

macx {
DEFINES+=__MACOSX_CORE__
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
}
