buildqtapp: buildqtlibs buildqtplugins buildqt

qtapp.install: qtlibs.install qtplugins.install qt.install

qtapp.clean: qtlibs.clean qtplugins.clean qt.clean


vpath %.a libgideros/release:libgvfs/release:libgid/release:lua/release:libgid/external/openal-soft-1.13/build/mingw48_32

$(SDK)/lib/desktop/%: %
	cp $^ $(SDK)/lib/desktop
	

SDK_LIBS_QT=libgideros.a liblua.a libgid.a libgvfs.a libOpenAL32.dll.a

sdk.qtlibs.dir:
	mkdir -p $(SDK)/lib/desktop	

sdk.qtlibs: sdk.headers sdk.qtlibs.dir $(addprefix $(SDK)/lib/desktop/,$(SDK_LIBS_QT))			
			
buildqtlibs: $(addsuffix .qmake.rel,libpystring libgvfs) libgid.qmake5.rel $(addsuffix .qmake.rel,lua libgideros) sdk.qtlibs


qtlibs.install: buildqtlibs
	mkdir -p $(RELEASE)
	cp $(ROOT)/libgid/release/gid.dll $(RELEASE)
	cp $(ROOT)/libgvfs/release/gvfs.dll $(RELEASE)
	cp $(ROOT)/lua/release/lua.dll $(RELEASE)
	cp $(ROOT)/libgideros/release/gideros.dll $(RELEASE)
	cp $(ROOT)/libpystring/release/pystring.dll $(RELEASE)

%.qtplugin:
	cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; $(QMAKE) *.pro
	cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; $(MINGWMAKE) release

%.qtplugin.clean:
	cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; $(MINGWMAKE) clean

%.qtplugin.install:
	mkdir -p $(RELEASE)/Plugins
	mkdir -p $(RELEASE)/Templates/Qt/WindowsDesktopTemplate/Plugins
	mkdir -p $(RELEASE)/All\ Plugins/$*/bin/Windows
	R=$(PWD); cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; cp release/*.dll $$R/$(RELEASE)/Plugins	 
	R=$(PWD); cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; cp release/*.dll $$R/$(RELEASE)/Templates/Qt/WindowsDesktopTemplate/Plugins	 
	R=$(PWD); cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; cp release/*.dll $$R/$(RELEASE)/All\ Plugins/$*/bin/Windows	 

qtlibs.clean: $(addsuffix .qmake.clean,libpystring libgvfs libgid lua libgideros)

buildqt: $(addsuffix .qmake.rel,texturepacker fontcreator ui) player.qmake5.rel $(addsuffix .qmake.rel,gdrdeamon gdrbridge gdrexport desktop)

qt.clean: $(addsuffix .qmake.clean,texturepacker fontcreator ui player gdrdeamon gdrbridge gdrexport desktop)

qt.install: buildqt qt5.install qt.player
	cp $(ROOT)/ui/release/GiderosStudio.exe $(RELEASE)
	cp $(ROOT)/player/release/GiderosPlayer.exe $(RELEASE)
	cp $(ROOT)/texturepacker/release/GiderosTexturePacker.exe $(RELEASE)
	cp $(ROOT)/fontcreator/release/GiderosFontCreator.exe $(RELEASE)
	cp -R $(ROOT)/ui/Resources $(RELEASE)
	cd $(ROOT)/ui/;tar cf - --exclude=Tools/lua --exclude Tools/luac --exclude Tools/make Tools | (cd ../$(RELEASE) && tar xvf - )
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
	cp $(ROOT)/gdrdeamon/release/gdrdeamon.exe $(RELEASE)/Tools
	cp $(ROOT)/gdrbridge/release/gdrbridge.exe $(RELEASE)/Tools
	cp $(ROOT)/gdrexport/release/gdrexport.exe $(RELEASE)/Tools
	
QT5DLLS=icudt$(QT5ICUVER) icuin$(QT5ICUVER) icuuc$(QT5ICUVER) libgcc_s_dw2-1 libstdc++-6 libwinpthread-1 \
		Qt5Core Qt5Gui Qt5Network Qt5OpenGL Qt5PrintSupport Qt5Widgets Qt5Xml \
		Qt5Multimedia Qt5MultimediaQuick_p Qt5MultimediaWidgets
QT5DLLTOOLS=icudt$(QT5ICUVER) icuin$(QT5ICUVER) icuuc$(QT5ICUVER) libgcc_s_dw2-1 libstdc++-6 libwinpthread-1 \
		Qt5Core Qt5Network Qt5Xml
QT5PLATFORM=qminimal qoffscreen qwindows
QTDLLEXT?=

qt.player:
	mkdir -p $(RELEASE)/Templates/Qt/WindowsDesktopTemplate
	cp $(ROOT)/desktop/release/WindowsDesktopTemplate.exe $(RELEASE)/Templates/Qt/WindowsDesktopTemplate
	for f in gid gvfs lua gideros pystring; do cp $(RELEASE)/$$f.dll $(RELEASE)/Templates/Qt/WindowsDesktopTemplate; done
	for f in $(addsuffix $(QTDLLEXT),$(QT5DLLS)); do cp $(QT)/bin/$$f.dll $(RELEASE)/Templates/Qt/WindowsDesktopTemplate; done
	mkdir -p $(RELEASE)/Templates/Qt/WindowsDesktopTemplate/imageformats
	cp $(QT)/plugins/imageformats/qjpeg.dll $(RELEASE)/Templates/Qt/WindowsDesktopTemplate/imageformats
	mkdir -p $(RELEASE)/Templates/Qt/WindowsDesktopTemplate/platforms
	for f in $(addsuffix $(QTDLLEXT),$(QT5PLATFORM)); do cp $(QT)/plugins/platforms/$$f.dll $(RELEASE)/Templates/Qt/WindowsDesktopTemplate/platforms; done
	cp $(ROOT)/libgid/external/glew-1.10.0/lib/mingw48_32/glew32.dll $(RELEASE)/Templates/Qt/WindowsDesktopTemplate
	cp $(ROOT)/libgid/external/openal-soft-1.13/build/mingw48_32/OpenAL32.dll $(RELEASE)/Templates/Qt/WindowsDesktopTemplate
	mkdir -p $(RELEASE)/Templates/Qt/WindowsDesktopTemplate/Plugins

qt5.install:
	for f in $(addsuffix $(QTDLLEXT),$(QT5DLLS)); do cp $(QT)/bin/$$f.dll $(RELEASE); done
	mkdir -p $(RELEASE)/imageformats
	cp $(QT)/plugins/imageformats/qjpeg.dll $(RELEASE)/imageformats
	mkdir -p $(RELEASE)/platforms
	for f in $(addsuffix $(QTDLLEXT),$(QT5PLATFORM)); do cp $(QT)/plugins/platforms/$$f.dll $(RELEASE)/platforms; done
	cp $(QT)/lib/qscintilla2.dll $(RELEASE)
	cp $(ROOT)/libgid/external/glew-1.10.0/lib/mingw48_32/glew32.dll $(RELEASE)
	cp $(ROOT)/libgid/external/openal-soft-1.13/build/mingw48_32/OpenAL32.dll $(RELEASE)
	mkdir -p $(RELEASE)/Tools
	for f in $(QT5DLLTOOLS); do cp $(QT)/bin/$$f.dll $(RELEASE)/Tools; done
	
buildqtplugins: $(addsuffix .qtplugin,$(PLUGINS_WIN))

qtplugins.clean: $(addsuffix .qtplugin.clean,$(PLUGINS_WIN))

qtplugins.install: buildqtplugins $(addsuffix .qtplugin.install,$(PLUGINS_WIN))

%.qmake.clean:
	cd $(ROOT)/$*; $(MINGWMAKE) clean

%.qmake.rel:
	cd $(ROOT)/$*; $(QMAKE) $*.pro
	cd $(ROOT)/$*; $(MINGWMAKE) release

%.qmake.dbg:
	cd $(ROOT)/$*; $(QMAKE) $*.pro
	cd $(ROOT)/$*; $(MINGWMAKE) debug

%.qmake5.rel:
	cd $(ROOT)/$*; $(QMAKE) $*_qt5.pro
	cd $(ROOT)/$*; $(MINGWMAKE) release

%.qmake5.dbg:
	cd $(ROOT)/$*; $(QMAKE) $*_qt5.pro
	cd $(ROOT)/$*; $(MINGWMAKE) debug

tools:
	cd $(ROOT)/lua514u/src; gcc -o luac $(addsuffix .c,print lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio luac lgc)
	cd $(ROOT)/lua514u/src; gcc -shared -o lua51.dll -Wl,--out-implib,lua51.a $(addsuffix .c,lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib)
	cd $(ROOT)/lua514u/src; gcc -o lua lua.c lua51.a
	#cd $(ROOT)/lua514u/src; gcc -o lua $(addsuffix .c,lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lua lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib)
	
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
	cd plugins; git archive master | tar -x -C ../$(RELEASE).Final/All\ Plugins
	-wget --recursive --no-clobber --page-requisites --html-extension --convert-links --restrict-file-names=windows --domains docs.giderosmobile.com --no-parent http://docs.giderosmobile.com/
	rm -rf $(RELEASE).Final/Documentation
	cp -R docs.giderosmobile.com $(RELEASE).Final/Documentation
	-wget "http://docs.giderosmobile.com/reference/autocomplete.php" -O $(RELEASE).Final/Resources/gideros_annot.api

bundle.win:
	cd plugins; git archive master | tar -x -C ../$(RELEASE)/All\ Plugins
	