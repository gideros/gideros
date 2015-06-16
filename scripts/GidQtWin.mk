buildqtapp: buildqtlibs buildplugins buildqt

qtapp.install: qtlibs.install plugins.install qt.install

qtapp.clean: qtlibs.clean plugins.clean qt.clean


vpath %.h libgideros:libgvfs:libgid/include:lua/src:libgid/openal-soft-1.13/include
vpath %.a libgideros/release:libgvfs/release:libgid/release:lua/release:libgid/external/openal-soft-1.13/build/mingw48_32
$(SDK)/include/%: %
	cp $^ $(SDK)/include

$(SDK)/lib/desktop/%: %
	cp $^ $(SDK)/lib/desktop
	

SDK_INCS=$(addsuffix .h,gideros gplugin gproxy greferenced gexport) \
			$(addsuffix .h,gfile gpath) \
			$(addsuffix .h,gglobal glog gapplication gevent) \
			$(addsuffix .h,lua luaconf lualib lauxlib) \
			$(addprefix AL/,$(addsuffix .h,al alc alext efx efx-creative))

SDK_LIBS_QT=libgideros.a liblua.a libgid.a libgvfs.a libOpenAL32.dll.a

sdkdir.prep:
	mkdir -p $(SDK)/include/AL
	mkdir -p $(SDK)/lib/desktop	

sdkdir.headers: $(addprefix $(SDK)/include/,$(SDK_INCS))
sdkdir.qtlibs: $(addprefix $(SDK)/lib/desktop/,$(SDK_LIBS_QT))			
sdkdir: sdkdir.prep sdkdir.headers sdkdir.qtlibs
			
buildqtlibs: $(addsuffix .qmake.rel,libpystring libgvfs) libgid.qmake5.rel $(addsuffix .qmake.rel,lua libgideros) sdkdir


qtlibs.install: buildqtlibs
	mkdir -p $(RELEASE)
	cp $(ROOT)/libgid/release/gid.dll $(RELEASE)
	cp $(ROOT)/libgvfs/release/gvfs.dll $(RELEASE)
	cp $(ROOT)/lua/release/lua.dll $(RELEASE)
	cp $(ROOT)/libgideros/release/gideros.dll $(RELEASE)
	cp $(ROOT)/libpystring/release/pystring.dll $(RELEASE)

%.plugin:
	cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; $(QMAKE) *.pro
	cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; $(MINGWMAKE) release

%.plugin.clean:
	cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; $(MINGWMAKE) clean

%.plugin.install:
	mkdir -p $(RELEASE)/Plugins
	cd $(ROOT)/plugins/$*/source; if [ -d "Desktop" ]; then cd Desktop; fi; cp release/*.dll $(RELEASE)/Plugins	 

qtlibs.clean: $(addsuffix .qmake.clean,libpystring libgvfs libgid lua libgideros)

buildqt: $(addsuffix .qmake.rel,texturepacker fontcreator ui) player.qmake5.rel $(addsuffix .qmake.rel,gdrdeamon gdrbridge gdrexport)

qt.clean: $(addsuffix .qmake.clean,texturepacker fontcreator ui player gdrdeamon gdrbridge gdrexport)

qt.install: buildqt qt5.install
	cp $(ROOT)/ui/release/GiderosStudio.exe $(RELEASE)
	cp $(ROOT)/player/release/GiderosPlayer.exe $(RELEASE)
	cp $(ROOT)/texturepacker/release/GiderosTexturePacker.exe $(RELEASE)
	cp $(ROOT)/fontcreator/release/GiderosFontCreator.exe $(RELEASE)
	cp -R $(ROOT)/ui/Resources $(RELEASE)
	cp -R $(ROOT)/ui/Tools $(RELEASE)
	mkdir -p $(RELEASE)/Templates
	cp -R $(ROOT)/ui/Templates/Eclipse $(RELEASE)/Templates
	cp -R $(ROOT)/ui/Templates/Xcode4 $(RELEASE)/Templates
	mkdir -p $(RELEASE)/Templates/Eclipse/Android\ Template/assets
	mkdir -p $(RELEASE)/Templates/Eclipse/Android\ Template/gen
	mkdir -p $(RELEASE)/Templates/Eclipse/Android\ Template/res/layout
	mkdir -p $(RELEASE)/Templates/Xcode4/iOS\ Template/iOS\ Template/assets
	mkdir -p $(RELEASE)/Examples
	cp -R $(ROOT)/samplecode/* $(RELEASE)/Examples
	cp -R $(ROOT)/ios/GiderosiOSPlayer $(RELEASE)
	cp $(ROOT)/gdrdeamon/release/gdrdeamon.exe $(RELEASE)/Tools
	cp $(ROOT)/gdrbridge/release/gdrbridge.exe $(RELEASE)/Tools
	cp $(ROOT)/gdrexport/release/gdrexport.exe $(RELEASE)/Tools
	
QT5DLLS=icudt$(QT5ICUVER) icuin$(QT5ICUVER) icuuc$(QT5ICUVER) libgcc_s_dw2-1 libstdc++-6 libwinpthread-1 \
		Qt5Core Qt5Gui Qt5Network Qt5OpenGL Qt5PrintSupport Qt5Widgets Qt5Xml
QT5DLLTOOLS=icudt$(QT5ICUVER) icuin$(QT5ICUVER) icuuc$(QT5ICUVER) libgcc_s_dw2-1 libstdc++-6 libwinpthread-1 \
		Qt5Core Qt5Network Qt5Xml
QT5PLATFORM=qminimal qoffscreen qwindows

qt5.install:
	for f in $(QT5DLLS); do cp $(QT)/bin/$$f.dll $(RELEASE); done
	mkdir -p $(RELEASE)/imageformats
	cp $(QT)/plugins/imageformats/qjpeg.dll $(RELEASE)/imageformats
	mkdir -p $(RELEASE)/platforms
	for f in $(QT5PLATFORM); do cp $(QT)/plugins/platforms/$$f.dll $(RELEASE)/platforms; done
	cp $(QT)/lib/qscintilla2.dll $(RELEASE)
	cp $(ROOT)/libgid/external/glew-1.10.0/lib/mingw48_32/glew32.dll $(RELEASE)
	cp $(ROOT)/libgid/external/openal-soft-1.13/build/mingw48_32/OpenAL32.dll $(RELEASE)
	mkdir -p $(RELEASE)/Tools
	for f in $(QT5DLLTOOLS); do cp $(QT)/bin/$$f.dll $(RELEASE)/Tools; done
	
buildplugins: $(addsuffix .plugin,$(PLUGINS_WIN))

plugins.clean: $(addsuffix .plugin.clean,$(PLUGINS_WIN))

plugins.install: buildplugins $(addsuffix .plugin.install,$(PLUGINS_WIN))

%.qmake.clean:
	cd $(ROOT)/$*; $(MINGWMAKE) clean

%.qmake.rel:
	cd $(ROOT)/$*; $(QMAKE) $*.pro
	cd $(ROOT)/$*; $(MINGWMAKE) release

%.qmake5.rel:
	cd $(ROOT)/$*; $(QMAKE) $*_qt5.pro
	cd $(ROOT)/$*; $(MINGWMAKE) release

%.qmake5.dbg:
	cd $(ROOT)/$*; $(QMAKE) $*_qt5.pro
	cd $(ROOT)/$*; $(MINGWMAKE) debug

		