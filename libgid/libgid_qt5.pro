#-------------------------------------------------
#
# Project created by QtCreator 2012-04-11T02:20:58
#
#-------------------------------------------------

QT += opengl network

TARGET = gid
TEMPLATE = lib

DEFINES += GIDEROS_LIBRARY

SOURCES += \
    libgid.cpp \

SOURCES += \
    src/gvfs-native.cpp

SOURCES += \
    src/gimage-png.cpp \
    src/gimage-jpg.cpp \
    src/gimage.cpp \
    src/gtexture.cpp \
    src/gevent.cpp \
    src/glog.cpp \
    src/gglobal.cpp

SOURCES += \
    src/qt/gui-qt.cpp \
    src/qt/ginput-qt.cpp \
    src/qt/ggeolocation-qt.cpp \
    src/qt/gapplication-qt.cpp \
    src/qt/ghttp-qt.cpp

SOURCES += \
    src/gaudio.cpp \
    src/qt/gaudio-qt.cpp \
    src/gaudio-sample-openal.cpp \
    src/gaudio-stream-openal.cpp \
    src/gaudio-loader-wav.cpp \
    src/gaudio-loader-xmp.cpp \
    src/gaudio-loader-mp3.cpp
#HEADERS += src/ggaudiomanager.h

HEADERS += include/gexport.h

HEADERS += \
    libgid.h \
    include/gglobal.h \
    include/gimage.h \
    include/gtexture.h \
    include/gevent.h \
    include/glog.h \
    include/gvfs-native.h \
    include/ginput.h \
    include/ggeolocation.h \
    include/gui.h \
    include/gapplication.h \
    include/gaudio.h \
    include/ghttp.h

HEADERS += \
    include/qt/gui-qt.h \
    include/qt/ginput-qt.h \
    include/qt/ghttp-qt.h

INCLUDEPATH += \
    ./include \
    ./include/qt \
    ./include/private \
    ../2dsg/gfxbackends \
    ../2dsg \
    ../libgvfs


# snappy http://code.google.com/p/snappy/
SOURCES += \
    ./external/snappy-1.1.0/snappy.cc \
    ./external/snappy-1.1.0/snappy-c.cc \
    ./external/snappy-1.1.0/snappy-sinksource.cc \
    ./external/snappy-1.1.0/snappy-stubs-internal.cc
HEADERS += \
    ./external/snappy-1.1.0/snappy.h \
    ./external/snappy-1.1.0/snappy-c.h \
    ./external/snappy-1.1.0/snappy-sinksource.h
INCLUDEPATH += ./external/snappy-1.1.0


#LibXMP
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

DEFINES+=_REENTRANT LIBXMP_CORE_PLAYER
XMP_SRC=virtual period player read_event dataio lfo envelope \
		scan control filter effects mixer mix_all load_helpers load \
		hio smix memio
XMP_HDR=common effects envelope format lfo list mixer period player \
		virtual precomp_lut hio memio mdataio tempfile 
XMP_LOADERS=xm_load s3m_load it_load \
			common itsex sample
XMP_LOADERS_HDR=it loader mod s3m xm
SOURCES += $$expand(XMP_SRC,./external/libxmp-4.3/src/,.c)
SOURCES += $$expand(XMP_LOADERS,./external/libxmp-4.3/src/loaders/,.c)
SOURCES += \
	./external/libxmp-4.3/lite/src/format.c \
	./external/libxmp-4.3/lite/src/loaders/mod_load.c
HEADERS += $$expand(XMP_HDR,./external/libxmp-4.3/src/,.h)
HEADERS += $$expand(XMP_LOADERS_HDR,./external/libxmp-4.3/src/loaders/,.h)
INCLUDEPATH += "./external/libxmp-4.3/include"
INCLUDEPATH += "./external/libxmp-4.3/src"
INCLUDEPATH += "./external/libxmp-4.3/src/loaders"

INCLUDEPATH += "./external/libpng-1.6.2"
INCLUDEPATH += "./external/jpeg-9"
INCLUDEPATH += "./external/glew-1.10.0/include"
INCLUDEPATH += "../libgid/external/mpg123-1.15.3/src/libmpg123"

win32 {
DEFINES += OPENAL_SUBDIR_AL
INCLUDEPATH += "./external/openal-soft-1.13/include"
LIBS += -L"../libgid/external/openal-soft-1.13/build/mingw48_32" -lOpenAL32
LIBS += -L"../libgid/external/glew-1.10.0/lib/mingw48_32" -lglew32
LIBS += -L"../libgid/external/libpng-1.6.2/build/mingw48_32" -lpng
LIBS += -L"../libgid/external/jpeg-9/build/mingw48_32" -ljpeg

LIBS += -L"../libgid/external/mpg123-1.15.3/lib/mingw48_32" -lmpg123

LIBS += -lpthread

LIBS += -L"../libgid/external/zlib-1.2.8/build/mingw48_32" -lzlibx
}

macx {
DEFINES += OPENAL_SUBDIR_OPENAL

LIBS += -L"../libgid/external/mpg123-1.15.3/lib/gcc_64" -lmpg123

LIBS += -framework OpenAL
LIBS += -framework OpenGL
LIBS += -framework CoreFoundation

LIBS += -L"../libgid/external/glew-1.10.0/lib/clang_64" -lGLEW
LIBS += -L"../libgid/external/libpng-1.6.2/build/clang_64" -lpng
LIBS += -L"../libgid/external/jpeg-9/build/clang_64" -ljpeg
LIBS += -L"../libgid/external/zlib-1.2.8/build/clang_64" -lzlibx

LIBS += -lpthread
}

unix {
DEFINES += OPENAL_SUBDIR_AL
DEFINES += STRICT_LINUX
INCLUDEPATH += "./external/openal-soft-1.13/include"

LIBS += -L"../libgid/external/libpng-1.6.2/build/gcc484_64" -lpng
#LIBS += -L"../libgid/external/openal-soft-1.13/build/gcc484_64" -lOpenAL32
LIBS += "../libgid/external/openal-soft-1.13/build/gcc484_64/libopenal.so"

LIBS += -L"../libgid/external/glew-1.10.0/lib/gcc484_64" -lGLEW
LIBS += -L"../libgid/external/jpeg-9/build/gcc484_64" -ljpeg

LIBS += -L"../libgid/external/mpg123-1.15.3/lib/gcc484_64" -lmpg123

LIBS += -lpthread

LIBS += -L"../libgid/external/zlib-1.2.8/build/gcc484_64" -lzlibx
}

win32 {
LIBS += -L"../libgvfs/release" -lgvfs
}

macx {
LIBS += -L"../libgvfs" -lgvfs
}

unix {
LIBS += -L"../libgvfs" -lgvfs
}

