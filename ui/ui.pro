#-------------------------------------------------
#
# Project created by QtCreator 2011-05-12T11:34:33
#
#-------------------------------------------------

QT       += core gui xml network websockets
CONFIG   += silent

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
    outlinewidget.cpp\
    giderosnetworkclient2.cpp\
    finddialog.cpp\
    replacedialog.cpp\
    findinfilesdialog.cpp\
    newprojectdialog.cpp\
    fileassociationsdialog.cpp\
    fileassociationeditdialog.cpp\
    pluginselector.cpp \
    plugineditor.cpp \
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
    dependencygraph.cpp \
    addons.cpp \
    qtutils.cpp \
    preferencesdialog.cpp

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
    outlinewidget.h\
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
    pluginselector.h \
    plugineditor.h \
    dependencygraph.h \
    addons.h \
    qtutils.h \
    preferencesdialog.h \
    settingskeys.h

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
    pluginselector.ui \
    plugineditor.ui \
    startpagewidget2.ui \
    preferencesdialog.ui


DEFINES += DESKTOP_TOOLS 
INCLUDEPATH += ../lua/src
SOURCES += \
	../lua/src/lapi.c \
	../lua/src/lauxlib.c \
	../lua/src/lcode.c \
	../lua/src/ldebug.c \
	../lua/src/ldo.c \
	../lua/src/ldump.c \
	../lua/src/lfunc.c \
	../lua/src/llex.c \
	../lua/src/lmem.c \
	../lua/src/lobject.c \
	../lua/src/lopcodes.c \
	../lua/src/lparser.c \
	../lua/src/lstate.c \
	../lua/src/lstring.c \
	../lua/src/ltable.c \
	../lua/src/ltm.c \
	../lua/src/lundump.c \
	../lua/src/lvm.c \
	../lua/src/lzio.c \
	../lua/src/lgc.c \
	../lua/src/linit.c \
	../lua/src/lbaselib.c \
	../lua/src/ldblib.c \
	../lua/src/liolib.c \
	../lua/src/lmathlib.c \
	../lua/src/loslib.c \
	../lua/src/ltablib.c \
	../lua/src/lstrlib.c \
	../lua/src/lutf8lib.c \
	../lua/src/lint64.c \
	../lua/src/loadlib.c

SOURCES += \
    ../plugins/json/source/strbuf.c \
    ../plugins/json/source/fpconv.c \
    ../plugins/json/source/lua_cjson.c

HEADERS += \
    ../plugins/json/source/strbuf.h \
    ../plugins/json/source/fpconv.h

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
	CONFIG(debug, debug|release) {
	  LIBS += -lqscintilla2_qt5d
	}
	else
	{
   		LIBS += -lqscintilla2
   	}
}

macx {
   LIBS += -lqscintilla2
}

unix:!macx {
   LIBS += -lqt5scintilla2
}

RESOURCES += \
    ui.qrc
