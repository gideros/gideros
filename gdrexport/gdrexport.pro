#-------------------------------------------------
#
# Project created by QtCreator 2013-05-16T12:38:10
#
#-------------------------------------------------

QT       += core network xml

QT       -= gui

TARGET = gdrexport
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

INCLUDEPATH += ../ui ../libnetwork

HEADERS += \
    ../ui/projectproperties.h \
    ../ui/dependencygraph.h

SOURCES += \
    ../ui/projectproperties.cpp \
    ../ui/dependencygraph.cpp \
    ../libnetwork/bytebuffer.cpp

macx {
	LIBS += -framework CoreFoundation
	LIBS += -framework IOKit
}
