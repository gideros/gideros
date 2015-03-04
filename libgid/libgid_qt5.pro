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
    src/gaudio-loader-mp3.cpp
HEADERS += src/ggaudiomanager.h

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

win32 {
LIBS += -L"../libgvfs/release" -lgvfs
}

macx {
LIBS += -L"../libgvfs" -lgvfs
}

