QT += widgets multimedia multimediawidgets

TARGET = gmedia
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include
INCLUDEPATH += ../../../libgvfs
INCLUDEPATH += ./libpng
INCLUDEPATH += ./jpeglib

SOURCES += \
    mediabinder.cpp \
    media.cpp

HEADERS += \
    media.h \
    CImg.h \
    jpeglib/jpeglib.h \
    jpeglib/jmorecfg.h \
    jpeglib/jconfig.h \
    libpng/pngconf.h \
    libpng/pnglibconf.h \
    libpng/pngconf.h \
    libpng/png.h

defineReplace(expand) {
    variable = $$1
    prefix=$$2
    suffix=$$3
    names = $$eval($$variable)
    expanded =

    for(name, names) {
        expanded+= $${prefix}$${name}$${suffix}
    }
    return ($$expanded)
}

#ZLIB
LZIP_FILES=adler32 compress crc32 deflate gzclose gzlib gzread gzwrite infback inffast inflate inftrees trees uncompr zutil
#SOURCES += $$expand(LZIP_FILES,../../../libgid/external/zlib-1.2.8/,.c)

#PNG
PNGFILES=png pngerror pngget pngmem pngpread pngread pngrio pngrtran pngrutil pngset pngtrans \
        pngwio pngwrite pngwtran pngwutil
SOURCES += $$expand(PNGFILES,../../../libgid/external/libpng-1.6.2/,.c)

#JPEG
JPEGFILES=jaricom jdapimin jdapistd jdarith jdatadst jdatasrc jdcoefct jdcolor jddctmgr jdhuff jdinput \
        jdmainct jdmarker jdmaster jdmerge jdpostct jdsample jdtrans jerror jfdctflt jfdctfst jfdctint \
        jidctflt jidctfst jidctint jquant1 jquant2 jutils jmemmgr jmemnobs jcomapi \
        jcapimin jcapistd jcarith jccoefct jccolor jcdctmgr jchuff jcinit jcmainct \
        jcmarker jcmaster jcparam jcprepct jcsample jctrans
SOURCES += $$expand(JPEGFILES,../../../libgid/external/jpeg-9/,.c)


LIBS += -L"../../../Sdk/lib/desktop" -llua -lgid -lgideros -lgvfs

INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib

win32 {
#LIBS += -L"../../../libgid/external/libpng-1.6.2/build/mingw48_32" -lpng
#LIBS += -L"../../../libgid/external/jpeg-9/build/mingw48_32" -ljpeg
#LIBS += -L"../../../libgid/external/zlib-1.2.8/build/mingw48_32" -lzlibx
}

macx {
LIBS += -lz
QMAKE_CFLAGS_X86_64 += -mmacosx-version-min=10.7
QMAKE_CXXFLAGS_X86_64 = $$QMAKE_CFLAGS_X86_64
}

unix:!macx {
LIBS += -lz
}

DEFINES += GIDEROS_LIBRARY

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgvfs.1.dylib "@executable_path/../Frameworks/libgvfs.1.dylib" $(TARGET);
}
