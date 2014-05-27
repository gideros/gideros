#-------------------------------------------------
#
# Project created by QtCreator 2011-05-12T11:34:33
#
#-------------------------------------------------

QT       += core gui xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#macx {
#CONFIG += release x86 x86_64
#}

win32 {
RC_FILE = ui.rc
}

macx {
ICON = "gideros.icns"
}

win32 {
TARGET = "GiderosStudio"
}

macx {
TARGET = "Gideros Studio"
}

TEMPLATE = app

INCLUDEPATH +=	\
    "../iconlibrary" \
    "../libpreviewwidget"\
    "../libnetwork"\
    "../libplatform"\
    "../lua/src"\
    "../libpvrt" \
    "../libgid/include" \
    "../libgfile" \
    "../libgideros" \
    "../liblicensemanager"

SOURCES += \
    main.cpp\
    mainwindow.cpp\
    librarywidget.cpp\
    giderosnetworkclient2.cpp\
    finddialog.cpp\
    replacedialog.cpp\
    findinfilesdialog.cpp\
    newprojectdialog.cpp\
    fileassociationsdialog.cpp\
    fileassociationeditdialog.cpp\
    textedit.cpp\
    playersettingsdialog.cpp\
    gotolinedialog.cpp\
    savechangesdialog.cpp\
    librarytreewidget.cpp\
    codedependenciesdialog.cpp\
    addnewfiledialog.cpp\
    "../libpreviewwidget/previewwidget.cpp"\
    "../iconlibrary/iconlibrary.cpp"\
    "../libnetwork/bytebuffer.cpp" \
    exportprojectdialog.cpp \
    aboutdialog.cpp \
    projectpropertiesdialog.cpp \
    startpagewidget2.cpp \
    recentprojectswidget.cpp \
    exampleprojectswidget.cpp \
    projectproperties.cpp \
    mdiarea.cpp \
    mdisubwindow.cpp \
    dependencygraph.cpp \
    countly.cpp

SOURCES += $$files(../libpvrt/*.cpp)

SOURCES += "../libplatform/platformutil.cpp"\
			"../libplatform/md5.c"

HEADERS += ../liblicensemanager/licensemanager.h
SOURCES +=	../liblicensemanager/licensemanager.cpp \
			../liblicensemanager/uid.cpp

HEADERS  += \
    mainwindow.h\
    librarywidget.h\
    giderosnetworkclient2.h\
    finddialog.h\
    replacedialog.h\
    findinfilesdialog.h\
    newprojectdialog.h\
    fileassociationsdialog.h\
    fileassociationeditdialog.h\
    textedit.h\
    playersettingsdialog.h\
    gotolinedialog.h\
    savechangesdialog.h\
    librarytreewidget.h\
    codedependenciesdialog.h\
    addnewfiledialog.h\
    "../libpreviewwidget/previewwidget.h" \
    exportprojectdialog.h \
    aboutdialog.h \
    projectpropertiesdialog.h \
    projectproperties.h \
    startpagewidget2.h \
    recentprojectswidget.h \
    exampleprojectswidget.h \
    mdiarea.h \
    mdisubwindow.h \
    dependencygraph.h \
    countly.h

FORMS    += mainwindow.ui \
    savechangesdialog.ui \
    replacedialog.ui \
    playersettingsdialog.ui \
    newprojectdialog.ui \
    librarywidget.ui \
    gotolinedialog.ui \
    findreplacedialog.ui \
    findinfilesdialog.ui \
    finddialog.ui \
    fileassociationsdialog.ui \
    fileassociationeditdialog.ui \
    codedependenciesdialog.ui \
    addnewfiledialog.ui \
    exportprojectdialog.ui \
    aboutdialog.ui \
    projectpropertiesdialog.ui \
    startpagewidget2.ui

win32 {
	LIBS += -liphlpapi
}

macx {
	LIBS += -framework CoreFoundation
	LIBS += -framework IOKit
    INCLUDEPATH += /Qt/Qt5.3.0/5.3/clang_64/include
    LIBS += -L/Qt/Qt5.3.0/5.3/clang_64/lib
}

LIBS += -lqscintilla2

RESOURCES += \
    ui.qrc


