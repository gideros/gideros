QT -=  gui

TARGET = ogg
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include ../../../Sdk/include/gideros
INCLUDEPATH += ../../../2dsg
INCLUDEPATH += ../../../2dsg/gfxbackends
INCLUDEPATH += ../../../libgideros
INCLUDEPATH += ../../../libgid/include
INCLUDEPATH += ../../../luabinding
INCLUDEPATH += ../../../lua/src

XIPH_OGG=libogg-1.3.2
XIPH_THEORA=libtheora-1.1.1
XIPH_VORBIS=libvorbis-1.3.5

SOGG_F=bitwise framing
STHEORADEC_F=apiwrapper bitpack decapiwrapper decinfo decode dequant fragment huffdec idct th_info internal quant state
SVORBIS_F=mdct block window synthesis info floor1 floor0 res0 mapping0 registry codebook sharedbook envelope psy bitrate lpc lsp smallft vorbisfile

for(f,SOGG_F):SXIPH+=$$XIPH_OGG/src/$${f}.c
for(f,STHEORADEC_F):SXIPH+=$$XIPH_THEORA/lib/$${f}.c
for(f,SVORBIS_F):SXIPH+=$$XIPH_VORBIS/lib/$${f}.c

INCLUDEPATH += $$XIPH_OGG/include
INCLUDEPATH += $$XIPH_THEORA/include
INCLUDEPATH += $$XIPH_VORBIS/include

SOURCES += Common/oggbinder.cpp \
    $$SXIPH

HEADERS += 

LIBS += -L"../../../Sdk/lib/desktop" -llua -lgid -lgideros -lgvfs

DEFINES += GID_LIBRARY 

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgvfs.1.dylib "@executable_path/../Frameworks/libgvfs.1.dylib" $(TARGET);
}
