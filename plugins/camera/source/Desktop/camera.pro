QT -= gui
QT += multimedia opengl

TARGET = camera
TEMPLATE = lib

INCLUDEPATH += ../../../../Sdk/include
INCLUDEPATH += ../../../../libgideros
INCLUDEPATH += ../../../../libgid/include
INCLUDEPATH += ../../../../luabinding
INCLUDEPATH += ../../../../lua/src
INCLUDEPATH += ../../../../2dsg
INCLUDEPATH += ../../../../2dsg/gfxbackends

SOURCES += \
    ../common/camerabinder.cpp \
    CameraPlugin.cpp

HEADERS += \
    ../common/camerabinder.h

LIBS += -L"../../../../Sdk/lib/desktop" -llua -lgid -lgideros -lopengl32

DEFINES += GID_LIBRARY FIXED_ORIENTATION

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
}
