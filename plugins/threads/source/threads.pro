QT -= core gui

TARGET = threads
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include
INCLUDEPATH += ../../../luabinding

SOURCES += \
    threads_entry.cpp \
    luathread.cpp \
    StateToState.cpp \
    ../../../luabinding/binder.cpp

HEADERS += \
    luathread.h \
    StateToState.h \
    macros.h

win32{
    debug_in_place{
        message("Build Target: Debug In Place")
        DESTDIR = ../../../Build.Win/Plugins
        INCLUDEPATH += ../../../2dsg \
                       ../../../2dsg/gfxbackends \
                       ../../../2dsg/gfxbackends/gl2 \
                       ../../../2dsg/paths \
                       ../../../libsound \
                       ../../../libnetwork \
                       ../../../luabinding \
                       ../../../lua/src \
                       ../../../libpvrt \
                       ../../../libgvfs \
                       ../../../libgid/include \
                       ../../../libgid/include/qt \
                       ../../../libgideros \
                       ../../../libpystring \
                       ../../../external/glu \
                       ../../../external/minizip-1.1/source \
                       ../../../libraries/themes \
                       ../../../libraries/constants \
        SOURCES += $$files(../../../lua/*.c)
        LIBS += -L"../../../Build.Win" -llua -lgideros
    } else {
        message("Build Target: release or debug")
        LIBS += -L"../../../Sdk/lib/desktop" -llua -lgideros
    }
} else {
    LIBS += -L"../../../Sdk/lib/desktop" -llua -lgideros
}


macx {
    QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
}
