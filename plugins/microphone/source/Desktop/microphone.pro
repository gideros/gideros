QT -= core gui

TARGET = microphone
TEMPLATE = lib

INCLUDEPATH += ../../../../Sdk/include

SOURCES += \
    gmicrophone-openal.cpp \
    gsoundencoder-wav.cpp \
    gmicrophonebinder.cpp

HEADERS += \
    gmicrophone.h \
    gsoundencoder.h

LIBS += -L"../../../../Sdk/lib/desktop" -llua -lgid -lgideros -lgvfs

LIBS += -lpthread

win32 {
DEFINES += OPENAL_SUBDIR_AL
LIBS += -L"../../../../Sdk/lib/desktop" -lOpenAL32
}

macx {
DEFINES += OPENAL_SUBDIR_OPENAL
LIBS += -framework OpenAL
}

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgvfs.1.dylib "@executable_path/../Frameworks/libgvfs.1.dylib" $(TARGET);
}
