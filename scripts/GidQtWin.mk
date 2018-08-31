buildqtapp: buildqtlibs buildqtplugins buildqt

qtapp.install: qtlibs.install qtplugins.install qt.install addons.pkg

qtapp.clean: qtlibs.clean qtplugins.clean qt.clean

QTDLLEXT?=

ifneq ($(DEBUG),)
QTTGT_EXT=dbg
QTDLLEXT=d
QTTGT_DIR=debug
else
QTTGT_EXT=rel
QTTGT_DIR=release
endif


vpath %.a libgideros/$(QTTGT_DIR):libgvfs/$(QTTGT_DIR):libgid/$(QTTGT_DIR):lua/$(QTTGT_DIR):libgid/external/openal-soft-1.13/build/mingw48_32

$(SDK)/lib/desktop/%: %
	cp $^ $(SDK)/lib/desktop
	

SDK_LIBS_QT=libgideros.a liblua.a libgid.a libgvfs.a libOpenAL32.dll.a

sdk.qtlibs.dir:
	mkdir -p $(SDK)/lib/desktop	

sdk.qtlibs: sdk.headers sdk.qtlibs.dir $(addprefix $(SDK)/lib/desktop/,$(SDK_LIBS_QT))			
			
buildqtlibs: $(addsuffix .qmake.$(QTTGT_EXT),libpystring libgvfs) libgid.qmake5.$(QTTGT_EXT) $(addsuffix .qmake.$(QTTGT_EXT),lua libgideros) sdk.qtlibs

export MSBUILD

qtlibs.install: buildqtlibs
	mkdir -p $(RELEASE)
	cp $(ROOT)/libgid/$(QTTGT_DIR)/gid.dll $(RELEASE)
	cp $(ROOT)/libgvfs/$(QTTGT_DIR)/gvfs.dll $(RELEASE)
	cp $(ROOT)/lua/$(QTTGT_DIR)/lua.dll $(RELEASE)
	cp $(ROOT)/libgideros/$(QTTGT_DIR)/gideros.dll $(RELEASE)
	cp $(ROOT)/libpystring/$(QTTGT_DIR)/pystring.dll $(RELEASE)

%.qtplugin:
	cd $(ROOT)/plugins/$*/source; if [ -d "vs" ]; then cd vs; $(MINGWMAKE); \
		else if [ -d "Desktop" ]; then cd Desktop; fi; $(QMAKE) *.pro; $(MINGWMAKE) $(QTTGT_DIR); fi 

%.qtplugin.clean:
	cd $(ROOT)/plugins/$*/source; if [ -d "vs" ]; then cd vs; elif [ -d "Desktop" ]; then cd Desktop; fi; $(MINGWMAKE) clean

%.qtplugin.install:
	mkdir -p $(RELEASE)/Plugins
	mkdir -p $(RELEASE)/Templates/Qt/WindowsDesktopTemplate/Plugins
	mkdir -p $(RELEASE)/All\ Plugins/$*/bin/Windows
	R=$(PWD); cd $(ROOT)/plugins/$*/source; if [ -d "vs" ]; then cd vs; \
		cp *.dll $$R/$(RELEASE)/Plugins; \
		cp *.dll $$R/$(RELEASE)/Templates/Qt/WindowsDesktopTemplate/Plugins; \
		cp *.dll $$R/$(RELEASE)/All\ Plugins/$*/bin/Windows;\
		else if [ -d "Desktop" ]; then cd Desktop; fi; \
		cp $(QTTGT_DIR)/*.dll $$R/$(RELEASE)/Plugins; \
		cp $(QTTGT_DIR)/*.dll $$R/$(RELEASE)/Templates/Qt/WindowsDesktopTemplate/Plugins; \
		cp $(QTTGT_DIR)/*.dll $$R/$(RELEASE)/All\ Plugins/$*/bin/Windows;\
		fi

qtlibs.clean: $(addsuffix .qmake.clean,libpystring libgvfs libgid lua libgideros)

buildqt: versioning $(addsuffix .qmake.$(QTTGT_EXT),texturepacker fontcreator ui) player.qmake5.$(QTTGT_EXT) $(addsuffix .qmake.$(QTTGT_EXT),gdrdeamon gdrbridge gdrexport desktop)

qt.clean: $(addsuffix .qmake.clean,texturepacker fontcreator ui player gdrdeamon gdrbridge gdrexport desktop)

qt.install: buildqt qt5.install qt.player tools html5.tools
	cp $(ROOT)/ui/$(QTTGT_DIR)/GiderosStudio.exe $(RELEASE)
	cp $(ROOT)/player/$(QTTGT_DIR)/GiderosPlayer.exe $(RELEASE)
	cp $(ROOT)/texturepacker/$(QTTGT_DIR)/GiderosTexturePacker.exe $(RELEASE)
	cp $(ROOT)/fontcreator/$(QTTGT_DIR)/GiderosFontCreator.exe $(RELEASE)
	cp -R $(ROOT)/ui/Resources $(RELEASE)
	cd $(ROOT)/ui/;tar cf - --exclude=Tools/lua --exclude Tools/luac --exclude Tools/make Tools | (cd ../$(RELEASE) && tar xvf - )
	cp $(ROOT)/lua/src/lua.exe $(RELEASE)/Tools
	cp $(ROOT)/lua/src/luac.exe $(RELEASE)/Tools
	cp $(ROOT)/lua/src/lua51.dll $(RELEASE)/Tools
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
		Qt5Multimedia Qt5MultimediaQuick_p Qt5MultimediaWidgets Qt5WebSockets
QT5DLLTOOLS=icudt$(QT5ICUVER) icuin$(QT5ICUVER) icuuc$(QT5ICUVER) libgcc_s_dw2-1 libstdc++-6 libwinpthread-1 \
		Qt5Core Qt5Network Qt5Xml Qt5WebSockets
QT5PLATFORM=qminimal qoffscreen qwindows
QT5PLUGINS=$(addprefix mediaservice/,dsengine qtmedia_audioengine) $(addprefix platforms/,$(QT5PLATFORM)) imageformats/qjpeg

qt.player:
	mkdir -p $(RELEASE)/Templates/Qt/WindowsDesktopTemplate
	cp $(ROOT)/desktop/release/WindowsDesktopTemplate.exe $(RELEASE)/Templates/Qt/WindowsDesktopTemplate
	for f in gid gvfs lua gideros pystring; do cp $(RELEASE)/$$f.dll $(RELEASE)/Templates/Qt/WindowsDesktopTemplate; done
	for f in $(addsuffix $(QTDLLEXT),$(QT5DLLS)); do cp $(QT)/bin/$$f.dll $(RELEASE)/Templates/Qt/WindowsDesktopTemplate; done
	mkdir -p $(addprefix $(RELEASE)/Templates/Qt/WindowsDesktopTemplate/,$(dir $(QT5PLUGINS)))
	for a in $(QT5PLUGINS); do cp $(QT)/plugins/$$a.dll$(QTDLLEXT) $(RELEASE)/Templates/Qt/WindowsDesktopTemplate/$$a.dll$(QTDLLEXT); done
	cp $(ROOT)/libgid/external/openal-soft-1.13/build/mingw48_32/OpenAL32.dll $(RELEASE)/Templates/Qt/WindowsDesktopTemplate
	mkdir -p $(RELEASE)/Templates/Qt/WindowsDesktopTemplate/Plugins

qt5.install:
	for f in $(addsuffix $(QTDLLEXT),$(QT5DLLS)); do cp $(QT)/bin/$$f.dll $(RELEASE); done
	mkdir -p $(addprefix $(RELEASE)/,$(dir $(QT5PLUGINS)))
	for a in $(QT5PLUGINS); do cp $(QT)/plugins/$$a.dll$(QTDLLEXT) $(RELEASE)/$$a.dll$(QTDLLEXT); done
	cp $(QT)/lib/qscintilla2.dll $(RELEASE)
	cp $(ROOT)/libgid/external/openal-soft-1.13/build/mingw48_32/OpenAL32.dll $(RELEASE)
	mkdir -p $(RELEASE)/Tools
	for f in $(QT5DLLTOOLS); do cp $(QT)/bin/$$f.dll $(RELEASE)/Tools; done
	
buildqtplugins: $(addsuffix .qtplugin,$(PLUGINS_WIN) $(PLUGINS_WINONLY))

qtplugins.clean: $(addsuffix .qtplugin.clean,$(PLUGINS_WIN) $(PLUGINS_WINONLY))

qtplugins.install: buildqtplugins $(addsuffix .qtplugin.install,$(PLUGINS_WIN) $(PLUGINS_WINONLY))

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
	cd $(ROOT)/lua/src; gcc -I. -DDESKTOP_TOOLS -o luac $(addsuffix .c,print lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio luac lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib lutf8lib lint64)
	cd $(ROOT)/lua/src; gcc -I. -DDESKTOP_TOOLS -shared -o lua51.dll -Wl,--out-implib,lua51.a $(addsuffix .c,lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib lutf8lib lint64)
	cd $(ROOT)/lua/src; gcc -I. -DDESKTOP_TOOLS -o lua lua.c lua51.a
	
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
	if [ -f $(WIN_SIGN) ]; then $(WIN_SIGN) sign //v //f $(WIN_KEYSTORE) //p $(WIN_KEYPASS) //t $(WIN_KEYTSS) $(ROOT)/Gideros.exe; fi
	if [ -f $(WIN_SIGN) ]; then $(WIN_SIGN) sign //v //f $(WIN_KEYSTORE) //p $(WIN_KEYPASS) //fd sha256 //tr $(WIN_KEYTSS) //td sha256 //as $(ROOT)/Gideros.exe; fi

bundle.sign:
	if [ -f $(WIN_SIGN) ]; then $(WIN_SIGN) sign //v //f $(WIN_KEYSTORE) //p $(WIN_KEYPASS) //t $(WIN_KEYTSS) $(ROOT)/Gideros.exe; fi
	if [ -f $(WIN_SIGN) ]; then $(WIN_SIGN) sign //v //f $(WIN_KEYSTORE) //p $(WIN_KEYPASS) //fd sha256 //tr $(WIN_KEYTSS) //td sha256 //as $(ROOT)/Gideros.exe; fi
