#-------------------------------------------------
#
# Project created by QtCreator 2012-05-04T00:40:14
#
#-------------------------------------------------

QT       += core network xml

QT       -= gui

TARGET = gdrdeamon
#CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    application.cpp \
    ../ui/giderosnetworkclient2.cpp \
    ../libnetwork/bytebuffer.cpp \
    qtsinglecoreapplication.cpp \
    qtlockedfile.cpp \
    qtlocalpeer.cpp \
    ../ui/projectproperties.cpp \
    ../libplatform/md5.c \
    ../ui/dependencygraph.cpp

HEADERS += \
    application.h \
    ../ui/giderosnetworkclient2.h \
    qtsinglecoreapplication.h \
    qtlockedfile.h \
    QtLockedFile \
    qtlocalpeer.h \
    QtSingleCoreApplication \
    ../ui/projectproperties.h \
    ../ui/dependencygraph.h

INCLUDEPATH += . ../libnetwork ../ui ../libplatform

win32 {
    SOURCES += qtlockedfile_win.cpp
}

macx {
    SOURCES += qtlockedfile_unix.cpp
}


