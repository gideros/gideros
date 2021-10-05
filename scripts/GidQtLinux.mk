DEPLOYQT=$(QT)/bin/macdeployqt

buildqtapp: buildqtlibs buildqtplugins buildqt

qtapp.install: qtlibs.install qtplugins.install qt.install

qtapp.clean: qtlibs.clean qtplugins.clean qt.clean


vpath %.so libgideros:libgvfs:libgid:lua

$(SDK)/lib/desktop/%: %
	cp -P $^* $(SDK)/lib/desktop
	

SDK_LIBS_QTLIST=libgideros liblua libgid libgvfs
SDK_LIBS_QT=$(addsuffix .so,$(SDK_LIBS_QTLIST)) $(addsuffix .so,$(SDK_LIBS_QTLIST))

sdk.qtlibs.dir:
	mkdir -p $(SDK)/lib/desktop	

sdk.qtlibs: sdk.headers sdk.qtlibs.dir $(addprefix $(SDK)/lib/desktop/,$(SDK_LIBS_QT))			
			
buildqtlibs: $(addsuffix .qmake.rel,libpystring libgvfs) libgid.qmake5.rel $(addsuffix .qmake.rel,lua libgideros) sdk.qtlibs


qtlibs.install: buildqtlibs
	mkdir -p $(RELEASE)

%.qtplugin:
	cd $(ROOT)/plugins/$*/source; if [ -d "linux" ]; then cd linux; $(MAKE) $(MAKEJOBS); \
		else if [ -d "Desktop" ]; then cd Desktop; fi; $(QMAKE) *.pro; $(MAKE) $(MAKEJOBS); fi 

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

qtlibs.clean: $(addsuffix .qmake.clean,libpystring libgvfs libgid lua libgideros)

buildqt: versioning $(addsuffix .qmake.rel,texturepacker fontcreator ui) player.qmake5.rel $(addsuffix .qmake.rel,gdrdeamon gdrbridge gdrexport desktop)

qt.clean: $(addsuffix .qmake.clean,texturepacker fontcreator ui player gdrdeamon gdrbridge gdrexport desktop)

QT5DLLS=icudata icui18n icuuc \
		Qt5Core Qt5Gui Qt5Network Qt5OpenGL Qt5PrintSupport Qt5Widgets Qt5Xml \
		Qt5XcbQpa Qt5DBus \
		Qt5Multimedia Qt5MultimediaWidgets Qt5WebSockets
QT5DLLTOOLS=icudata icui18n icuuc \
		Qt5Core Qt5Network Qt5Xml Qt5WebSockets
QT5PLATFORM=qminimal qoffscreen qxcb
QT5PLUGINS= \
	$(addprefix platforms/,$(QT5PLATFORM)) \
	$(addprefix xcbglintegrations/,qxcb-egl-integration qxcb-ogl-integration) \
	imageformats/qjpeg \
	#$(addprefix mediaservice/,dsengine qtmedia_audioengine) \


qt.install: buildqt qt.player tools html5.tools
	#STUDIO
	cp -R $(ROOT)/ui/GiderosStudio $(RELEASE)
	cp -P $(QT)/lib/libqscintilla2_qt5.so* $(RELEASE)
	cp -R $(ROOT)/ui/Resources $(RELEASE)
	-wget -nv "http://wiki.giderosmobile.com/gidapi.php" -O $(RELEASE)/Resources/gideros_annot.api	
	cp -R $(ROOT)/ui/Tools $(RELEASE)/Tools
	cp $(ROOT)/lua/src/lua $(RELEASE)/Tools
	cp $(ROOT)/lua/src/luac $(RELEASE)/Tools
	for t in gdrdeamon gdrbridge gdrexport; do \
	cp $(ROOT)/$$t/$$t $(RELEASE)/Tools; done 
	for f in $(QT5DLLS); do cp -P $(QT)/lib/lib$$f.so* $(RELEASE); done
	for f in $(QT5DLLTOOLS); do cp -P $(QT)/lib/lib$$f.so* $(RELEASE)/Tools; done
	for a in $(QT5PLUGINS); do mkdir -p $(RELEASE)/$$(dirname $$a); cp -P $(QT)/plugins/$$(dirname $$a)/lib$$(basename $$a).so* $(RELEASE)/$$(dirname $$a)/; done
	#PLAYER
	cp -R $(ROOT)/player/GiderosPlayer $(RELEASE)
	cp -P $(SDK)/lib/desktop/*.so* $(RELEASE)
	cp -P libpystring/*.so* $(RELEASE)
	#TEXTUREPACKER
	cp -R $(ROOT)/texturepacker/GiderosTexturePacker $(RELEASE)
	#FONT CREATOR
	cp -R $(ROOT)/fontcreator/GiderosFontCreator $(RELEASE)
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
	

QTDLLEXT?=

qt.player:
	mkdir -p $(RELEASE)/Templates/Qt/LinuxDesktopTemplate
	cp -R $(ROOT)/desktop/LinuxDesktopTemplate $(RELEASE)/Templates/Qt/LinuxDesktopTemplate
	cp -P $(SDK)/lib/desktop/*.so* $(RELEASE)/Templates/Qt/LinuxDesktopTemplate/
	cp -P libpystring/*.so* $(RELEASE)/Templates/Qt/LinuxDesktopTemplate
	for f in $(QT5DLLS); do cp -P $(QT)/lib/lib$$f.so* $(RELEASE)/Templates/Qt/LinuxDesktopTemplate; done
	
buildqtplugins: $(addsuffix .qtplugin,$(PLUGINS_WIN))

qtplugins.clean: $(addsuffix .qtplugin.clean,$(PLUGINS_WIN))

qtplugins.install: buildqtplugins $(addsuffix .qtplugin.install,$(PLUGINS_WIN))

%.qmake.clean:
	cd $(ROOT)/$*; if [ -f Makefile ]; then $(MAKE) clean; fi

%.qmake.rel:
	cd $(ROOT)/$*; $(QMAKE) $*.pro
	cd $(ROOT)/$*; $(MAKE) $(MAKEJOBS)

%.qmake5.rel:
	cd $(ROOT)/$*; $(QMAKE) $*_qt5.pro
	cd $(ROOT)/$*; $(MAKE) $(MAKEJOBS) 

tools:
	cd $(ROOT)/lua/src; gcc -I. -DDESKTOP_TOOLS -o luac $(addsuffix .c,print lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio luac lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib lutf8lib lint64) -lm
	cd $(ROOT)/lua/src; gcc -I. -DDESKTOP_TOOLS -o lua $(addsuffix .c,lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lua lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib lutf8lib lint64) -lm

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
	mv $(RELEASE).Final/Templates $(RELEASE).Final/Gideros\ Studio.app/Contents
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
	pkgbuild --root $(ROOT)/ROOTMAC --identifier com.giderosmobile.gideros --version $(GIDEROS_VERSION) --component-plist $(ROOT)/Release/pkg.plist $(ROOT)/Gideros-App.pkg
	security -v unlock-keychain -p $(OSX_SIGNING_PASSWORD) "$$HOME/Library/Keychains/login.keychain" && productbuild --distribution Release/GiderosDist.plist --package-path $(ROOT)/Gideros-App.pkg --sign $(OSX_SIGNING_IDENTITY) $(ROOT)/Gideros.pkg
	rm -rf $(ROOT)/ROOTMAC
	cd plugins; git archive $(CURRENT_GIT_BRANCH) | tar -x -C ../$(RELEASE)/All\ Plugins
