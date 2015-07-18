QT -= core gui

TARGET = lpeg
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include

SOURCES += \
	lpvm.c lpcap.c lptree.c lpcode.c lpprint.c \
	lpeg_stub.cpp

HEADERS +=

LIBS += -L"../../../Sdk/lib/desktop" -llua

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
}
