#-------------------------------------------------
#
# Project created by QtCreator 2011-12-07T15:49:38
#
#-------------------------------------------------

QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32 {
RC_FILE = licensemanager.rc
}

macx {
ICON = "licensemanager.icns"
}

win32 {
TARGET = "GiderosLicenseManager"
}

macx {
TARGET = "Gideros License Manager"
}

TEMPLATE = app


INCLUDEPATH += ../liblicensemanager

SOURCES += main.cpp\
		mainwindow.cpp \
    activatewidget.cpp \
    deactivatewidget.cpp \
	../liblicensemanager/uid.cpp \
    deactivatealldialog.cpp \
    ../liblicensemanager/licensemanager.cpp

HEADERS  += mainwindow.h \
    activatewidget.h \
    deactivatewidget.h \
	../liblicensemanager/uid.h \
    deactivatealldialog.h \
    ../liblicensemanager/licensemanager.h

FORMS    += mainwindow.ui \
    activatewidget.ui \
    deactivatewidget.ui \
    deactivatealldialog.ui

RESOURCES += \
    licensemanager.qrc


macx {
	LIBS += -framework CoreFoundation
	LIBS += -framework IOKit
}
















