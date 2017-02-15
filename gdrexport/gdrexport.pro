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
CONFIG   += silent

TEMPLATE = app


SOURCES += main.cpp GAppFormat.cpp Utilities.cpp WinRTExport.cpp MacOSXExport.cpp \
	ExportCommon.cpp ExportBuiltin.cpp ExportXml.cpp filedownloader.cpp ExportLua.cpp
HEADERS += GAppFormat.h Utilities.h WinRTExport.h MacOSXExport.h ExportCommon.h \
	ExportBuiltin.h ExportXml.h filedownloader.h ExportLua.h

INCLUDEPATH += ../ui ../libnetwork ../2dsg 

HEADERS += \
    ../ui/projectproperties.h \
    ../2dsg/orientation.h \
    ../ui/dependencygraph.h

SOURCES += \
    ../ui/projectproperties.cpp \
    ../ui/dependencygraph.cpp \
    ../libnetwork/bytebuffer.cpp

INCLUDEPATH += ../lua514u/src
SOURCES += \
	../lua514u/src/lapi.c \
	../lua514u/src/lauxlib.c \
	../lua514u/src/lcode.c \
	../lua514u/src/ldebug.c \
	../lua514u/src/ldo.c \
	../lua514u/src/ldump.c \
	../lua514u/src/lfunc.c \
	../lua514u/src/llex.c \
	../lua514u/src/lmem.c \
	../lua514u/src/lobject.c \
	../lua514u/src/lopcodes.c \
	../lua514u/src/lparser.c \
	../lua514u/src/lstate.c \
	../lua514u/src/lstring.c \
	../lua514u/src/ltable.c \
	../lua514u/src/ltm.c \
	../lua514u/src/lundump.c \
	../lua514u/src/lvm.c \
	../lua514u/src/lzio.c \
	../lua514u/src/lgc.c \
	../lua514u/src/linit.c \
	../lua514u/src/lbaselib.c \
	../lua514u/src/ldblib.c \
	../lua514u/src/liolib.c \
	../lua514u/src/lmathlib.c \
	../lua514u/src/loslib.c \
	../lua514u/src/ltablib.c \
	../lua514u/src/lstrlib.c \
	../lua514u/src/loadlib.c

INCLUDEPATH += ./luaext
SOURCES += \
	./luaext/lfs.c

LIBS += -lz

macx {
	LIBS += -framework CoreFoundation
	LIBS += -framework IOKit
}
