#-------------------------------------------------
#
# Project created by QtCreator 2011-05-12T11:34:33
#
#-------------------------------------------------

QT       += core gui xml network websockets
CONFIG   += silent c++17

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

USE_SCINTILLAEDIT=y

LUA_ENGINE=$$(LUA_ENGINE)
LUA_ENGINE=luau
isEmpty(LUA_ENGINE): LUA_ENGINE=lua
equals(LUA_ENGINE,luau): LUA_INCLUDE=../luau/VM/include ../luau/VM/src ../luau/Ast/include ../luau/Compiler/include ../luau/Analysis/include
equals(LUA_ENGINE,lua): LUA_INCLUDE=../lua/src

DEFINES+= LUAU_ENABLE_ASSERT
CONFIG(debug, debug|release){
    _CONFIG_ = debug
} else {
    _CONFIG_ = release
}

defineReplace(expand) {
    names = $$1
    prefix=$$2
    suffix=$$3
    expanded =

    for(name, names) {
        expanded+= $${prefix}$${name}$${suffix}
    }
    return ($$expanded)
}

equals(LUA_ENGINE,luau): LUA_SOURCES =\
         $$expand(lapi laux lbaselib lbitlib lbuiltins lcorolib ldblib ldebug ldo lfunc lgc lgcdebug linit lint64lib liolib lmathlib lmem lnumprint lobject loslib lperf lstate lstring lstrlib \
         ltable ltablib ltm ludata lutf8lib lvmexecute lvmload lvmutils,../luau/VM/src/,.cpp) \
         $$expand(Builtins BytecodeBuilder ConstantFolding Compiler lcode PseudoCode TableShape ValueTracking,../luau/Compiler/src/,.cpp) \
         $$expand(AstQuery Autocomplete BuiltinDefinitions Config EmbeddedBuiltinDefinitions Error Frontend IostreamHelpers JsonEncoder Linter LValue Module Quantify RequireTracer \
         Scope Substitution Symbol ToDot TopoSortStatements Tostring Transpiler TxnLog TypeAttach TypedAllocator TypeInfer TypePack TypeUtils TypeVar Unifiable Unifier,../luau/Analysis/src/,.cpp) \
         $$expand(Ast Confusables Lexer Location Parser StringUtils TimeTrace,../luau/Ast/src/,.cpp)
equals(LUA_ENGINE,lua): LUA_SOURCES =\
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

         
#macx {
#CONFIG += release x86 x86_64
#}

win32 {
RC_FILE = ui.rc
}

macx {
ICON = "gideros.icns"
#QT += macextras
}

win32 {
TARGET = "GiderosStudio"
}

debug_in_place{
DESTDIR = ../Build.Win
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
    $$LUA_INCLUDE \
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
    preferencesdialog.cpp \
    profilerreport.cpp \
    wordhighlighter.cpp

equals(USE_SCINTILLAEDIT,y): SOURCES+= textedit.cpp
equals(USE_SCINTILLAEDIT,y): DEFINES+= USE_SCINTILLAEDIT
equals(USE_SCINTILLAEDIT,n): SOURCES+= textedit_qs.cpp

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
    settingskeys.h \
    profilerreport.h \
    wordhighlighter.h

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
SOURCES += $$LUA_SOURCES

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
}


win32 {
	equals(USE_SCINTILLAEDIT,y): LIBS += -L"../scintilla/qt/ScintillaEdit/$$_CONFIG_" -L"../lexilla/src/$$_CONFIG_"
    LIBS += -L$$[QT_INSTALL_LIBS]
        CONFIG(debug, debug|release) {
                        equals(USE_SCINTILLAEDIT,y): LIBS += -lScintillaEdit5 -lLexilla5
			equals(USE_SCINTILLAEDIT,n): LIBS += -lqscintilla2_qt$${QT_MAJOR_VERSION}d        
        }
	else
	{
        equals(USE_SCINTILLAEDIT,n): LIBS += -lqscintilla2_qt$${QT_MAJOR_VERSION}
        equals(USE_SCINTILLAEDIT,y): LIBS += -lScintillaEdit5 -lLexilla5
    }
}

macx {
   equals(USE_SCINTILLAEDIT,n): LIBS += -lqscintilla2_qt$${QT_MAJOR_VERSION}
   equals(USE_SCINTILLAEDIT,y): LIBS += -L"../scintilla/qt/ScintillaEdit" -L"../lexilla/src" -lScintillaEdit -lLexilla
}

unix:!macx {
   equals(USE_SCINTILLAEDIT,n): LIBS += -lqscintilla2_qt$${QT_MAJOR_VERSION}
   equals(USE_SCINTILLAEDIT,y): LIBS += -L"../scintilla/qt/ScintillaEdit" -L"../lexilla/src" -lScintillaEdit -lLexilla
   #LIBS += -lqt5scintilla2 #For PI ?
   QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN\''
}

RESOURCES += \
    ui.qrc
