QT -= core gui

TARGET = harfbuzz
TEMPLATE = lib

INCLUDEPATH += ../../../../Sdk/include ../../../../Sdk/include/gideros
INCLUDEPATH += ../../../../2dsg
INCLUDEPATH += ../../../../2dsg/gfxbackends
INCLUDEPATH += ../../../../libgideros
INCLUDEPATH += ../../../../libgid/include
INCLUDEPATH += ../../../../luabinding
INCLUDEPATH += ../../../../lua/src

HB=../../harfbuzz/src
INCLUDEPATH += $$HB $$HB/hb-ucdn

SOURCES += \
		../Common/harfbuzzbinder.cpp \
		$$HB/hb-common.cc  \ 
		$$HB/hb-cst.cc $$HB/hb-warning.cc \
		$$HB/hb-shape.cc $$HB/hb-shape-plan.cc $$HB/hb-shaper.cc\
		$$HB/hb-font.cc $$HB/hb-face.cc \
		$$HB/hb-blob.cc $$HB/hb-map.cc $$HB/hb-set.cc\
		$$HB/hb-buffer.cc $$HB/hb-buffer-serialize.cc $$HB/hb-unicode.cc\
  		$$files($$HB/hb-aat*.cc) \
  		$$files($$HB/hb-ot*.cc) \
  		$$HB/hb-ucdn.cc \
  		$$HB/hb-ucdn/ucdn.c
  		
DEFINES += HAVE_UCDN

HEADERS += 

LIBS += -L"../../../../Sdk/lib/desktop" -llua -lgid -lgideros -lgvfs

DEFINES += GID_LIBRARY 

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgvfs.1.dylib "@executable_path/../Frameworks/libgvfs.1.dylib" $(TARGET);
}
