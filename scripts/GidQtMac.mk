DEPLOYQT=$(QT)/bin/macdeployqt
SUBMAKE=$(MAKE) -f scripts/Makefile.gid $(MAKEJOBS)

QT_VER?=6

buildqtapp: buildqtlibs buildqtplugins buildqt

qtapp.install: qtlibs.install qtplugins.install qt.install

qtapp.clean: qtlibs.clean qtplugins.clean qt.clean


vpath %.dylib libgideros:libgvfs:libgid:$(LUA_ENGINE)

$(SDK)/lib/desktop/%: %
	cp $^ $(SDK)/lib/desktop
	

SDK_LIBS_QTLIST=libgideros liblua libgid libgvfs
SDK_LIBS_QT=$(addsuffix .1.dylib,$(SDK_LIBS_QTLIST)) $(addsuffix .dylib,$(SDK_LIBS_QTLIST))

sdk.qtlibs.dir:
	mkdir -p $(SDK)/lib/desktop	

sdk.qtlibs: sdk.headers sdk.qtlibs.dir $(addprefix $(SDK)/lib/desktop/,$(SDK_LIBS_QT))			
			
buildqtlibs: $(addsuffix .qmake.rel,libpystring libgvfs libgid/xmp) libgid.qmake5.rel $(addsuffix .qmake.rel,$(LUA_ENGINE) libgideros) sdk.qtlibs

qtlibs.clean: $(addsuffix .qmake.clean,libpystring libgvfs libgid/xmp libgid $(LUA_ENGINE) libgideros)


qtlibs.install: buildqtlibs
	mkdir -p $(RELEASE)
	
qscintilla:
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(QMAKE) ScintillaEdit.pro
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(MAKE) $(MAKEJOBS)
	mkdir -p $(QT)/include/ScintillaEdit
	cp scintilla/include/*.h scintilla/src/*.h scintilla/qt/ScintillaEdit/*.h scintilla/qt/ScintillaEditBase/*.h $(QT)/include/ScintillaEdit

qlexilla:
	cd $(ROOT)/lexilla/src; $(QMAKE) Lexilla.pro
	cd $(ROOT)/lexilla/src; $(MAKE) $(MAKEJOBS)
	mkdir -p $(QT)/include/Lexilla
	cp lexilla/include/*.h $(QT)/include/Lexilla

qscintilla.debug:
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(QMAKE) ScintillaEdit.pro
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(MAKE) $(MAKEJOBS) debug

qlexilla.debug:
	cd $(ROOT)/lexilla/src; $(QMAKE) Lexilla.pro
	cd $(ROOT)/lexilla/src; $(MAKE) $(MAKEJOBS) debug


%.qtplugin:
	cd $(ROOT)/plugins/$*/source; if [ -d "osx" ]; then cd osx; $(MAKE) $(MAKEJOBS); \
		else if [ -d "Desktop" ]; then cd Desktop; fi; $(QMAKE) *.pro; $(MAKE) $(MAKEJOBS); fi 

%.qtplugin.clean:
	cd $(ROOT)/plugins/$*/source; if [ -d "osx" ]; then cd osx; elif [ -d "Desktop" ]; then cd Desktop; fi; git clean -dfx .

%.qtplugin.install:
	mkdir -p $(RELEASE)/Plugins
	mkdir -p $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Plugins
	mkdir -p $(RELEASE)/All\ Plugins/$(notdir $*)/bin/MacOSX
	R=$(PWD); cd $(ROOT)/plugins/$*/source; if [ -d "osx" ]; then cd osx; \
		else if [ -d "Desktop" ]; then cd Desktop; fi; \
		for fl in *.1.0.0.dylib; do fl2=`echo $$fl | sed -e 's/\\..*//'`; echo $$fl $$fl2; cp  -L $$fl ../$$fl2.dylib; rm *.dylib; mv ../*.dylib .; done; fi; \
	cp *.dylib $$R/$(RELEASE)/Plugins; \
	cp *.dylib $$R/$(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Plugins; \
	cp *.dylib $$R/$(RELEASE)/All\ Plugins/$(notdir $*)/bin/MacOSX	

buildqt: versioning $(addsuffix .qmake.rel,texturepacker fontcreator ui) player.qmake5.rel $(addsuffix .qmake.rel,gdrdeamon gdrbridge gdrexport desktop)

qt.clean: qtlibs.clean $(addsuffix .qmake.clean,texturepacker fontcreator ui player gdrdeamon gdrbridge gdrexport desktop) html5.tools.clean

QSCINTILLA_LIBVER=$(word 2,$(subst ., ,$(filter libqscintilla%,$(subst /, ,$(shell otool -L $(ROOT)/ui/Gideros\ Studio.app/Contents/MacOS/Gideros\ Studio | grep libqscintilla)))))
qt.install: buildqt qt.player tools html5.tools
	#STUDIO
	rm -rf $(RELEASE)/Gideros\ Studio.app
	cp -R $(ROOT)/ui/Gideros\ Studio.app $(RELEASE)
	$(DEPLOYQT) $(RELEASE)/Gideros\ Studio.app
	#cp $(QT)/lib/libqscintilla2_qt$(QT_VER).$(QSCINTILLA_LIBVER).dylib $(RELEASE)/Gideros\ Studio.app/Contents/Frameworks/ 
	cp $(ROOT)/scintilla/qt/ScintillaEdit/libScintillaEdit.5.dylib $(RELEASE)/Gideros\ Studio.app/Contents/Frameworks/ 
	cp $(ROOT)/lexilla/src/libLexilla.5.dylib $(RELEASE)/Gideros\ Studio.app/Contents/Frameworks/ 
	#install_name_tool -change libqscintilla2_qt$(QT_VER).$(QSCINTILLA_LIBVER).dylib @rpath/libqscintilla2_qt$(QT_VER).$(QSCINTILLA_LIBVER).dylib  $(RELEASE)/Gideros\ Studio.app/Contents/MacOS/Gideros\ Studio
	install_name_tool -change libScintillaEdit.5.dylib @rpath/libScintillaEdit.5.dylib  $(RELEASE)/Gideros\ Studio.app/Contents/MacOS/Gideros\ Studio
	install_name_tool -change libLexilla.5.dylib @rpath/libLexilla.5.dylib  $(RELEASE)/Gideros\ Studio.app/Contents/MacOS/Gideros\ Studio
	cp -R $(ROOT)/ui/Resources $(RELEASE)/Gideros\ Studio.app/Contents/
	-wget -nv "http://wiki.giderosmobile.com/gidapi.php" -O $(RELEASE)/Gideros\ Studio.app/Contents/Resources/gideros_annot.api	
	install_name_tool -add_rpath @executable_path/../Frameworks $(ROOT)/ui/Tools/crunchme
	cp -R $(ROOT)/ui/Tools $(RELEASE)/Gideros\ Studio.app/Contents/Tools
	cp $(BUILDTOOLS)/lua $(RELEASE)/Gideros\ Studio.app/Contents/Tools
	cp $(BUILDTOOLS)/luac $(RELEASE)/Gideros\ Studio.app/Contents/Tools
	cp $(BUILDTOOLS)/luauc $(RELEASE)/Gideros\ Studio.app/Contents/Tools
	for t in gdrdeamon gdrbridge gdrexport; do \
	install_name_tool -add_rpath @executable_path/../Frameworks $(ROOT)/$$t/$$t;\
	cp $(ROOT)/$$t/$$t $(RELEASE)/Gideros\ Studio.app/Contents/Tools; done 
	#PLAYER
	rm -rf $(RELEASE)/Gideros\ Player.app
	cp -R $(ROOT)/player/Gideros\ Player.app $(RELEASE)
	$(DEPLOYQT) $(RELEASE)/Gideros\ Player.app
	cp $(SDK)/lib/desktop/*.1.dylib $(RELEASE)/Gideros\ Player.app/Contents/Frameworks/
	cp libpystring/libpystring.1.dylib $(RELEASE)/Gideros\ Player.app/Contents/Frameworks/
	install_name_tool -change libgvfs.1.dylib @rpath/libgvfs.1.dylib  $(RELEASE)/Gideros\ Player.app/Contents/Frameworks/libgid.1.dylib 
	install_name_tool -change libgvfs.1.dylib @rpath/libgvfs.1.dylib  $(RELEASE)/Gideros\ Player.app/Contents/Frameworks/liblua.1.dylib 
	install_name_tool -change libgid.1.dylib @rpath/libgid.1.dylib  $(RELEASE)/Gideros\ Player.app/Contents/Frameworks/libgideros.1.dylib 
	install_name_tool -change liblua.1.dylib @rpath/liblua.1.dylib  $(RELEASE)/Gideros\ Player.app/Contents/Frameworks/libgideros.1.dylib 
	install_name_tool -change libpystring.1.dylib @rpath/libpystring.1.dylib  $(RELEASE)/Gideros\ Player.app/Contents/Frameworks/libgideros.1.dylib 
	install_name_tool -change libgvfs.1.dylib @rpath/libgvfs.1.dylib  $(RELEASE)/Gideros\ Player.app/Contents/MacOS/Gideros\ Player 
	install_name_tool -change libgideros.1.dylib @rpath/libgideros.1.dylib  $(RELEASE)/Gideros\ Player.app/Contents/MacOS/Gideros\ Player 
	install_name_tool -change libgid.1.dylib @rpath/libgid.1.dylib  $(RELEASE)/Gideros\ Player.app/Contents/MacOS/Gideros\ Player 
	install_name_tool -change liblua.1.dylib @rpath/liblua.1.dylib  $(RELEASE)/Gideros\ Player.app/Contents/MacOS/Gideros\ Player 
	install_name_tool -change libpystring.1.dylib @rpath/libpystring.1.dylib  $(RELEASE)/Gideros\ Player.app/Contents/MacOS/Gideros\ Player 
	#TEXTUREPACKER
	rm -rf $(RELEASE)/Gideros\ Texture\ Packer.app
	cp -R $(ROOT)/texturepacker/Gideros\ Texture\ Packer.app $(RELEASE)
	$(DEPLOYQT) $(RELEASE)/Gideros\ Texture\ Packer.app
	#FONT CREATOR
	rm -rf $(RELEASE)/Gideros\ Font\ Creator.app
	cp -R $(ROOT)/fontcreator/Gideros\ Font\ Creator.app $(RELEASE)
	$(DEPLOYQT) $(RELEASE)/Gideros\ Font\ Creator.app
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
	cd plugins; git archive master | tar -x -C ../$(RELEASE)/All\ Plugins
	

QTDLLEXT?=

qt.player:
	mkdir -p $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate
	cp -R $(ROOT)/desktop/MacOSXDesktopTemplate.app $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate
	cp -R $(ROOT)/desktop/Entitlements.plist $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate
	$(DEPLOYQT) $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/
	cp $(SDK)/lib/desktop/*.1.dylib $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Frameworks/
	cp libpystring/libpystring.1.dylib $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Frameworks/
	install_name_tool -change libgvfs.1.dylib @rpath/libgvfs.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Frameworks/libgid.1.dylib 
	install_name_tool -change libgvfs.1.dylib @rpath/libgvfs.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Frameworks/liblua.1.dylib 
	install_name_tool -change libgid.1.dylib @rpath/libgid.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Frameworks/libgideros.1.dylib 
	install_name_tool -change liblua.1.dylib @rpath/liblua.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Frameworks/libgideros.1.dylib 
	install_name_tool -change libpystring.1.dylib @rpath/libpystring.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Frameworks/libgideros.1.dylib 
	install_name_tool -change libgideros.1.dylib @rpath/libgideros.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/MacOS/MacOSXDesktopTemplate 
	install_name_tool -change libgvfs.1.dylib @rpath/libgvfs.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/MacOS/MacOSXDesktopTemplate 
	install_name_tool -change libgid.1.dylib @rpath/libgid.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/MacOS/MacOSXDesktopTemplate 
	install_name_tool -change liblua.1.dylib @rpath/liblua.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/MacOS/MacOSXDesktopTemplate 
	install_name_tool -change libpystring.1.dylib @rpath/libpystring.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/MacOS/MacOSXDesktopTemplate 
	
buildqtplugins: 
	$(SUBMAKE) $(addsuffix .qtplugin,$(PLUGINS_WIN) $(PLUGINS_MACONLY))

qtplugins.clean: 
	$(SUBMAKE)  $(addsuffix .qtplugin.clean,$(PLUGINS_WIN) $(PLUGINS_MACONLY)) 

qtplugins.install: buildqtplugins 
	$(SUBMAKE)  $(addsuffix .qtplugin.install,$(PLUGINS_WIN) $(PLUGINS_MACONLY))

%.qmake.clean:
	cd $(ROOT)/$*; git clean -dfx .

%.qmake.rel:
	cd $(ROOT)/$*; $(QMAKE) $(notdir $*).pro
	cd $(ROOT)/$*; $(MAKE) $(MAKEJOBS)

%.qmake5.rel:
	cd $(ROOT)/$*; $(QMAKE) $(notdir $*)_qt5.pro
	cd $(ROOT)/$*; $(MAKE) $(MAKEJOBS) 

tools:
	mkdir -p $(BUILDTOOLS)
	cd $(ROOT)/luau; g++ -std=c++17 -Wno-attributes -IVM/include -ICompiler/include -IAst/include -ICommon/include -Iextern -Iextern/isocline/include -DDESKTOP_TOOLS -o../$(BUILDTOOLS)/luauc $(addsuffix .cpp,\
		$(addprefix CLI/,Coverage FileUtils Flags Profiler Repl ReplEntry) \
		$(addprefix VM/src/,lapi laux lbaselib lbitlib lbuiltins lcorolib ldblib ldebug ldo lfunc lgc\
    	lgcdebug linit lint64lib liolib lmathlib lmem lnumprint lobject loslib lperf lstate lstring lstrlib ltable ltablib ltm\
        ludata lutf8lib lvmexecute lvmload lvmutils) \
		$(addprefix Compiler/src/,Builtins BuiltinFolding BytecodeBuilder ConstantFolding Compiler CostModel lcode PseudoCode TableShape ValueTracking) \
		$(addprefix Ast/src/,Ast Confusables Lexer Location Parser StringUtils TimeTrace)) extern/isocline/src/isocline.c

	cd $(ROOT)/lua/src; gcc -I. -DDESKTOP_TOOLS -o ../../$(BUILDTOOLS)/luac $(addsuffix .c,print lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio luac lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib lutf8lib lint64)
	cd $(ROOT)/lua/src; gcc -I. -DDESKTOP_TOOLS -o ../../$(BUILDTOOLS)/lua $(addsuffix .c,lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lua lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib lutf8lib lint64)
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
	cd $(RELEASE).Final; if [ -f ../$(notdir $(RELEASE))/BuildWin.zip ]; then unzip -o ../$(notdir $(RELEASE))/BuildWin.zip; fi
	#Use our local version of that file due to line endings change
	cp $(ROOT)/ui/Templates/AndroidStudio/Android\ Template/gradlew $(RELEASE).Final/Templates/AndroidStudio/Android\ Template
	cd plugins; git archive master | tar -x -C ../$(RELEASE).Final/All\ Plugins
	#mv $(RELEASE).Final/Templates $(RELEASE).Final/Gideros\ Studio.app/Contents
	cp -r $(RELEASE).Final/Resources $(RELEASE).Final/Gideros\ Studio.app/Contents

bundle.mac:
	cd plugins; git archive $(CURRENT_GIT_BRANCH) | tar -x -C ../$(RELEASE).Final/All\ Plugins
	cp -r $(RELEASE)/Templates $(RELEASE)/Gideros\ Studio.app/Contents/
	cd plugins; git archive master | tar -x -C ../$(RELEASE)/All\ Plugins

bundle.installer: bundle
	rm -rf $(ROOT)/ROOTMAC
	mkdir  -p $(ROOT)/ROOTMAC/Applications
	mv $(RELEASE).Final $(ROOT)/ROOTMAC/Applications/Gideros\ Studio
	rm -f $(ROOT)/Gideros.pkg
	pkgbuild --compression latest --min-os-version 10.12 --root $(ROOT)/ROOTMAC --identifier com.giderosmobile.gideros --version $(GIDEROS_VERSION) --component-plist $(ROOT)/Release/pkg.plist $(ROOT)/Gideros-App.pkg
	security -v unlock-keychain -p $(OSX_SIGNING_PASSWORD) "$$HOME/Library/Keychains/login.keychain" && productbuild --distribution Release/GiderosDist.plist --package-path $(ROOT)/Gideros-App.pkg --sign $(OSX_SIGNING_IDENTITY) $(ROOT)/Gideros.pkg
	rm -rf $(ROOT)/ROOTMAC
	cd plugins; git archive $(CURRENT_GIT_BRANCH) | tar -x -C ../$(RELEASE)/All\ Plugins
