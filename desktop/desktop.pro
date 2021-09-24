QT += core gui opengl network multimedia
CONFIG   += silent

win32{
    RC_FILE = other_files/desktop.rc

    TARGET = WindowsDesktopTemplate

    LIBS += -lopengl32 \
        -L"../libgid/external/zlib-1.2.8/build/mingw48_32" -lzlibx \
        -L"../libgid/external/glew-1.10.0/lib/mingw48_32" -lglew32 \
        -L"../libgid/release" -lgid \
        -L"../libgvfs/release" -lgvfs \
        -L"../lua/release" -llua \
        -L"../libgideros/release" -lgideros \
        -L"../libpystring/release" -lpystring \
        -lws2_32 \
        -liphlpapi
}

macx {
    ICON = other_files/desktop.icns

    TARGET = "MacOSXDesktopTemplate"

    LIBS += \
        -framework OpenAL \
        -framework OpenGL \
        -framework CoreFoundation \
        -framework IOKit\
        -L"../libgid" -lgid \
        -L"../libgvfs" -lgvfs \
        -L"../lua" -llua \
        -L"../libgideros" -lgideros \
        -L"../libpystring" -lpystring \
        -L"../libgid/external/zlib-1.2.8/build/clang_64" -lzlibx \
        -L"../libgid/external/glew-1.10.0/lib/clang_64" -lGLEW

    QMAKE_LFLAGS += -pagezero_size 10000 -image_base 100000000
    QMAKE_CFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_OBJECTIVE_CFLAGS_RELEASE =  $$QMAKE_OBJECTIVE_CFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
}

unix:!macx {
    TARGET = "LinuxDesktopTemplate"

    DEFINES += STRICT_LINUX
    LIBS += \
        -L"../libgid/external/zlib-1.2.8/build/gcc484_64" -lzlibx\
        -L"../libgid/external/glew-1.10.0/lib/gcc484_64" -lGLEW\
#        -lwsock32\
#        -liphlpapi\
        ../libgid/libgid.so \
        ../libgvfs/libgvfs.so \
        ../lua/liblua.so \
        ../libgideros/libgideros.so \
        ../libpystring/libpystring.so
    LIBS += "../libgid/external/openal-soft-1.13/build/gcc484_64/libopenal.so"
    QMAKE_CXXFLAGS += -std=gnu++11
    QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN\''
}

LIBS += -lpthread

TEMPLATE = app

INCLUDEPATH += \
    headers \
    sources \
    forms \
    ../libgid/external/zlib-1.2.8 \
    ../libgid/external/glew-1.10.0/include \
    ../2dsg \
    ../2dsg/gfxbackends \
    ../2dsg/gfxbackends/gl2 \
    ../2dsg/paths \
    ../libsound \
    ../libnetwork \
    ../luabinding \
    ../lua/src \
    ../libpvrt \
    ../libgvfs \
    ../libgid/include \
    ../libgid/include/qt \
    ../libgideros \
    ../libpystring \
    ../external/glu \
    ../external/minizip-1.1/source \
    ../libraries/themes \
    ../libraries/constants \
    ../player/Sources \
    ../player/Headers \
    ../player/Forms

SOURCES += \
    ../player/Sources/tabletapplication.cpp \
    sources/main.cpp \
    sources/mainwindow.cpp \
    ../player/Sources/glcanvas.cpp \
    ../player/Sources/errordialog.cpp \
#    ../player/Sources/settingsdialog.cpp \
    $$files(../luabinding/*.cpp)	../luabinding/tlsf.c \
    $$files(../libnetwork/*.cpp) \
    $$files(../2dsg/*.cpp) \
    $$files(../2dsg/gfxbackends/*.cpp) \
    $$files(../2dsg/gfxbackends/gl2/*.cpp) \
    $$files(../2dsg/paths/*.cpp) ../2dsg/paths/ft-path.c ../2dsg/paths/svg-path.c \
    $$files(../libpvrt/*.cpp) \
    $$files(../libpvrt/*.h) \
    $$files(../external/glu/libtess/*.c) \
    ../libgid/src/md5.c \
    ../libgid/src/aes.c \
    ../libgid/src/platformutil.cpp \
    ../libgid/src/utf8.c \
    ../libgid/src/drawinfo.cpp \
    ../libgid/src/qt/platform-qt.cpp \
    ../libgid/src/gtimer.cpp \
    ../external/minizip-1.1/source/ioapi.c \
    ../external/minizip-1.1/source/unzip.c

FORMS += \
    forms/mainwindow.ui \
    ../player/Forms/errordialog.ui \
    ../player/Forms/settingsdialog.ui \

HEADERS += \
    ../player/Headers/tabletapplication.h \
    headers/mainwindow.h \
    ../player/Headers/applicationwrapper.h \
    ../player/Headers/glcanvas.h \
    ../player/Headers/errordialog.h \
#    ../player/Headers/settingsdialog.h \
    $$files(../libsound/*.h) \
    $$files(../2dsg/*.h) \
    $$files(../luabinding/*.h) \
    $$files(../libnetwork/*.h) \
    ../2dsg/gfxbackends/gl2/gl2Shaders.h \
    ../2dsg/gfxbackends/Shaders.h

#FREETYPE_VER=2.4.12
FREETYPE_VER=2.7.1

DEFINES += \
    USE_FILE32API \
    FT2_BUILD_LIBRARY \
    DARWIN_NO_CARBON

INCLUDEPATH += ../libgid/include
INCLUDEPATH += ../libgid/external/freetype-$${FREETYPE_VER}/include
INCLUDEPATH += ../libgid/external/freetype-$${FREETYPE_VER}/src

SOURCES += \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftbbox.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftbitmap.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftglyph.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftlcdfil.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftstroke.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftbase.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftsystem.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftinit.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftgasp.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/raster/raster.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/sfnt/sfnt.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/smooth/smooth.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/autofit/autofit.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/truetype/truetype.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/cff/cff.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/gzip/ftgzip.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/psnames/psnames.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/pshinter/pshinter.c
#2.4.12    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftxf86.c
