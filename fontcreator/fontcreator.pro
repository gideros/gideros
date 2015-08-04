#-------------------------------------------------
#
# Project created by QtCreator 2011-03-19T10:14:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#macx {
#CONFIG += release x86 x86_64
#}

win32 {
TARGET = "GiderosFontCreator"
}

macx {
TARGET = "Gideros Font Creator"
}

win32 {
RC_FILE = fontcreator.rc
}

macx {
ICON = fontcreator.icns
}



TEMPLATE = app


SOURCES += main.cpp\
		mainwindow.cpp\
		fontcanvas.cpp\
	optionswidget.cpp \
    texturepacker.cpp \
    MaxRectsBinPack.cpp

HEADERS  += mainwindow.h\
			fontcanvas.h\
	optionswidget.h \
    texturepacker.h \
    MaxRectsBinPack.h \
    Rect.h


FORMS    += mainwindow.ui\
	optionswidget.ui


# modified from http://source-android.frandroid.com/external/freetype/Android.mk
# also ftmodule.h should be configured like http://source-android.frandroid.com/external/freetype/include/freetype/config/ftmodule.h
DEFINES += FT2_BUILD_LIBRARY
DEFINES += DARWIN_NO_CARBON

INCLUDEPATH += ../libgid/external/freetype-2.4.12/include
INCLUDEPATH += ../libgid/external/freetype-2.4.12/src

SOURCES += \
    ../libgid/external/freetype-2.4.12/src/base/ftbbox.c \
    ../libgid/external/freetype-2.4.12/src/base/ftbitmap.c \
    ../libgid/external/freetype-2.4.12/src/base/ftglyph.c \
    ../libgid/external/freetype-2.4.12/src/base/ftlcdfil.c \
    ../libgid/external/freetype-2.4.12/src/base/ftstroke.c \
    ../libgid/external/freetype-2.4.12/src/base/ftxf86.c \
    ../libgid/external/freetype-2.4.12/src/base/ftbase.c \
    ../libgid/external/freetype-2.4.12/src/base/ftsystem.c \
    ../libgid/external/freetype-2.4.12/src/base/ftinit.c \
    ../libgid/external/freetype-2.4.12/src/base/ftgasp.c \
    ../libgid/external/freetype-2.4.12/src/raster/raster.c \
    ../libgid/external/freetype-2.4.12/src/sfnt/sfnt.c \
    ../libgid/external/freetype-2.4.12/src/smooth/smooth.c \
    ../libgid/external/freetype-2.4.12/src/autofit/autofit.c \
    ../libgid/external/freetype-2.4.12/src/truetype/truetype.c \
    ../libgid/external/freetype-2.4.12/src/cff/cff.c \
    ../libgid/external/freetype-2.4.12/src/psnames/psnames.c \
    ../libgid/external/freetype-2.4.12/src/pshinter/pshinter.c
