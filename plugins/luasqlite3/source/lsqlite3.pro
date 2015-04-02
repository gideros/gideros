QT -= core gui

TARGET = lsqlite3
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include

SOURCES += \
	sqlite3.c \
    lsqlite3.c \
	lsqlite3_stub.cpp

HEADERS +=

LIBS += -L"../../../Sdk/lib/desktop" -lgvfs -llua

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgvfs.1.dylib "@executable_path/../Frameworks/libgvfs.1.dylib" $(TARGET);
}
