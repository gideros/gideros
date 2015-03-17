QT -= core gui

TARGET = clipper
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include

SOURCES += \
    clipper.cpp \
    clipperbinder.cpp

HEADERS += \
    clipper.hpp

LIBS += -L"../../../Sdk/lib/desktop" -llua -lgid -lgideros

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
}
