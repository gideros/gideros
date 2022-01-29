QT -= gui
QT -= core

TEMPLATE = lib

INCLUDEPATH += ../../../../Sdk/include
INCLUDEPATH += ../../../../Sdk/include/gideros
INCLUDEPATH += ../../../../2dsg
INCLUDEPATH += ../../../../2dsg/gfxbackends
INCLUDEPATH += ../../../../libgideros
INCLUDEPATH += ../../../../libgid/include
INCLUDEPATH += ../../../../luabinding
#INCLUDEPATH += ../../../../lua/src
INCLUDEPATH += ../Common/imgui_src
INCLUDEPATH += ../Common

SOURCES += \
        ../Common/imgui_src/imgui.cpp \
        ../Common/imgui_src/imgui_demo.cpp \
        ../Common/imgui_src/imgui_draw.cpp \
        ../Common/imgui_src/imgui_widgets.cpp \
        ../Common/imgui_src/imgui_tables.cpp \
        ../Common/imgui_user.cpp \
        ../Common/TextEditor.cpp \
	   	../../../../2dsg/Matrices.cpp \
	   	../../../../2dsg/mouseevent.cpp \
	   	../../../../2dsg/touchevent.cpp \
	   	../../../../2dsg/keyboardevent.cpp \
        ../Common/imgui_bindings.cpp 

HEADERS += \
    ../Common/imgui_user.h \
    ../Common/TextEditor.h \
    ../Common/imgui_src/imconfig.h \
    ../Common/imgui_src/imgui.h \
    ../Common/imgui_src/imgui_internal.h \
    ../Common/imgui_src/imstb_rectpack.h \
    ../Common/imgui_src/imstb_textedit.h \
    ../Common/imgui_src/imstb_truetype.h


LIBS += -L"../../../../Sdk/lib/desktop" -llua -lgideros -lgid -lgvfs


CONFIG(release, debug|release) {
	TARGET = imgui
} else {
	TARGET = imgui_beta
    
    HEADERS += \
        ../../../../luabinding/binder.h \
        ../../../../luabinding/stackchecker.h
        
	SOURCES += \
        ../../../../luabinding/binder.cpp \
        ../../../../luabinding/stackchecker.cpp
}

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgvfs.1.dylib "@executable_path/../Frameworks/libgvfs.1.dylib" $(TARGET);
}
