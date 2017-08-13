QT -= gui
#QT -= core

TARGET = bump
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include

SOURCES += \
	bump.cpp

HEADERS +=

LIBS += -L"../../../Sdk/lib/desktop" -llua -lgideros

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
}
