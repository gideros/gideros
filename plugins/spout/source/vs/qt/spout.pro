QT -= gui
QT += opengl

TARGET = spout
TEMPLATE = lib

INCLUDEPATH += ../../../../../Sdk/include
INCLUDEPATH += ../../../../../libgideros
INCLUDEPATH += ../../../../../libgid/include
INCLUDEPATH += ../../../../../luabinding
INCLUDEPATH += ../../../../../lua/src
INCLUDEPATH += ../../../../../2dsg
INCLUDEPATH += ../../../../../2dsg/gfxbackends

SOURCES += \
    spoutplugin.cpp

HEADERS += 

LIBS += -L"../../../../../Sdk/lib/desktop" -L../spoutplugin/spoutplugin/Release -llua -lgid -lgideros -lspoutplugin

DEFINES += GID_LIBRARY

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
}
