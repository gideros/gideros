#-------------------------------------------------
#
# Project created by QtCreator 2013-05-16T12:38:10
#
#-------------------------------------------------

QT       += core network xml

#QT       -= gui

TARGET = gdrexport
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp GAppFormat.cpp Utilities.cpp WinRTExport.cpp MacOSXExport.cpp \
	ExportCommon.cpp ExportBuiltin.cpp ExportXml.cpp filedownloader.cpp
HEADERS += GAppFormat.h Utilities.h WinRTExport.h MacOSXExport.h ExportCommon.h \
	ExportBuiltin.h ExportXml.h filedownloader.h

INCLUDEPATH += ../ui ../libnetwork ../2dsg

HEADERS += \
    ../ui/projectproperties.h \
    ../2dsg/orientation.h \
    ../ui/dependencygraph.h

SOURCES += \
    ../ui/projectproperties.cpp \
    ../ui/dependencygraph.cpp \
    ../libnetwork/bytebuffer.cpp

LIBS += -lz

macx {
	LIBS += -framework CoreFoundation
	LIBS += -framework IOKit
}
