QT -= core ui

TARGET = CuteC2_beta
TEMPLATE = lib

#DESTDIR = ../../plugins/

INCLUDEPATH += ../../../Sdk/include
INCLUDEPATH += ../../../Sdk/include/gideros
INCLUDEPATH += ../../../Src/2dsg
INCLUDEPATH += ../../../Src/2dsg/gfxbackends
INCLUDEPATH += ../../../Src/libgideros
INCLUDEPATH += ../../../Src/libgid/include
INCLUDEPATH += ../../../Src/luabinding
INCLUDEPATH += ../../../Src/lua/src

SOURCES += \
    main.cpp

HEADERS += \
    cute_c2.h

LIBS += -L"../../../Sdk/lib/desktop" -llua -lgid -lgideros

DEFINES += GID_LIBRARY

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
}
