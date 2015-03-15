QT -= core gui

TARGET = controller
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include

SOURCES += \
    controllerbinder.cpp \
    controller.cpp \
    Gamepad_private.c \
    gcontroller.cpp

HEADERS += \
    controller.h \
    hidapi.h \
    Gamepad.h \
    Gamepad_private.h \
    gcontroller.h

LIBS += -L"../../../Sdk/lib/desktop" -llua -lgid -lgideros

DEFINES += GID_LIBRARY

win32 {
SOURCES += Gamepad_windows.c
LIBS += -lwinmm
}

macx {
SOURCES += Gamepad_macosx.c
LIBS += -framework CoreFoundation
LIBS += -framework IOKit
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
}
