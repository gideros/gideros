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
QT += macextras
}

win32 {
TARGET = "GiderosStudio"
}

macx {
TARGET = "Gideros Studio"
}

unix:!macx {
TARGET = "GiderosStudio"
QMAKE_CXXFLAGS += -std=gnu++0x
}

TEMPLATE = app

INCLUDEPATH +=	\
    "../iconlibrary" \
    "../libpreviewwidget"\
    "../libnetwork"\
    "../libgid/src/qt"\
    "../lua/src"\
    "../libpvrt" \
    "../libgid/include" \
    "../libgfile" \
    "../libgideros"

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
    pluginschooser.cpp \
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
    exportprogress.cpp \
    aboutdialog.cpp \
    projectpropertiesdialog.cpp \
    startpagewidget2.cpp \
    recentprojectswidget.cpp \
    exampleprojectswidget.cpp \
    projectproperties.cpp \
    propertyeditingtable.cpp \
    mdiarea.cpp \
    mdisubwindow.cpp \
    dependencygraph.cpp

SOURCES += $$files(../libpvrt/*.cpp)

SOURCES += "../libgid/src/platformutil.cpp"\
			"../libgid/src/aes.c" \
			"../libgid/src/md5.c"

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
    exportprogress.h \
    aboutdialog.h \
    projectpropertiesdialog.h \
    projectproperties.h \
    propertyeditingtable.h \
    startpagewidget2.h \
    recentprojectswidget.h \
    exampleprojectswidget.h \
    mdiarea.h \
    mdisubwindow.h \
    pluginschooser.h \
    dependencygraph.h

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
    exportprogress.ui \
    aboutdialog.ui \
    projectpropertiesdialog.ui \
    pluginschooser.ui \
    startpagewidget2.ui

win32 {
	LIBS += -liphlpapi
}

macx {
	LIBS += -framework CoreFoundation
	LIBS += -framework IOKit
    INCLUDEPATH += $$[QT_INSTALL_HEADERS]
    LIBS += -L$$[QT_INSTALL_LIBS]
	CONFIG += c++11
}

win32 {
   LIBS += -lqscintilla2
}

macx {
   LIBS += -lqscintilla2
}

unix:!macx {
   LIBS += -lqt5scintilla2
}

RESOURCES += \
    ui.qrc
