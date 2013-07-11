QT -= core gui

TARGET = lfs
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include

SOURCES += \
    lfs.c \
	lfs_stub.cpp

HEADERS +=

LIBS += -L"../../../Sdk/lib/desktop" -llua -lgid

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
}
