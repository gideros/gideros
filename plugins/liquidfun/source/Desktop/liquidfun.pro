QT -= gui
QT -= core

TARGET = liquidfun
TEMPLATE = lib

INCLUDEPATH += ../../../../Sdk/include
INCLUDEPATH += ../../../../Sdk/include/gideros
INCLUDEPATH += ../../../../2dsg
INCLUDEPATH += ../../../../2dsg/gfxbackends
INCLUDEPATH += ../../../../libgideros
INCLUDEPATH += ../../../../libgid/include
INCLUDEPATH += ../../../../luabinding
INCLUDEPATH += ../../../../lua/src
INCLUDEPATH += ../liquidfun/Box2D
INCLUDEPATH += ../common

SOURCES += \
    $$files(../common/*.cpp) \
   	../../../../luabinding/binder.cpp \
   	../../../../2dsg/Matrices.cpp \
    $$files(../liquidfun/Box2D/Box2D/Common/*.cpp) \
    $$files(../liquidfun/Box2D/Box2D/Collision/*.cpp) \
    $$files(../liquidfun/Box2D/Box2D/Collision/Shapes/*.cpp) \
    $$files(../liquidfun/Box2D/Box2D/Dynamics/*.cpp) \
    $$files(../liquidfun/Box2D/Box2D/Dynamics/Contacts/*.cpp) \
    $$files(../liquidfun/Box2D/Box2D/Dynamics/Joints/*.cpp) \
    $$files(../liquidfun/Box2D/Box2D/Rope/*.cpp) \
    $$files(../liquidfun/Box2D/Box2D/Particle/*.cpp) \

HEADERS +=

LIBS += -L"../../../../Sdk/lib/desktop" -llua -lgideros -lgid

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
}
