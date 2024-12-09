QT -= core gui

TARGET = reactphysics3d
TEMPLATE = lib

INCLUDEPATH += ../../../../Sdk/include ../../../../Sdk/include/gideros
INCLUDEPATH += ../../../../2dsg
INCLUDEPATH += ../../../../2dsg/gfxbackends
INCLUDEPATH += ../../../../libgideros
INCLUDEPATH += ../../../../libgid/include
INCLUDEPATH += ../../../../luabinding
INCLUDEPATH += ../../../../lua/src

INCLUDEPATH += ../../reactphysics3d/include

SOURCES += ../Common/reactbinder.cpp \
 	 	../../../../luabinding/binder.cpp \
  		$$files(../../reactphysics3d/src/body/*.cpp) \
  		$$files(../../reactphysics3d/src/collision/*.cpp) \
  		$$files(../../reactphysics3d/src/collision/broadphase/*.cpp) \
  		$$files(../../reactphysics3d/src/collision/narrowphase/*.cpp) \
  		$$files(../../reactphysics3d/src/collision/narrowphase/GJK/*.cpp) \
  		$$files(../../reactphysics3d/src/collision/narrowphase/SAT/*.cpp) \
  		$$files(../../reactphysics3d/src/collision/shapes/*.cpp) \
  		$$files(../../reactphysics3d/src/components/*.cpp) \
  		$$files(../../reactphysics3d/src/constraint/*.cpp) \
  		$$files(../../reactphysics3d/src/engine/*.cpp) \
  		$$files(../../reactphysics3d/src/mathematics/*.cpp) \
  		$$files(../../reactphysics3d/src/memory/*.cpp) \
  		$$files(../../reactphysics3d/src/systems/*.cpp) \
                $$files(../../reactphysics3d/src/utils/*.cpp) \
                $$files(../../reactphysics3d/src/utils/quickhull\*.cpp)

HEADERS += 

LIBS += -L"../../../../Sdk/lib/desktop" -llua -lgid -lgideros -lgvfs

DEFINES += GID_LIBRARY 

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgvfs.1.dylib "@executable_path/../Frameworks/libgvfs.1.dylib" $(TARGET);
}
