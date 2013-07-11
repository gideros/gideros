#-------------------------------------------------
#
# Project created by QtCreator 2011-03-31T10:14:06
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = animationeditor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    timelinewidget.cpp \
    canvaswidget.cpp \
    document.cpp \
    controller.cpp

HEADERS  += mainwindow.h \
    timelinewidget.h \
    canvaswidget.h \
    document.h \
    controller.h

FORMS    += mainwindow.ui
