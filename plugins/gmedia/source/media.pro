QT += widgets multimedia multimediawidgets

TARGET = gmedia
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include
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
    libpng/png.h \
    gstdio.h \
    gpath.h

LIBS += -L"../../../Sdk/lib/desktop" -llua -lgid -lgideros -lgvfs
LIBS += -L"../../../libgid/external/libpng-1.6.2/build/mingw48_32" -lpng
LIBS += -L"../../../libgid/external/jpeg-9/build/mingw48_32" -ljpeg

win32 {
QMAKE_LFLAGS = -enable-auto-import
}

macx {
QMAKE_CFLAGS_X86_64 += -mmacosx-version-min=10.7
QMAKE_CXXFLAGS_X86_64 = $$QMAKE_CFLAGS_X86_64
}

DEFINES += GIDEROS_LIBRARY

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgvfs.1.dylib "@executable_path/../Frameworks/libgvfs.1.dylib" $(TARGET);
}
