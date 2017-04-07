#-------------------------------------------------
#
# Project created by QtCreator 2011-03-19T10:14:52
#
#-------------------------------------------------

QT       += core gui
CONFIG   += silent

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

unix:!macx {
TARGET = "GiderosFontCreator"
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
#FREETYPE_VER=2.4.12
FREETYPE_VER=2.7.1

DEFINES += FT2_BUILD_LIBRARY
DEFINES += DARWIN_NO_CARBON

INCLUDEPATH += ../libgid/include
INCLUDEPATH += ../libgid/external/freetype-$${FREETYPE_VER}/include
INCLUDEPATH += ../libgid/external/freetype-$${FREETYPE_VER}/src

SOURCES += \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftbbox.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftbitmap.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftglyph.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftlcdfil.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftstroke.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftbase.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftsystem.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftinit.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftgasp.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/raster/raster.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/sfnt/sfnt.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/smooth/smooth.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/autofit/autofit.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/truetype/truetype.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/cff/cff.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/gzip/ftgzip.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/psnames/psnames.c \
    ../libgid/external/freetype-$${FREETYPE_VER}/src/pshinter/pshinter.c
#2.4.12    ../libgid/external/freetype-$${FREETYPE_VER}/src/base/ftxf86.c
