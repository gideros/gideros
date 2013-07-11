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

INCLUDEPATH += ../liblicensemanager ../ui ../libnetwork

HEADERS += \
    ../liblicensemanager/licensemanager.h \
    ../ui/projectproperties.h \
    ../ui/dependencygraph.h

SOURCES += \
    ../liblicensemanager/licensemanager.cpp \
    ../liblicensemanager/uid.cpp \
    ../ui/projectproperties.cpp \
    ../ui/dependencygraph.cpp \
    ../libnetwork/bytebuffer.cpp

macx {
	LIBS += -framework CoreFoundation
	LIBS += -framework IOKit
}

macx {
QMAKE_POST_LINK += install_name_tool -change "/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/lib/QtCore.framework/Versions/4/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change "/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/lib/QtNetwork.framework/Versions/4/QtNetwork" "@executable_path/../Frameworks/QtNetwork.framework/Versions/4/QtNetwork" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change "/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/lib/QtXml.framework/Versions/4/QtXml" "@executable_path/../Frameworks/QtXml.framework/Versions/4/QtXml" $(TARGET);
}
