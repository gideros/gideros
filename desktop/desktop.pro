QT += core gui opengl network multimedia

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
    ../libgid/external/freetype-2.4.12/include \
    ../libgid/external/freetype-2.4.12/src \
    ../libgideros \
    ../libpystring \
    ../external/glu \
    ../external/liquidfun-1.0.0/liquidfun/Box2D \
    ../external/minizip-1.1/source \
    ../libraries/themes \
    ../libraries/constants \
    ../player/sources \
    ../player/headers \
    ../player/forms

SOURCES += \
    sources/main.cpp \
    sources/mainwindow.cpp \
    ../player/sources/glcanvas.cpp \
    ../player/sources/errordialog.cpp \
#    ../player/sources/settingsdialog.cpp \
    $$files(../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Common/*.cpp) \
    $$files(../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/*.cpp) \
    $$files(../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/Shapes/*.cpp) \
    $$files(../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/*.cpp) \
    $$files(../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Contacts/*.cpp) \
    $$files(../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints/*.cpp) \
    $$files(../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Rope/*.cpp) \
    $$files(../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Particle/*.cpp) \
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
    ../player/forms/errordialog.ui \
    ../player/forms/settingsdialog.ui \

HEADERS += \
    headers/mainwindow.h \
    ../player/headers/applicationwrapper.h \
    ../player/headers/glcanvas.h \
    ../player/headers/errordialog.h \
#    ../player/headers/settingsdialog.h \
    $$files(../libsound/*.h) \
    $$files(../2dsg/*.h) \
    $$files(../luabinding/*.h) \
    $$files(../libnetwork/*.h) \
    ../2dsg/gfxbackends/gl2/gl2Shaders.h \
    ../2dsg/gfxbackends/Shaders.h

DEFINES += \
    USE_FILE32API \
    FT2_BUILD_LIBRARY \
    DARWIN_NO_CARBON

SOURCES += \
    ../libgid/external/freetype-2.4.12/src/base/ftbbox.c \
    ../libgid/external/freetype-2.4.12/src/base/ftbitmap.c \
    ../libgid/external/freetype-2.4.12/src/base/ftglyph.c \
    ../libgid/external/freetype-2.4.12/src/base/ftlcdfil.c \
    ../libgid/external/freetype-2.4.12/src/base/ftstroke.c \
    ../libgid/external/freetype-2.4.12/src/base/ftxf86.c \
    ../libgid/external/freetype-2.4.12/src/base/ftbase.c \
    ../libgid/external/freetype-2.4.12/src/base/ftsystem.c \
    ../libgid/external/freetype-2.4.12/src/base/ftinit.c \
    ../libgid/external/freetype-2.4.12/src/base/ftgasp.c \
    ../libgid/external/freetype-2.4.12/src/raster/raster.c \
    ../libgid/external/freetype-2.4.12/src/sfnt/sfnt.c \
    ../libgid/external/freetype-2.4.12/src/smooth/smooth.c \
    ../libgid/external/freetype-2.4.12/src/autofit/autofit.c \
    ../libgid/external/freetype-2.4.12/src/truetype/truetype.c \
    ../libgid/external/freetype-2.4.12/src/cff/cff.c \
    ../libgid/external/freetype-2.4.12/src/psnames/psnames.c \
    ../libgid/external/freetype-2.4.12/src/pshinter/pshinter.c
