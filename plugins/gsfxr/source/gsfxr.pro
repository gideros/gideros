QT -= core gui

TARGET = gsfxr
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include ../../../Sdk/include/gideros
INCLUDEPATH += ../../../libgideros
INCLUDEPATH += ../../../libgid/include
INCLUDEPATH += ../../../luabinding
INCLUDEPATH += ../../../lua/src


SOURCES += Common/gaudio-loader-sfx.cpp Common/retrosfxvoice.cpp

HEADERS += 

LIBS += -L"../../../Sdk/lib/desktop" -llua -lgid -lgideros -lgvfs

DEFINES += GID_LIBRARY 

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgvfs.1.dylib "@executable_path/../Frameworks/libgvfs.1.dylib" $(TARGET);
}
