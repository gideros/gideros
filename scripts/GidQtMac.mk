DEPLOYQT=$(QT)/bin/macdeployqt

buildqtapp: buildqtlibs buildplugins buildqt

qtapp.install: qtlibs.install plugins.install qt.install

qtapp.clean: qtlibs.clean plugins.clean qt.clean


vpath %.dylib libgideros:libgvfs:libgid:lua

$(SDK)/lib/desktop/%: %
	cp $^ $(SDK)/lib/desktop
	

SDK_LIBS_QT=libgideros.1.dylib liblua.1.dylib libgid.1.dylib libgvfs.1.dylib

sdk.qtlibs.dir:
	mkdir -p $(SDK)/lib/desktop	

sdk.qtlibs: sdk.headers sdk.qtlibs.dir $(addprefix $(SDK)/lib/desktop/,$(SDK_LIBS_QT))			
			
buildqtlibs: $(addsuffix .qmake.rel,libpystring libgvfs) libgid.qmake5.rel $(addsuffix .qmake.rel,lua libgideros) sdk.qtlibs


qtlibs.install: buildqtlibs
	mkdir -p $(RELEASE)

%.plugin:
	cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; $(QMAKE) *.pro
	cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; $(MAKE) 

%.plugin.clean:
	cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; $(MAKE) clean

%.plugin.install:
	mkdir -p $(RELEASE)/Plugins
	mkdir -p $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/Plugins
	mkdir -p $(RELEASE)/All\ Plugins/$*/bin/MacOSX
	R=$(PWD); cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; cp *.dylib $$R/$(RELEASE)/Plugins	 
	R=$(PWD); cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; cp *.dylib $$R/$(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/Plugins	 
	R=$(PWD); cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; cp *.dylib $$R/$(RELEASE)/All\ Plugins/$*/bin/MacOSX	 

qtlibs.clean: $(addsuffix .qmake.clean,libpystring libgvfs libgid lua libgideros)

buildqt: $(addsuffix .qmake.rel,texturepacker fontcreator ui) player.qmake5.rel $(addsuffix .qmake.rel,gdrdeamon gdrbridge gdrexport desktop)

qt.clean: $(addsuffix .qmake.clean,texturepacker fontcreator ui player gdrdeamon gdrbridge gdrexport desktop)

qt.install: buildqt qt.player
	#STUDIO
	rm -rf $(RELEASE)/Gideros\ Studio.app
	cp -R $(ROOT)/ui/Gideros\ Studio.app $(RELEASE)
	$(DEPLOYQT) $(RELEASE)/Gideros\ Studio.app
	cp $(QT)/lib/libqscintilla2.11.dylib $(RELEASE)/Gideros\ Studio.app/Contents/Frameworks/ 
	cp -R $(ROOT)/ui/Resources $(RELEASE)/Gideros\ Studio.app/Contents/
	cp -R $(ROOT)/ui/Tools $(RELEASE)/Gideros\ Studio.app/Contents/Tools
	for t in gdrdeamon gdrbridge gdrexport; do cp $(ROOT)/$$t/$$t $(RELEASE)/Gideros\ Studio.app/Contents/Tools; done 
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
	cp -R $(ROOT)/ui/Templates/Eclipse $(RELEASE)/Templates
	mkdir -p $(RELEASE)/Templates/Eclipse/Android\ Template/assets
	mkdir -p $(RELEASE)/Templates/Eclipse/Android\ Template/gen
	mkdir -p $(RELEASE)/Templates/Eclipse/Android\ Template/res/layout
	cp -R $(ROOT)/ui/Templates/AndroidStudio $(RELEASE)/Templates
	mkdir -p $(RELEASE)/Templates/AndroidStudio/Android\ Template/app/libs
	mkdir -p $(RELEASE)/Templates/AndroidStudio/Android\ Template/app/src/main/assets
	mkdir -p $(RELEASE)/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs
	cp -R $(ROOT)/ui/Templates/Xcode4 $(RELEASE)/Templates
	mkdir -p $(RELEASE)/Templates/Xcode4/iOS\ Template/iOS\ Template/assets
	mkdir -p $(RELEASE)/Examples
	cp -R $(ROOT)/samplecode/* $(RELEASE)/Examples
	cp -R $(ROOT)/ios/GiderosiOSPlayer $(RELEASE)
	

QTDLLEXT?=

qt.player:
	mkdir -p $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate
	cp -R $(ROOT)/desktop/MacOSXDesktopTemplate.app $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate
	cp -R $(ROOT)/desktop/Entitlements.plist $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate
	$(DEPLOYQT) $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app
	cp $(SDK)/lib/desktop/*.1.dylib $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Frameworks/
	cp libpystring/libpystring.1.dylib $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Frameworks/
	install_name_tool -change libgvfs.1.dylib @rpath/libgvfs.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Frameworks/libgid.1.dylib 
	install_name_tool -change libgvfs.1.dylib @rpath/libgvfs.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Frameworks/liblua.1.dylib 
	install_name_tool -change libgid.1.dylib @rpath/libgid.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Frameworks/libgideros.1.dylib 
	install_name_tool -change liblua.1.dylib @rpath/liblua.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Frameworks/libgideros.1.dylib 
	install_name_tool -change libpystring.1.dylib @rpath/libpystring.1.dylib  $(RELEASE)/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/Frameworks/libgideros.1.dylib 
	
buildplugins: $(addsuffix .plugin,$(PLUGINS_WIN))

plugins.clean: $(addsuffix .plugin.clean,$(PLUGINS_WIN))

plugins.install: buildplugins $(addsuffix .plugin.install,$(PLUGINS_WIN))

%.qmake.clean:
	cd $(ROOT)/$*; $(MAKE) clean

%.qmake.rel:
	cd $(ROOT)/$*; $(QMAKE) $*.pro
	cd $(ROOT)/$*; $(MAKE)

%.qmake5.rel:
	cd $(ROOT)/$*; $(QMAKE) $*_qt5.pro
	cd $(ROOT)/$*; $(MAKE) 

tools:
	cd $(ROOT)/lua514u/src; gcc -o luac $(addsuffix .c,print lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio luac lgc)
	#cd $(ROOT)/lua514u/src; gcc -shared -o lua51.dll -Wl,--out-implib,lua51.a $(addsuffix .c,lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib)
	#cd $(ROOT)/lua514u/src; gcc -o lua lua.c lua51.a
	cd $(ROOT)/lua514u/src; gcc -o lua $(addsuffix .c,lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lua lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib)

bundle:
	mv $(RELEASE)/Templates $(RELEASE)/Gideros\ Studio.app/Contents
