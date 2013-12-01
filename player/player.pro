#-------------------------------------------------
#
# Project created by QtCreator 2011-03-09T08:32:11
#
#-------------------------------------------------

QT       += core gui opengl network

win32 {
RC_FILE = player.rc
}

macx {
ICON = player.icns
}

INCLUDEPATH += \
    ../2dsg\
    ../libplatform\
    ../libsound\
    ../libnetwork\
    ../luabinding\
    ../lua/src\
    ../libpvrt\
    ../libgvfs\
    ../libgid/include\
    ../libgid/include/qt\
    ../libgideros\
    ../libpystring\
    "../external/glu"

macx {
INCLUDEPATH += "/opt/local/include"
INCLUDEPATH += "/opt/local/include/freetype2"
}

macx {
TARGET = "Gideros Player"
}

win32 {
TARGET = GiderosPlayer
}

TEMPLATE = app

INCLUDEPATH += ../external/Box2D_v2.3.0/Box2D
SOURCES += $$files(../external/Box2D_v2.3.0/Box2D/Box2D/Common/*.cpp)
SOURCES += $$files(../external/Box2D_v2.3.0/Box2D/Box2D/Collision/*.cpp)
SOURCES += $$files(../external/Box2D_v2.3.0/Box2D/Box2D/Collision/Shapes/*.cpp)
SOURCES += $$files(../external/Box2D_v2.3.0/Box2D/Box2D/Dynamics/*.cpp)
SOURCES += $$files(../external/Box2D_v2.3.0/Box2D/Box2D/Dynamics/Contacts/*.cpp)
SOURCES += $$files(../external/Box2D_v2.3.0/Box2D/Box2D/Dynamics/Joints/*.cpp)
SOURCES += $$files(../external/Box2D_v2.3.0/Box2D/Box2D/Rope/*.cpp)

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    errordialog.cpp \
    glcanvas.cpp \
    projectpropertiesdialog.cpp

SOURCES += $$files(../luabinding/*.cpp)	../luabinding/tlsf.c

SOURCES += $$files(../libnetwork/*.cpp)

SOURCES += \
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
    ../libplatform/gtimer.cpp

SOURCES += $$files(../2dsg/*.cpp)

SOURCES += $$files(../libpvrt/*.cpp)

SOURCES += $$files(../external/glu/libtess/*.c)

DEFINES += USE_FILE32API
SOURCES += \
    "../external/minizip-1.1/source/ioapi.c" \
    "../external/minizip-1.1/source/unzip.c"

INCLUDEPATH += "../external/minizip-1.1/source"

win32 {
INCLUDEPATH += "../libgid/external/zlib-1.2.8"
LIBS += -L"../libgid/external/zlib-1.2.8/build/mingw" -lzlib
INCLUDEPATH += "../libgid/external/glew-1.10.0/include"
LIBS += -L"../libgid/external/glew-1.10.0/lib/mingw" -lglew32
}

LIBS += -lpthread

macx {
LIBS += -framework OpenAL
LIBS += -framework OpenGL
LIBS += -framework CoreFoundation
LIBS += -lz
LIBS += -L"/opt/local/lib"
}

win32 {
LIBS += -lwsock32
LIBS += -liphlpapi
}

win32 {
LIBS += -L"../libgid/release" -lgid
LIBS += -L"../libgvfs/release" -lgvfs
LIBS += -L"../lua/release" -llua
LIBS += -L"../libgideros/release" -lgideros
LIBS += -L"../libpystring/release" -lpystring
}

macx {
LIBS += -L"../libgid" -lgid
LIBS += -L"../libgvfs" -lgvfs
LIBS += -L"../lua" -llua
LIBS += -L"../libgideros" -lgideros
LIBS += -L"../libpystring" -lpystring
LIBS += -L"/opt/local/lib" -lGLEW -lfreetype
}


HEADERS += \
    mainwindow.h \
    errordialog.h \
    glcanvas.h \
    projectpropertiesdialog.h

HEADERS += $$files(../libsound/*.h)
HEADERS += $$files(../2dsg/*.h)
HEADERS += $$files(../luabinding/*.h)
HEADERS += $$files(../libnetwork/*.h)
HEADERS += $$files(../libplatform/*.h)

SOURCES += $$files(../libpvrt/*.h)


FORMS += \
    mainwindow.ui\
    errordialog.ui \
    projectpropertiesdialog.ui

win32 {
    INCLUDEPATH += ../libgid/external/freetype-2.4.12/include
    LIBS += -L"../libgid/external/freetype-2.4.12/build/mingw" -lfreetype
}

macx {
QMAKE_LFLAGS += -pagezero_size 10000 -image_base 100000000
}
