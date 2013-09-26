#-------------------------------------------------
#
# Project created by QtCreator 2012-04-11T02:20:58
#
#-------------------------------------------------

QT += opengl network

TARGET = gid
TEMPLATE = lib

DEFINES += GID_LIBRARY

SOURCES += \
    libgid.cpp \

SOURCES += \
    src/stdio/fileops.c \
    src/stdio/extra.c \
    src/stdio/stdio.c \
    src/stdio/flags.c \
    src/stdio/flockfile.c \
    src/stdio/fwalk.c \
    src/stdio/fflush.c \
    src/stdio/findfp.c \
    src/stdio/fopen.c \
    src/stdio/freopen.c \
    src/stdio/fclose.c \
    src/stdio/makebuf.c \
    src/stdio/refill.c \
    src/stdio/fread.c \
    src/stdio/fwrite.c \
    src/stdio/fvwrite.c \
    src/stdio/wsetup.c \
    src/stdio/fseek.c \
    src/stdio/ftell.c \
    src/stdio/feof.c \
    src/stdio/ferror.c \
    src/stdio/clrerr.c \
    src/stdio/getc.c \
    src/stdio/fgetc.c \
    src/stdio/rget.c \
    src/stdio/fgets.c \
    src/stdio/ungetc.c \
    src/stdio/vfscanf.c \
    src/stdio/fscanf.c \
    src/stdio/vfprintf.c \
    src/stdio/fprintf.c \
    src/stdio/setvbuf.c \
    src/stdio/tmpfile.c \
    src/stdio/fputs.c \

SOURCES += \
    src/gvfs-native.cpp

SOURCES += \
    src/gimage-png.cpp \
    src/gimage-jpg.cpp \
    src/gimage.cpp \
    src/gtexture.cpp \
    src/gevent.cpp \
    src/gpath.cpp \
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

HEADERS += \
    libgid.h \
    include/gglobal.h \
    include/gstdio.h \
    include/gimage.h \
    include/gtexture.h \
    include/gpath.h \
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

win32 {
INCLUDEPATH += "./external/libpng-1.6.2"
INCLUDEPATH += "./external/jpeg-9"
INCLUDEPATH += "./external/glew-1.10.0/include"
DEFINES += OPENAL_SUBDIR_AL
INCLUDEPATH += "./external/openal-soft-1.13/include"
LIBS += -L"../libgid/external/openal-soft-1.13/build/mingw" -lOpenAL32
LIBS += -L"../libgid/external/glew-1.10.0/lib/mingw" -lglew32
LIBS += -L"../libgid/external/libpng-1.6.2/build/mingw" -lpng
LIBS += -L"../libgid/external/jpeg-9/build/mingw" -ljpeg

INCLUDEPATH += "../libgid/external/mpg123-1.15.3/src/libmpg123"
LIBS += -L"../libgid/external/mpg123-1.15.3/lib/mingw" -lmpg123

LIBS += -lpthread

LIBS += -L"../libgid/external/zlib-1.2.8/build/mingw" -lzlib
}

macx {
DEFINES += OPENAL_SUBDIR_OPENAL
INCLUDEPATH += "/opt/local/include"

INCLUDEPATH += "../libgid/external/mpg123-1.15.3/src/libmpg123"
LIBS += -L"../libgid/external/mpg123-1.15.3/lib/gcc_64" -lmpg123

LIBS += -framework OpenAL
LIBS += -framework OpenGL
LIBS += -framework CoreFoundation
LIBS += -L"/opt/local/lib" -lpng -ljpeg -lGLEW
LIBS += -lpthread -lz
}

