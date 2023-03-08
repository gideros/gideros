buildqtapp: buildqtlibs buildqtplugins buildqt

qtapp.install: qtlibs.install qtplugins.install qt.install addons.pkg

qtapp.clean: qtlibs.clean qtplugins.clean qt.clean

QTDLLEXT?=

QT_VER?=6

ifneq ($(DEBUG),)
QTTGT_EXT=dbg
QTDLLEXT=d
QTTGT_DIR=debug
else
QTTGT_EXT=rel
QTTGT_DIR=release
endif

SUBMAKE=$(MAKE) -f scripts/Makefile.gid $(MAKEJOBS)


vpath %.a libgideros/$(QTTGT_DIR):libgvfs/$(QTTGT_DIR):libgid/$(QTTGT_DIR):libgid/openal/$(QTTGT_DIR):$(LUA_ENGINE)/$(QTTGT_DIR)

$(SDK)/lib/desktop/%: %
	cp $^ $(SDK)/lib/desktop
	

SDK_LIBS_QT=libgideros.a liblua.a libgid.a libgvfs.a libopenal.a

sdk.qtlibs.dir:
	mkdir -p $(SDK)/lib/desktop	

sdk.qtlibs: sdk.headers sdk.qtlibs.dir $(addprefix $(SDK)/lib/desktop/,$(SDK_LIBS_QT))			
			
buildqtlibs: $(addsuffix .qmake.$(QTTGT_EXT),libpystring libgvfs libgid/openal libgid/xmp) libgid.qmake5.$(QTTGT_EXT) $(addsuffix .qmake.$(QTTGT_EXT),$(LUA_ENGINE) libgideros) sdk.qtlibs

qtlibs.clean: $(addsuffix .qmake.clean,libpystring libgvfs libgid/openal libgid/xmp libgid $(LUA_ENGINE) libgideros)

export MSBUILD

qtlibs.install: buildqtlibs
	mkdir -p $(RELEASE)
	cp $(ROOT)/libgid/$(QTTGT_DIR)/gid.dll $(RELEASE)
	cp $(ROOT)/libgvfs/$(QTTGT_DIR)/gvfs.dll $(RELEASE)
	cp $(ROOT)/$(LUA_ENGINE)/$(QTTGT_DIR)/lua.dll $(RELEASE)
	cp $(ROOT)/libgideros/$(QTTGT_DIR)/gideros.dll $(RELEASE)
	cp $(ROOT)/libpystring/$(QTTGT_DIR)/pystring.dll $(RELEASE)

%.qtplugin:
	cd $(ROOT)/plugins/$*/source; if [ -d "vs" ]; then cd vs; $(MINGWMAKE) $(MAKEJOBS); \
		else if [ -d "Desktop" ]; then cd Desktop; fi; $(QMAKE) *.pro; $(MINGWMAKE) $(MAKEJOBS) $(QTTGT_DIR); fi 

%.qtplugin.clean:
	cd $(ROOT)/plugins/$*/source; if [ -d "vs" ]; then cd vs; elif [ -d "Desktop" ]; then cd Desktop; fi; if [ -f Makefile ]; then $(MINGWMAKE) clean; fi

%.qtplugin.install:
	mkdir -p $(RELEASE)/Plugins
	mkdir -p $(RELEASE)/Templates/Qt/WindowsDesktopTemplate/Plugins
	mkdir -p $(RELEASE)/All\ Plugins/$(notdir $*)/bin/Windows
	R=$(PWD); cd $(ROOT)/plugins/$*/source; if [ -d "vs" ]; then cd vs; \
		cp *.dll $$R/$(RELEASE)/Plugins; \
		cp *.dll $$R/$(RELEASE)/Templates/Qt/WindowsDesktopTemplate/Plugins; \
		cp *.dll $$R/$(RELEASE)/All\ Plugins/$(notdir $*)/bin/Windows;\
		if [ -d "deps" ]; then cd deps; \
		cp *.dll $$R/$(RELEASE); \
		cp *.dll $$R/$(RELEASE)/Templates/Qt/WindowsDesktopTemplate; \
		cp *.dll $$R/$(RELEASE)/All\ Plugins/$(notdir $*)/bin/Windows;\
		fi;\
		else if [ -d "Desktop" ]; then cd Desktop; fi; \
		cp $(QTTGT_DIR)/*.dll $$R/$(RELEASE)/Plugins; \
		cp $(QTTGT_DIR)/*.dll $$R/$(RELEASE)/Templates/Qt/WindowsDesktopTemplate/Plugins; \
		cp $(QTTGT_DIR)/*.dll $$R/$(RELEASE)/All\ Plugins/$(notdir $*)/bin/Windows;\
		fi

qtlibs.clean: $(addsuffix .qmake.clean,libpystring libgvfs libgid $(LUA_ENGINE) libgideros)

qscintilla:
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(QMAKE) ScintillaEdit.pro
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(MINGWMAKE) $(MAKEJOBS) release
	mkdir -p $(QT)/include/ScintillaEdit
	cp scintilla/include/*.h scintilla/src/*.h scintilla/qt/ScintillaEdit/*.h scintilla/qt/ScintillaEditBase/*.h $(QT)/include/ScintillaEdit

qlexilla:
	cd $(ROOT)/lexilla/src; $(QMAKE) Lexilla.pro
	cd $(ROOT)/lexilla/src; $(MINGWMAKE) $(MAKEJOBS) release
	mkdir -p $(QT)/include/Lexilla
	cp lexilla/include/*.h $(QT)/include/Lexilla

qscintilla.debug:
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(QMAKE) ScintillaEdit.pro
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(MINGWMAKE) $(MAKEJOBS) debug

qlexilla.debug:
	cd $(ROOT)/lexilla/src; $(QMAKE) Lexilla.pro
	cd $(ROOT)/lexilla/src; $(MINGWMAKE) $(MAKEJOBS) debug
	
	

buildqt: versioning $(addsuffix .qmake.$(QTTGT_EXT),texturepacker fontcreator ui) player.qmake5.$(QTTGT_EXT) $(addsuffix .qmake.$(QTTGT_EXT),gdrdeamon gdrbridge gdrexport desktop)

qt.clean: qtlibs.clean $(addsuffix .qmake.clean,texturepacker fontcreator ui player gdrdeamon gdrbridge gdrexport desktop) qtplugins.clean html5.tools.clean

qt.install: buildqt qt5.install qt.player html5.tools
	cp $(ROOT)/ui/$(QTTGT_DIR)/GiderosStudio.exe $(RELEASE)
	cp $(ROOT)/player/$(QTTGT_DIR)/GiderosPlayer.exe $(RELEASE)
	cp $(ROOT)/texturepacker/$(QTTGT_DIR)/GiderosTexturePacker.exe $(RELEASE)
	cp $(ROOT)/fontcreator/$(QTTGT_DIR)/GiderosFontCreator.exe $(RELEASE)
	cp -R $(ROOT)/ui/Resources $(RELEASE)
	-wget -nv "http://wiki.giderosmobile.com/gidapi.php" -O $(RELEASE)/Resources/gideros_annot.api
	cd $(ROOT)/ui/;tar cf - --exclude=Tools/lua --exclude Tools/luac --exclude Tools/make Tools | (cd ../$(RELEASE) && tar xvf - )
	cp $(BUILDTOOLS)/lua.exe $(RELEASE)/Tools
	cp $(BUILDTOOLS)/luac.exe $(RELEASE)/Tools
	cp $(BUILDTOOLS)/luauc.exe $(RELEASE)/Tools
	cp $(BUILDTOOLS)/lua51.dll $(RELEASE)/Tools
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
	cp $(ROOT)/gdrdeamon/release/gdrdeamon.exe $(RELEASE)/Tools
	cp $(ROOT)/gdrbridge/release/gdrbridge.exe $(RELEASE)/Tools
	cp $(ROOT)/gdrexport/release/gdrexport.exe $(RELEASE)/Tools
	cp $(ROOT)/external/fbxconv-bin/fbx-conv-win32.exe $(RELEASE)/Tools
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
QT5DLLS=libgcc_s_seh-1 libstdc++-6 libwinpthread-1 \
		Qt6Core Qt6Gui Qt6Network Qt6OpenGL Qt6OpenGLWidgets Qt6PrintSupport Qt6Widgets Qt6Xml \
		Qt6Multimedia Qt6MultimediaQuick Qt6MultimediaWidgets Qt6WebSockets Qt6Core5Compat
QT5DLLTOOLS=libgcc_s_seh-1 libstdc++-6 libwinpthread-1 \
		Qt6Core Qt6Network Qt6Xml Qt6WebSockets
QT5PLATFORM=qminimal qoffscreen qwindows
QT5PLUGINS=$(addprefix tls/,qopensslbackend) $(addprefix platforms/,$(QT5PLATFORM)) imageformats/qjpeg
endif

qt.player:
	mkdir -p $(RELEASE)/Templates/Qt/WindowsDesktopTemplate
	cp $(ROOT)/desktop/release/WindowsDesktopTemplate.exe $(RELEASE)/Templates/Qt/WindowsDesktopTemplate
	for f in gid gvfs lua gideros pystring; do cp $(RELEASE)/$$f.dll $(RELEASE)/Templates/Qt/WindowsDesktopTemplate; done
	for f in $(addsuffix $(QTDLLEXT),$(QT5DLLS)); do cp $(QT)/bin/$$f.dll $(RELEASE)/Templates/Qt/WindowsDesktopTemplate; done
	mkdir -p $(addprefix $(RELEASE)/Templates/Qt/WindowsDesktopTemplate/,$(dir $(QT5PLUGINS)))
	for a in $(QT5PLUGINS); do cp $(QT)/plugins/$$a.dll$(QTDLLEXT) $(RELEASE)/Templates/Qt/WindowsDesktopTemplate/$$a.dll$(QTDLLEXT); done
	cp $(ROOT)/libgid/openal/release/openal.dll $(RELEASE)/Templates/Qt/WindowsDesktopTemplate
	mkdir -p $(RELEASE)/Templates/Qt/WindowsDesktopTemplate/Plugins

qt5.install:
	for f in $(addsuffix $(QTDLLEXT),$(QT5DLLS)); do cp $(QT)/bin/$$f.dll $(RELEASE); done
	mkdir -p $(addprefix $(RELEASE)/,$(dir $(QT5PLUGINS)))
	for a in $(QT5PLUGINS); do cp $(QT)/plugins/$$a.dll$(QTDLLEXT) $(RELEASE)/$$a.dll$(QTDLLEXT); done
	#cp $(QT)/lib/qscintilla2_qt$(QT_VER).dll $(RELEASE)
	cp $(ROOT)/scintilla/qt/ScintillaEdit/release/ScintillaEdit5.dll $(RELEASE)
	cp $(ROOT)/lexilla/src/release/Lexilla5.dll $(RELEASE)
	cp $(ROOT)/libgid/openal/release/openal.dll $(RELEASE)
	mkdir -p $(RELEASE)/Tools
	for f in $(QT5DLLTOOLS); do cp $(QT)/bin/$$f.dll $(RELEASE)/Tools; done
	
buildqtplugins: 
	$(SUBMAKE) $(addsuffix .qtplugin,$(PLUGINS_WIN) $(PLUGINS_WINONLY))

qtplugins.clean: 
	$(SUBMAKE)  $(addsuffix .qtplugin.clean,$(PLUGINS_WIN) $(PLUGINS_WINONLY)) 

qtplugins.install: buildqtplugins 
	$(SUBMAKE)  $(addsuffix .qtplugin.install,$(PLUGINS_WIN) $(PLUGINS_WINONLY))

%.qmake.clean:
	cd $(ROOT)/$*; if [ -f Makefile ]; then $(MINGWMAKE) clean; fi

%.qmake.rel:
	cd $(ROOT)/$*; $(QMAKE) $(notdir $*).pro
	cd $(ROOT)/$*; $(MINGWMAKE) $(MAKEJOBS) release

%.qmake.dbg:
	cd $(ROOT)/$*; $(QMAKE) $(notdir $*).pro
	cd $(ROOT)/$*; $(MINGWMAKE) $(MAKEJOBS) debug

%.qmake5.rel:
	cd $(ROOT)/$*; $(QMAKE) $(notdir $*)_qt5.pro
	cd $(ROOT)/$*; $(MINGWMAKE) $(MAKEJOBS) release

%.qmake5.dbg:
	cd $(ROOT)/$*; $(QMAKE) $(notdir $*)_qt5.pro
	cd $(ROOT)/$*; $(MINGWMAKE) $(MAKEJOBS) debug

tools:
	mkdir -p $(BUILDTOOLS)
	for f in libgcc_s_seh-1 libstdc++-6 libwinpthread-1; do cp $(QT)/bin/$$f.dll $(BUILDTOOLS); done
	cd $(ROOT)/luau; g++ -std=c++17 -Wno-attributes -IVM/include -ICompiler/include -IAst/include -ICommon/include -DNO_CODEGEN -Iextern -Iextern/isocline/include -DDESKTOP_TOOLS -o../$(BUILDTOOLS)/luauc $(addsuffix .cpp,\
		$(addprefix CLI/,Coverage FileUtils Flags Profiler Repl ReplEntry) \
		$(addprefix VM/src/,lapi laux lbaselib lbitlib lbuiltins lcorolib ldblib ldebug ldo lfunc lgc\
    	lgcdebug linit lint64lib liolib lmathlib lmem lnumprint lobject loslib lperf lstate lstring lstrlib ltable ltablib ltm\
        ludata lutf8lib lvmexecute lvmload lvmutils) \
		$(addprefix Compiler/src/,Builtins BuiltinFolding BytecodeBuilder ConstantFolding Compiler CostModel lcode PseudoCode TableShape ValueTracking) \
		$(addprefix Ast/src/,Ast Confusables Lexer Location Parser StringUtils TimeTrace)) extern/isocline/src/isocline.c
	cd $(ROOT)/lua/src; gcc -I. -DDESKTOP_TOOLS -o../../$(BUILDTOOLS)/luac $(addsuffix .c,print lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio luac lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib lutf8lib lint64)
	cd $(ROOT)/lua/src; gcc -I. -DDESKTOP_TOOLS -shared -o../../$(BUILDTOOLS)/lua51.dll -Wl,--out-implib,lua51.a $(addsuffix .c,lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib lutf8lib lint64)
	cd $(ROOT)/lua/src; gcc -I. -DDESKTOP_TOOLS -o../../$(BUILDTOOLS)/lua lua.c lua51.a
	gcc -I. -DDESKTOP_TOOLS -o$(BUILDTOOLS)/bin2c scripts/bin2c.c
	
	
bundle:
	rm -rf $(RELEASE).Tmp
	mkdir -p $(RELEASE).Tmp
	rm -rf $(RELEASE).Final
	mkdir -p $(RELEASE).Final
	mv $(RELEASE)/*.zip $(RELEASE).Tmp
	cp -R $(RELEASE)/* $(RELEASE).Final
	mv $(RELEASE).Tmp/* $(RELEASE)
	rm -rf $(RELEASE).Tmp
	cd $(RELEASE).Final; if [ -f ../$(notdir $(RELEASE))/BuildMac.zip ]; then unzip -o ../$(notdir $(RELEASE))/BuildMac.zip; fi
	-cd plugins; git archive $(CURRENT_GIT_BRANCH) | tar -x -C ../$(RELEASE).Final/All\ Plugins

bundle.win:
	-cd plugins; git archive $(CURRENT_GIT_BRANCH) | tar -x -C ../$(RELEASE)/All\ Plugins
	
bundle.installer: bundle
	cp $(ROOT)/Release/*.nsi $(RELEASE).Final
	cp $(ROOT)/Release/*.nsh $(RELEASE).Final
	cd $(RELEASE).Final; $(NSIS) gideros_mui2.nsi
	mv $(RELEASE).Final/Gideros.exe $(ROOT)/
	#if [ -f $(WIN_SIGN) ]; then $(WIN_SIGN) sign //v //f $(WIN_KEYSTORE) //p $(WIN_KEYPASS) //t $(WIN_KEYTSS) $(ROOT)/Gideros.exe; fi
	if [ -f $(WIN_SIGN) ]; then $(WIN_SIGN) sign //v //f $(WIN_KEYSTORE) //p $(WIN_KEYPASS) //fd sha256 //tr $(WIN_KEYTSS) //td sha256 //as $(ROOT)/Gideros.exe; fi

bundle.sign:
	#if [ -f $(WIN_SIGN) ]; then $(WIN_SIGN) sign //v //f $(WIN_KEYSTORE) //p $(WIN_KEYPASS) //t $(WIN_KEYTSS) $(ROOT)/Gideros.exe; fi
	if [ -f $(WIN_SIGN) ]; then $(WIN_SIGN) sign //v //f $(WIN_KEYSTORE) //p $(WIN_KEYPASS) //fd sha256 //tr $(WIN_KEYTSS) //td sha256 //as $(ROOT)/Gideros.exe; fi
