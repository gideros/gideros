#-------------------------------------------------
#
# Project created by QtCreator 2011-03-07T13:50:06
#
#-------------------------------------------------

QT       += core gui xml network

TARGET = "Gideros Project Manager"
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        iconlibrary.cpp \
    codedependenciesdialog.cpp \
    librarytreewidget.cpp \
    librarywidget.cpp \
    addnewfiledialog.cpp \
    newprojectdialog.cpp \
	playersettingsdialog.cpp \
    bytebuffer.cpp \
    giderosnetworkclient.cpp \
    fileassociationsdialog.cpp \
    fileassociationeditdialog.cpp

HEADERS  += mainwindow.h\
        iconlibrary.h \
    codedependenciesdialog.h \
    librarywidget.h \
    librarytreewidget.h \
    addnewfiledialog.h \
    newprojectdialog.h \
	playersettingsdialog.h \
    bytebuffer.h \
    giderosnetworkclient.h \
    fileassociationsdialog.h \
    fileassociationeditdialog.h

FORMS    += mainwindow.ui \
    codedependenciesdialog.ui \
    librarywidget.ui \
    addnewfiledialog.ui \
    newprojectdialog.ui \
	playersettingsdialog.ui \
    fileassociationsdialog.ui \
    fileassociationeditdialog.ui

RESOURCES += projectmanager.qrc
