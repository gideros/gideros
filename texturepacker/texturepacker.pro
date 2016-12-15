#-------------------------------------------------
#
# Project created by QtCreator 2011-03-14T16:27:11
#
#-------------------------------------------------

QT       += core gui xml
CONFIG   += silent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#macx {
#CONFIG += release x86 x86_64
#}


macx {
TARGET = "Gideros Texture Packer"
}

win32 {
TARGET = "GiderosTexturePacker"
}

unix:!macx {
TARGET = "GiderosTexturePacker"
}

win32 {
RC_FILE = texturepacker.rc
}

macx {
ICON = texturepacker.icns
}


TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    texturepacker.cpp \
    optionswidget.cpp \
    newprojectdialog.cpp \
    librarytreewidget.cpp \
    canvas.cpp \
    iconlibrary.cpp \
	projectproperties.cpp \
	MaxRectsBinPack.cpp


HEADERS  += mainwindow.h \
    texturepacker.h \
    optionswidget.h \
    newprojectdialog.h \
    librarytreewidget.h \
    canvas.h \
    iconlibrary.h \
	projectproperties.h \
	MaxRectsBinPack.h \
	Rect.h

FORMS    += mainwindow.ui \
    newprojectdialog.ui \
    optionswidget.ui

#RESOURCES += mainwindow.qrc



