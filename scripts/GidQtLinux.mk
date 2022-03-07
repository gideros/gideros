buildqtapp: buildqtlibs buildqtplugins buildqt

qtapp.install: qtlibs.install qtplugins.install qt.install

qtapp.clean: qtlibs.clean qtplugins.clean qt.clean

ifneq ($(DEBUG),)
QTTGT_EXT=dbg
QTDLLEXT=d
QTTGT_DIR=debug
else
QTTGT_EXT=rel
endif

SUBMAKE=$(MAKE) -f scripts/Makefile.gid $(MAKEJOBS)

vpath %.so libgideros:libgvfs:libgid:$(LUA_ENGINE)

$(SDK)/lib/desktop/%: %
	cp -P $^* $(SDK)/lib/desktop

SDK_LIBS_QTLIST=libgideros liblua libgid libgvfs
SDK_LIBS_QT=$(addsuffix .so,$(SDK_LIBS_QTLIST))

sdk.qtlibs.dir:
	mkdir -p $(SDK)/lib/desktop	

sdk.qtlibs: sdk.headers sdk.qtlibs.dir $(addprefix $(SDK)/lib/desktop/,$(SDK_LIBS_QT))	

buildqtlibs: $(addsuffix .qmake.rel, libpystring libgvfs libgid/xmp) libgid.qmake5.rel $(addsuffix .qmake.rel,$(LUA_ENGINE) libgideros) sdk.qtlibs

qtlibs.clean: $(addsuffix .qmake.clean,libpystring libgvfs libgid/xmp libgid $(LUA_ENGINE) libgideros)

qtlibs.install: buildqtlibs
	mkdir -p $(RELEASE)
	cp $(ROOT)/libgid/libgid.so* $(RELEASE)
	cp $(ROOT)/libgvfs/libgvfs.so* $(RELEASE)
	cp $(ROOT)/$(LUA_ENGINE)/liblua.so* $(RELEASE)
	cp $(ROOT)/libgideros/libgideros.so* $(RELEASE)
	cp $(ROOT)/libpystring/libpystring.so* $(RELEASE)

%.qtplugin:
	cd $(ROOT)/plugins/$*/source; if [ -d "linux" ]; then cd linux; $(MAKE) $(MAKEJOBS); \
		else if [ -d "Desktop" ]; then cd Desktop; fi; $(QMAKE) *.pro; $(MAKE) $(MAKEJOBS) $(QTTGT_DIR); fi 

%.qtplugin.clean:
	cd $(ROOT)/plugins/$*/source; if [ -d "linux" ]; then cd linux; elif [ -d "Desktop" ]; then cd Desktop; fi; if [ -f Makefile ]; then $(MAKE) clean; fi

%.qtplugin.install:
	mkdir -p $(RELEASE)/Plugins
	mkdir -p $(RELEASE)/Templates/Qt/LinuxDesktopTemplate/Plugins
	mkdir -p $(RELEASE)/All\ Plugins/$*/bin/Linux
	R=$(PWD); cd $(ROOT)/plugins/$*/source; if [ -d "linux" ]; then cd linux; \
		else if [ -d "Desktop" ]; then cd Desktop; fi; fi; \
	cp -P *.so* $$R/$(RELEASE)/Plugins; \
	cp -P *.so* $$R/$(RELEASE)/Templates/Qt/LinuxDesktopTemplate/Plugins; \
	cp -P *.so* $$R/$(RELEASE)/All\ Plugins/$*/bin/Linux	

qscintilla:
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(QMAKE) "DESTDIR=./release" ScintillaEdit.pro
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(MAKE) $(MAKEJOBS)
	mkdir -p $(QT)/include/ScintillaEdit
	cp scintilla/include/*.h scintilla/src/*.h scintilla/qt/ScintillaEdit/*.h scintilla/qt/ScintillaEditBase/*.h $(QT)/include/ScintillaEdit

qlexilla:
	cd $(ROOT)/lexilla/src; $(QMAKE) "DESTDIR=./release" Lexilla.pro
	cd $(ROOT)/lexilla/src; $(MAKE) $(MAKEJOBS)
	mkdir -p $(QT)/include/Lexilla
	cp lexilla/include/*.h $(QT)/include/Lexilla
	
qscintilla.clean:
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(MAKE) clean
qlexilla.clean:
	cd $(ROOT)/lexilla/src; $(MAKE) clean
	
qscintilla.debug:
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(QMAKE) "DESTDIR=./debug" ScintillaEdit.pro
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(MAKE) $(MAKEJOBS)

qlexilla.debug:
	cd $(ROOT)/lexilla/src; $(QMAKE) "DESTDIR=./debug" Lexilla.pro
	cd $(ROOT)/lexilla/src; $(MAKE) $(MAKEJOBS)

buildqt: versioning $(addsuffix .qmake.$(QTTGT_EXT),texturepacker fontcreator ui) player.qmake5.$(QTTGT_EXT) $(addsuffix .qmake.$(QTTGT_EXT),gdrdeamon gdrbridge gdrexport desktop)

qt.clean: qscintilla.clean qlexilla.clean qtlibs.clean $(addsuffix .qmake.clean,texturepacker fontcreator ui player gdrdeamon gdrbridge gdrexport desktop) html5.tools.clean

qt.install: buildqt qt5.install qt.player html5.tools
	cp $(ROOT)/ui/GiderosStudio $(RELEASE)
	cp $(ROOT)/player/GiderosPlayer $(RELEASE)
	cp $(ROOT)/texturepacker/GiderosTexturePacker $(RELEASE)
	cp $(ROOT)/fontcreator/GiderosFontCreator $(RELEASE)
	cp -R $(ROOT)/ui/Resources $(RELEASE)
	-wget -nv "http://wiki.giderosmobile.com/gidapi.php" -O $(RELEASE)/Resources/gideros_annot.api
	cd $(ROOT)/ui/;tar cf - --exclude=Tools/lua --exclude Tools/luac --exclude Tools/make Tools | (cd ../$(RELEASE) && tar xvf - )
	cp $(BUILDTOOLS)/lua $(RELEASE)/Tools
	cp $(BUILDTOOLS)/luac $(RELEASE)/Tools
	cp $(BUILDTOOLS)/luauc $(RELEASE)/Tools
	mkdir -p $(RELEASE)/Templates
	#Other templates
	cp -R $(ROOT)/ui/Templates/*.gexport $(RELEASE)/Templates
	cp -R $(ROOT)/ui/Templates/AndroidStudio $(RELEASE)/Templates
	mkdir -p $(RELEASE)/Templates/AndroidStudio/Android\ Template/app/libs
	mkdir -p $(RELEASE)/Templates/AndroidStudio/Android\ Template/app/src/main/assets
	mkdir -p $(RELEASE)/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs
	cp -R $(ROOT)/ui/Templates/Xcode4 $(RELEASE)/Templates
	mkdir -p $(RELEASE)/Templates/Xcode4/iOS\ Template/iOS\ Template/assets
	mkdir -p $(RELEASE)/Examples
	cp -R $(ROOT)/samplecode/* $(RELEASE)/Examples
	cp -R $(ROOT)/Library $(RELEASE)/
	cp $(ROOT)/gdrdeamon/gdrdeamon $(RELEASE)/Tools
	cp $(ROOT)/gdrbridge/gdrbridge $(RELEASE)/Tools
	cp $(ROOT)/gdrexport/gdrexport $(RELEASE)/Tools
	-cd plugins; git archive $(CURRENT_GIT_BRANCH) | tar -x -C ../$(RELEASE)/All\ Plugins

ifeq ($(QT_VER),5)
QT5DLLS=icudt$(QT5ICUVER) icuin$(QT5ICUVER) icuuc$(QT5ICUVER) libgcc_s_dw2-1 libstdc++-6 libwinpthread-1 \
		Qt5Core Qt5Gui Qt5Network Qt5OpenGL Qt5PrintSupport Qt5Widgets Qt5Xml \
		Qt5Multimedia Qt5MultimediaQuick_p Qt5MultimediaWidgets Qt5WebSockets
QT5DLLTOOLS=icudt$(QT5ICUVER) icuin$(QT5ICUVER) icuuc$(QT5ICUVER) libgcc_s_dw2-1 libstdc++-6 libwinpthread-1 \
		Qt5Core Qt5Network Qt5Xml Qt5WebSockets
QT5PLATFORM=qminimal qoffscreen qwindows
QT5PLUGINS=$(addprefix mediaservice/,dsengine qtmedia_audioengine) $(addprefix platforms/,$(QT5PLATFORM)) imageformats/qjpeg
else
QT5DLLS=$(addprefix lib,icui18n icudata icuuc \
		Qt6Core Qt6Gui Qt6Network Qt6OpenGL Qt6OpenGLWidgets Qt6PrintSupport Qt6Widgets Qt6Xml \
		Qt6Multimedia Qt6MultimediaQuick Qt6MultimediaWidgets Qt6WebSockets Qt6Core5Compat)
QT5DLLTOOLS=$(addprefix lib,icui18n icudata icuuc \
			Qt6Core Qt6Network Qt6Xml Qt6WebSockets)
QT5PLATFORM=$(addprefix lib,qminimal qoffscreen qlinuxfb qxcb)
QT5PLUGINS=$(addprefix tls/lib,qopensslbackend) $(addprefix xcbglintegrations/lib,qxcb-egl-integration qxcb-glx-integration) \
			$(addprefix platforms/,$(QT5PLATFORM)) imageformats/libqjpeg
endif

qt.player:
	mkdir -p $(RELEASE)/Templates/Qt/LinuxDesktopTemplate
	cp -R $(ROOT)/desktop/LinuxDesktopTemplate $(RELEASE)/Templates/Qt/LinuxDesktopTemplate
	cp -P $(SDK)/lib/desktop/*.so* $(RELEASE)/Templates/Qt/LinuxDesktopTemplate/
	cp -P libpystring/*.so* $(RELEASE)/Templates/Qt/LinuxDesktopTemplate
	mkdir -p $(addprefix $(RELEASE)/Templates/Qt/LinuxDesktopTemplate/,$(dir $(QT5PLUGINS)))
	for a in $(QT5PLUGINS); do cp $(QT)/plugins/$$a.so $(RELEASE)/Templates/Qt/LinuxDesktopTemplate/$$a.so; done
	cp $(ROOT)/libgid/external/openal-soft-1.13/build/gcc484_64/libopenal.so* $(RELEASE)/Templates/Qt/LinuxDesktopTemplate/
	for f in $(QT5DLLS); do cp -P $(QT)/lib/$$f.so* $(RELEASE)/Templates/Qt/LinuxDesktopTemplate; done

qt5.install:
	for f in $(addsuffix $(QTDLLEXT),$(QT5DLLS)); do cp $(QT)/lib/$$f.so* $(RELEASE); done
	mkdir -p $(addprefix $(RELEASE)/,$(dir $(QT5PLUGINS)))
	for a in $(QT5PLUGINS); do cp $(QT)/plugins/$$a.so $(RELEASE)/$$a.so; done
	cp $(ROOT)/scintilla/qt/ScintillaEdit/release/libScintillaEdit.so* $(RELEASE)
	cp $(ROOT)/lexilla/src/release/libLexilla.so* $(RELEASE)
	cp $(ROOT)/libgid/external/openal-soft-1.13/build/gcc484_64/libopenal.so* $(RELEASE)
	mkdir -p $(RELEASE)/Tools
	for f in $(QT5DLLTOOLS); do cp $(QT)/lib/$$f.so* $(RELEASE)/Tools; done
	
buildqtplugins: 
	$(SUBMAKE) $(addsuffix .qtplugin,$(PLUGINS_WIN))

qtplugins.clean: 
	$(SUBMAKE)  $(addsuffix .qtplugin.clean,$(PLUGINS_WIN)) 

qtplugins.install: buildqtplugins 
	$(SUBMAKE)  $(addsuffix .qtplugin.install,$(PLUGINS_WIN))

%.qmake.clean:
	cd $(ROOT)/$*; if [ -f Makefile ]; then $(MAKE) clean; fi

%.qmake.rel:
	#cd $(ROOT)/$*; $(QMAKE) $*.pro # previous state
	cd $(ROOT)/$*; $(QMAKE) $(notdir $*).pro
	cd $(ROOT)/$*; $(MAKE) $(MAKEJOBS)

%.qmake.dbg:
	cd $(ROOT)/$*; $(QMAKE) $(notdir $*).pro
	cd $(ROOT)/$*; $(MAKE) $(MAKEJOBS) debug

%.qmake5.rel:
	cd $(ROOT)/$*; $(QMAKE) $(notdir $*)_qt5.pro
	cd $(ROOT)/$*; $(MAKE) $(MAKEJOBS)

%.qmake5.dbg:
	cd $(ROOT)/$*; $(QMAKE) $(notdir $*)_qt5.pro
	cd $(ROOT)/$*; $(MAKE) $(MAKEJOBS) debug

tools:
	mkdir -p $(BUILDTOOLS)
	cd $(ROOT)/luau; g++ -std=c++17 -lpthread -Wno-attributes -IVM/include -ICompiler/include -IAst/include -Iextern -DDESKTOP_TOOLS -o../$(BUILDTOOLS)/luauc $(addsuffix .cpp,\
		$(addprefix CLI/,Coverage FileUtils Profiler Repl ReplEntry) \
		$(addprefix VM/src/,lapi laux lbaselib lbitlib lbuiltins lcorolib ldblib ldebug ldo lfunc lgc\
    	lgcdebug linit lint64lib liolib lmathlib lmem lnumprint lobject loslib lperf lstate lstring lstrlib ltable ltablib ltm\
        ludata lutf8lib lvmexecute lvmload lvmutils) \
		$(addprefix Compiler/src/,Builtins BytecodeBuilder ConstantFolding Compiler lcode PseudoCode TableShape ValueTracking) \
		$(addprefix Ast/src/,Ast Confusables Lexer Location Parser StringUtils TimeTrace))
	cd $(ROOT)/lua/src; gcc -I. -lm -DDESKTOP_TOOLS -o ../../$(BUILDTOOLS)/luac $(addsuffix .c,print lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio luac lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib lutf8lib lint64)
	cd $(ROOT)/lua/src; gcc -I. -lm -DDESKTOP_TOOLS -o ../../$(BUILDTOOLS)/lua $(addsuffix .c,lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lua lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib lutf8lib lint64)
	gcc -I. -DDESKTOP_TOOLS -o$(BUILDTOOLS)/bin2c scripts/bin2c.c
	
bundle:
	#to do

bundle.mac:
	#to do
	
bundle.installer: bundle
	#to do
