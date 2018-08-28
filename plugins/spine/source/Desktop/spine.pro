QT -= core gui

TARGET = spine
TEMPLATE = lib

INCLUDEPATH += ../../../../Sdk/include ../../../../Sdk/include/gideros
INCLUDEPATH += ../../../../2dsg
INCLUDEPATH += ../../../../2dsg/gfxbackends
INCLUDEPATH += ../../../../libgideros
INCLUDEPATH += ../../../../libgid/include
INCLUDEPATH += ../../../../luabinding
INCLUDEPATH += ../../../../lua/src


INCLUDEPATH += ../../spine-runtimes/spine-c/spine-c/include

SOURCES += ../Common/spinebinder.cpp \
 	 	../../../../luabinding/binder.cpp \
  		$$files(../../spine-runtimes/spine-c/spine-c/src/spine/*.c)

HEADERS += 

LIBS += -L"../../../../Sdk/lib/desktop" -llua -lgid -lgideros -lgvfs

DEFINES += GID_LIBRARY 

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgvfs.1.dylib "@executable_path/../Frameworks/libgvfs.1.dylib" $(TARGET);
}
