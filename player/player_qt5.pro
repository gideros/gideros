QT += core gui opengl network multimedia widgets
CONFIG   += silent qt

//QMAKE_CXXFLAGS_DEBUG += -O2

equals(QT_MAJOR_VERSION, 6){
   QT += openglwidgets
   win32 {
       #This is ugly, but needed for wintab mode
       INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtGui/$$[QT_VERSION]
       INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtGui/$$[QT_VERSION]/QtGui
       INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtCore/$$[QT_VERSION]
       INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtCore/$$[QT_VERSION]/QtCore
   }
}

LUA_ENGINE=$$(LUA_ENGINE)
isEmpty(LUA_ENGINE): LUA_ENGINE=luau
equals(LUA_ENGINE,luau): LUA_INCLUDE=../luau/VM/include ../luau/VM/src ../luau/Common/include ../luau/CodeGen/include
equals(LUA_ENGINE,lua): LUA_INCLUDE=../lua/src

INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib

INCLUDEPATH += \
#    "../libgid/external/zlib-1.2.8"
#    "../libgid/external/glew-1.10.0/include"

win32{
    RC_FILE = other_files/player.rc

    TARGET = GiderosPlayer
    QMAKE_LFLAGS+=-Wl,-Map=LinkerMap.txt

    LIBS += \
#        -L"../libgid/external/zlib-1.2.8/build/mingw48_32" -lzlibx\
#        -L"../libgid/external/glew-1.10.0/lib/mingw48_32" -lglew32\
        -lws2_32\
        -liphlpapi

    CONFIG(debug, debug|release){
        _CONFIG_ = debug
    } else {
        _CONFIG_ = release
    }

    debug_in_place{
        message("Build Target: Debug with Installed Libs")
        DESTDIR = ../Build.Win
        LIBS += \
            -L"../Build.Win" -lgid\
            -L"../Build.Win" -lgvfs\
            -L"../Build.Win" -llua\
            -L"../Build.Win" -lgideros\
            -L"../Build.Win" -lpystring
    } else {
        message("Build Target: "$$_CONFIG_)
        LIBS += \
            -L"../libgid/$$_CONFIG_" -lgid\
            -L"../libgid/openal/$$_CONFIG_" -lopenal\
            -L"../libgvfs/$$_CONFIG_" -lgvfs\
            -L"../$$LUA_ENGINE/$$_CONFIG_" -llua\
            -L"../libgideros/$$_CONFIG_" -lgideros\
            -L"../libpystring/$$_CONFIG_" -lpystring
    }
}

macx {
    ICON = other_files/player.icns

    TARGET = "Gideros Player"

    LIBS += \
        -framework OpenAL\
        -framework OpenGL\
        -framework CoreFoundation\
        -framework IOKit\
        -L"../libgid" -lgid\
        -L"../libgvfs" -lgvfs\
        -L"../$$LUA_ENGINE" -llua\
        -L"../libgideros" -lgideros\
        -L"../libpystring" -lpystring\
		-lz\
#        -L"../libgid/external/zlib-1.2.8/build/clang_64" -lzlibx\
#        -L"../libgid/external/glew-1.10.0/lib/clang_64" -lGLEW\

    QMAKE_LFLAGS += -pagezero_size 10000 -image_base 100000000
    QMAKE_CFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_OBJECTIVE_CFLAGS_RELEASE =  $$QMAKE_OBJECTIVE_CFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
} 

unix:!macx {
    TARGET = GiderosPlayer
    DEFINES += STRICT_LINUX
    LIBS += \
#        -L"../libgid/external/zlib-1.2.8/build/gcc484_64" -lzlibx\
#        -L"../libgid/external/glew-1.10.0/lib/gcc484_64" -lGLEW\
#        -lwsock32\
#        -liphlpapi\
		-lz\
        ../libgid/libgid.so \
        ../libgvfs/libgvfs.so \
        ../$$LUA_ENGINE/liblua.so \
        ../libgideros/libgideros.so \
        ../libpystring/libpystring.so
    LIBS += "../libgid/external/openal-soft-1.13/build/gcc484_64/libopenal.so"
    QMAKE_CXXFLAGS += -std=gnu++17
    QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN\''
}


TEMPLATE = app

INCLUDEPATH += \
    Headers \
    Sources \
    Forms \
    ../2dsg \
    ../2dsg/gfxbackends \
    ../2dsg/gfxbackends/gl2 \
    ../2dsg/paths \
    ../libsound \
    ../libnetwork \
    ../luabinding \
    $$LUA_INCLUDE \
    ../libpvrt \
    ../libgvfs \
    ../libgid/include \
    ../libgid/include/qt \
    ../libgideros \
    ../libpystring \
    "../external/glu" \
    "../external/minizip-1.1/source" \
    ../libraries/themes \
    ../libraries/constants \

SOURCES += \
    Sources/main.cpp \
    Sources/mainwindow.cpp \
    Sources/errordialog.cpp \
    Sources/glcanvas.cpp \
    Sources/settingsdialog.cpp \
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
    $$files(../libpvrt/*.h) \
    Sources/tabletapplication.cpp

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
    ../2dsg/gfxbackends/Shaders.h \
    Headers/tabletapplication.h

DEFINES += USE_FILE32API LUAU_ENABLE_CODEGEN

LIBS += -lpthread


# modified from http://source-android.frandroid.com/external/freetype/Android.mk
# also ftmodule.h should be configured like http://source-android.frandroid.com/external/freetype/include/freetype/config/ftmodule.h
DEFINES += FT2_BUILD_LIBRARY FT_CONFIG_OPTION_SYSTEM_ZLIB
DEFINES += DARWIN_NO_CARBON

#FREETYPE_VER=2.4.12
FREETYPE_VER=2.7.1

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

    
