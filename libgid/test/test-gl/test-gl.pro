#-------------------------------------------------
#
# Project created by QtCreator 2012-11-16T22:33:01
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = test-gl
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui


INCLUDEPATH += ../../include ../../include/qt

LIBS += -L"C:/myprojects/gideros/libgid/debug" -lgid

!greaterThan(QT_MAJOR_VERSION, 4) {
INCLUDEPATH += ../../../libgid/external/glew-1.9.0/include
LIBS += -L"../../../libgid/external/glew-1.9.0/lib/mingw" -lglew32
}
