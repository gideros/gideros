QT += core gui opengl network
CONFIG += console

win32{
    RC_FILE = other_files/player.rc

    TARGET = GiderosPlayer

    INCLUDEPATH += \
        "../libgid/external/zlib-1.2.8"\
        "../libgid/external/glew-1.10.0/include"\
        ../libgid/external/freetype-2.4.12/include

    LIBS += \
        -L"../libgid/external/zlib-1.2.8/build/mingw48_32" -lzlibx\
        -L"../libgid/external/glew-1.10.0/lib/mingw48_32" -lglew32\
        -L"../libgid/external/freetype-2.4.12/build/mingw48_32" -lfreetype\
        -lwsock32\
        -liphlpapi\
        -L"../libgid/release" -lgid\
        -L"../libgvfs/release" -lgvfs\
        -L"../lua/release" -llua\
        -L"../libgideros/release" -lgideros\
        -L"../libpystring/release" -lpystring
}

macx {
    ICON = other_files/player.icns

    INCLUDEPATH += \
        "/usr/local/include"\
        "/usr/local/include/freetype2"

    TARGET = "Gideros Player"

    LIBS += \
        -framework OpenAL\
        -framework OpenGL\
        -framework CoreFoundation\
        -lz\
        -L"/usr/local/lib"\
        -L"../libgid" -lgid\
        -L"../libgvfs" -lgvfs\
        -L"../lua" -llua\
        -L"../libgideros" -lgideros\
        -L"../libpystring" -lpystring\
        -L"/usr/local/lib" -lGLEW -lfreetype

    QMAKE_LFLAGS += -pagezero_size 10000 -image_base 100000000
}

TEMPLATE = app

INCLUDEPATH += \
    headers \
    sources \
    forms \
    ../2dsg \
    ../libplatform \
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
    sources/main.cpp \
    sources/mainwindow.cpp \
    sources/errordialog.cpp \
    sources/glcanvas.cpp \
    sources/settingsdialog.cpp \
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
    ../libplatform/md5.c \
    ../libplatform/platform.cpp \
    ../libplatform/platformutil.cpp \
    ../libplatform/utf8.c \
    ../libplatform/openurl-qt.cpp \
    ../libplatform/vibrate-empty.cpp \
    ../libplatform/locale-qt.cpp \
    ../libplatform/keepawake-empty.cpp \
    ../libplatform/deviceinfo-qt.cpp \
    ../libplatform/drawinfo.cpp \
    ../libplatform/fps-generic.cpp \
    ../libplatform/exit-generic.cpp \
#    ../libplatform/windowsize-generic.cpp \
    ../libplatform/gtimer.cpp \
    $$files(../2dsg/*.cpp) \
    $$files(../libpvrt/*.cpp) \
    $$files(../external/glu/libtess/*.c) \
    "../external/minizip-1.1/source/ioapi.c" \
    "../external/minizip-1.1/source/unzip.c" \
    $$files(../libpvrt/*.h)

FORMS += \
    forms\mainwindow.ui\
    forms\errordialog.ui \
    forms\settingsdialog.ui

HEADERS += \
    headers\mainwindow.h \
    headers\errordialog.h \
    headers\glcanvas.h \
    headers\settingsdialog.h\
    $$files(../libsound/*.h)\
    $$files(../2dsg/*.h)\
    $$files(../luabinding/*.h)\
    $$files(../libnetwork/*.h)\
    $$files(../libplatform/*.h)\

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
