QT += core gui opengl network

INCLUDEPATH += \
    "../libgid/external/zlib-1.2.8"\
    "../libgid/external/glew-1.10.0/include"



TARGET = GiderosPlayer
DEFINES += STRICT_LINUX
DEFINES += RASPBERRY_PI
LIBS += \
        -L"../libgid/external/zlib-1.2.8/build/gcc463_pi" -lzlibx\
#       -L"../libgid/external/glew-1.10.0/lib/gcc463_pi" -lGLEW\
#        -lwsock32\
#        -liphlpapi\
        ../libgid/libgid.so \
        ../libgvfs/libgvfs.so \
        ../lua/liblua.so \
        ../libgideros/libgideros.so \
        ../libpystring/libpystring.so
LIBS += "../libgid/external/openal-soft-1.13/build/gcc463_pi/libopenal.so"
QMAKE_CXXFLAGS += -std=gnu++0x

TEMPLATE = app

INCLUDEPATH += \
    Headers \
    Sources \
    Forms \
    ../2dsg \
    ../2dsg/paths \
    ../2dsg/gfxbackends \
    ../2dsg/gfxbackends/gl2 \
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
    "../external/glu" \
    ../external/liquidfun-1.0.0/liquidfun/Box2D \
    "../external/minizip-1.1/source" \
    ../libraries/themes \
    ../libraries/constants \

SOURCES += \
    Sources/main.cpp \
    Sources/mainwindow.cpp \
    Sources/errordialog.cpp \
    Sources/glcanvas.cpp \
    Sources/settingsdialog.cpp \
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
    ../libgid/src/aes.c \
    ../libgid/src/md5.c \
    ../libgid/src/platformutil.cpp \
    ../libgid/src/utf8.c \
    ../libgid/src/drawinfo.cpp \
    ../libgid/src/qt/platform-qt.cpp \
    ../libgid/src/gtimer.cpp \
    $$files(../2dsg/*.cpp) \
    $$files(../2dsg/gfxbackends/*.cpp) \
    $$files(../2dsg/gfxbackends/gl2/*.cpp) \
    $$files(../2dsg/paths/*.cpp) ../2dsg/paths/ft-path.c ../2dsg/paths/svg-path.c \
    $$files(../libpvrt/*.cpp) \
    $$files(../external/glu/libtess/*.c) \
    "../external/minizip-1.1/source/ioapi.c" \
    "../external/minizip-1.1/source/unzip.c" \
    $$files(../libpvrt/*.h)

FORMS += \
    Forms/mainwindow.ui\
    Forms/errordialog.ui \
    Forms/settingsdialog.ui

HEADERS += \
    Headers/mainwindow.h \
    Headers/errordialog.h \
    Headers/glcanvas.h \
    Headers/settingsdialog.h\
    $$files(../libsound/*.h)\
    $$files(../2dsg/*.h)\
    $$files(../luabinding/*.h)\
    $$files(../libnetwork/*.h)\
    ../2dsg/gfxbackends/gl2/gl2Shaders.h \
    ../2dsg/gfxbackends/Shaders.h

DEFINES += USE_FILE32API

LIBS += -lpthread

#INCLUDEPATH += ../external/Box2D_v2.3.0/Box2D
#SOURCES += $$files(../external/Box2D_v2.3.0/Box2D/Box2D/Common/*.cpp)
#SOURCES += $$files(../external/Box2D_v2.3.0/Box2D/Box2D/Collision/*.cpp)
#SOURCES += $$files(../external/Box2D_v2.3.0/Box2D/Box2D/Collision/Shapes/*.cpp)
#SOURCES += $$files(../external/Box2D_v2.3.0/Box2D/Box2D/Dynamics/*.cpp)
#SOURCES += $$files(../external/Box2D_v2.3.0/Box2D/Box2D/Dynamics/Contacts/*.cpp)
#SOURCES += $$files(../external/Box2D_v2.3.0/Box2D/Box2D/Dynamics/Joints/*.cpp)
#SOURCES += $$files(../external/Box2D_v2.3.0/Box2D/Box2D/Rope/*.cpp)


# modified from http://source-android.frandroid.com/external/freetype/Android.mk
# also ftmodule.h should be configured like http://source-android.frandroid.com/external/freetype/include/freetype/config/ftmodule.h
DEFINES += FT2_BUILD_LIBRARY
DEFINES += DARWIN_NO_CARBON

INCLUDEPATH += ../libgid/include
INCLUDEPATH += ../libgid/external/freetype-2.4.12/include
INCLUDEPATH += ../libgid/external/freetype-2.4.12/src

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
