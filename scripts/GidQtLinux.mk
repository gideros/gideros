LINUX_SYSLIBS?=/usr/lib/x86_64-linux-gnu

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

vpath %.so libgideros:libgvfs:libgid:libgid/openal:$(LUA_ENGINE)

$(SDK)/lib/desktop/%: %
	cp -P $^* $(SDK)/lib/desktop
	

SDK_LIBS_QTLIST=libgideros liblua libgid libgvfs libopenal
SDK_LIBS_QT=$(addsuffix .so,$(SDK_LIBS_QTLIST))

sdk.qtlibs.dir:
	mkdir -p $(SDK)/lib/desktop	

sdk.qtlibs: sdk.headers sdk.qtlibs.dir $(addprefix $(SDK)/lib/desktop/,$(SDK_LIBS_QT))			
			
buildqtlibs: $(addsuffix .qmake.rel,libpystring libgvfs libgid/xmp libgid/openal) libgid.qmake5.rel $(addsuffix .qmake.rel,$(LUA_ENGINE) libgideros) sdk.qtlibs

qtlibs.clean: $(addsuffix .qmake.clean,libpystring libgvfs libgid/xmp libgid/openal libgid $(LUA_ENGINE) libgideros)

qtlibs.install: buildqtlibs
	mkdir -p $(RELEASE)
	cp $(ROOT)/libgid/libgid.so* $(RELEASE)
	cp $(ROOT)/libgvfs/libgvfs.so* $(RELEASE)
	cp $(ROOT)/$(LUA_ENGINE)/liblua.so* $(RELEASE)
	cp $(ROOT)/libgideros/libgideros.so* $(RELEASE)
	cp $(ROOT)/libpystring/libpystring.so* $(RELEASE)

qscintilla:
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(QMAKE) ScintillaEdit.pro
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(MAKE) $(MAKEJOBS)
	mkdir -p $(ROOT)/qtinc/ScintillaEdit
	cp scintilla/include/*.h scintilla/src/*.h scintilla/qt/ScintillaEdit/*.h scintilla/qt/ScintillaEditBase/*.h $(ROOT)/qtinc/ScintillaEdit

qlexilla:
	cd $(ROOT)/lexilla/src; $(QMAKE) Lexilla.pro
	cd $(ROOT)/lexilla/src; $(MAKE) $(MAKEJOBS)
	mkdir -p $(ROOT)/qtinc/Lexilla
	cp lexilla/include/*.h $(ROOT)/qtinc/Lexilla

qscintilla.debug:
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(QMAKE) ScintillaEdit.pro
	cd $(ROOT)/scintilla/qt/ScintillaEdit; $(MAKE) $(MAKEJOBS) debug

qlexilla.debug:
	cd $(ROOT)/lexilla/src; $(QMAKE) Lexilla.pro
	cd $(ROOT)/lexilla/src; $(MAKE) $(MAKEJOBS) debug

%.qtplugin:
	cd $(ROOT)/plugins/$*/source; if [ -d "qtlinux" ]; then cd qtlinux; $(MAKE) $(MAKEJOBS); \
		else if [ -d "Desktop" ]; then cd Desktop; fi; $(QMAKE) *.pro; $(MAKE) $(MAKEJOBS) $(QTTGT_DIR); fi 

%.qtplugin.clean:
	cd $(ROOT)/plugins/$*/source; if [ -d "qtlinux" ]; then cd qtlinux; elif [ -d "Desktop" ]; then cd Desktop; fi; if [ -f Makefile ]; then $(MAKE) clean; fi

%.qtplugin.install:
	mkdir -p $(RELEASE)/Plugins
	mkdir -p $(RELEASE)/Templates/Qt/LinuxDesktopTemplate/Plugins
	mkdir -p $(RELEASE)/All\ Plugins/$*/bin/QtLinux
	R=`pwd`; cd $(ROOT)/plugins/$*/source; if [ -d "qtlinux" ]; then cd qtlinux; \
		else if [ -d "Desktop" ]; then cd Desktop; fi; fi; \
	cp -P *.so* $$R/$(RELEASE)/Plugins; \
	cp -P *.so* $$R/$(RELEASE)/Templates/Qt/LinuxDesktopTemplate/Plugins; \
	cp -P *.so* $$R/$(RELEASE)/All\ Plugins/$*/bin/QtLinux	

qtlibs.clean: $(addsuffix .qmake.clean,libpystring libgvfs libgid lua libgideros)

buildqt: versioning $(addsuffix .qmake.$(QTTGT_EXT),texturepacker fontcreator ui) player.qmake5.$(QTTGT_EXT) $(addsuffix .qmake.$(QTTGT_EXT),gdrdeamon gdrbridge gdrexport desktop)

qt.clean: qtlibs.clean $(addsuffix .qmake.clean,texturepacker fontcreator ui player gdrdeamon gdrbridge gdrexport desktop) html5.tools.clean

QT5DLLS=icudata icui18n icuuc \
		Qt6Core Qt6Gui Qt6Network Qt6OpenGL Qt6OpenGLWidgets Qt6PrintSupport Qt6Widgets Qt6Xml \
		Qt6XcbQpa Qt6DBus \
		Qt6Multimedia Qt6MultimediaQuick Qt6MultimediaWidgets Qt6WebSockets Qt6Core5Compat
QT5DLLTOOLS=icudata icui18n icuuc \
		Qt6Core Qt6Network Qt6Xml Qt6WebSockets
QT5PLATFORM=qminimal qoffscreen qxcb qlinuxfb
QT5PLUGINS= \
	$(addprefix platforms/,$(QT5PLATFORM)) \
	$(addprefix tls/,qopensslbackend) \
	$(addprefix xcbglintegrations/,qxcb-egl-integration qxcb-glx-integration) \
	imageformats/qjpeg \
	#$(addprefix mediaservice/,dsengine qtmedia_audioengine)
	
QT5DEPSDLLS=b2.so double-conversion.so md4c.so pcre2-16.so

qt.install: buildqt qt.player tools html5.tools
	#STUDIO
	cp -R $(ROOT)/ui/GiderosStudio $(RELEASE)
	cp -P $(ROOT)/scintilla/qt/ScintillaEdit/libScintillaEdit.so* $(RELEASE)
	cp -P $(ROOT)/lexilla/src/libLexilla.so* $(RELEASE)

	cp -R $(ROOT)/ui/Resources $(RELEASE)
	-wget -nv "http://wiki.giderosmobile.com/gidapi.php" -O $(RELEASE)/Resources/gideros_annot.api	
	cp -R $(ROOT)/ui/Tools $(RELEASE)/Tools
	#cp $(ROOT)/lua/src/lua $(RELEASE)/Tools
	#cp $(ROOT)/lua/src/luac $(RELEASE)/Tools
	cp $(BUILDTOOLS)/lua $(RELEASE)/Tools
	cp $(BUILDTOOLS)/luac $(RELEASE)/Tools
	cp $(BUILDTOOLS)/luauc $(RELEASE)/Tools
	for t in gdrdeamon gdrbridge gdrexport; do \
	cp $(ROOT)/$$t/$$t $(RELEASE)/Tools; done 
	for f in $(QT5DLLS); do cp -P $(QTLIBS)/lib$$f.so* $(RELEASE); done
	for f in $(QT5DLLTOOLS); do cp -P $(QTLIBS)/lib$$f.so* $(RELEASE)/Tools; done
	for a in $(QT5PLUGINS); do mkdir -p $(RELEASE)/$$(dirname $$a); cp -P $(QTPLUGINS)/$$(dirname $$a)/lib$$(basename $$a).so* $(RELEASE)/$$(dirname $$a)/; done
	for f in $(QT5DEPSDLLS); do cp -P $(LINUX_SYSLIBS)/lib$$f* $(RELEASE); done
	#PLAYER
	cp -R $(ROOT)/player/GiderosPlayer $(RELEASE)
	cp -P $(SDK)/lib/desktop/*.so* $(RELEASE)
	cp -P libpystring/*.so* $(RELEASE)
	#TEXTUREPACKER
	cp -R $(ROOT)/texturepacker/GiderosTexturePacker $(RELEASE)
	#FONT CREATOR
	cp -R $(ROOT)/fontcreator/GiderosFontCreator $(RELEASE)
	mkdir -p $(RELEASE)/Templates
	#EXTRAS
	cp $(ROOT)/Release/LinuxExtra/* $(RELEASE)
	chmod +x $(RELEASE)/*.sh
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
	-cd plugins; git archive $(CURRENT_GIT_BRANCH) | tar -x -C ../$(RELEASE)/All\ Plugins
	

QTDLLEXT?=

qt.player:
	mkdir -p $(RELEASE)/Templates/Qt/LinuxDesktopTemplate
	cp -R $(ROOT)/desktop/LinuxDesktopTemplate $(RELEASE)/Templates/Qt/LinuxDesktopTemplate
	cp -P $(SDK)/lib/desktop/*.so* $(RELEASE)/Templates/Qt/LinuxDesktopTemplate/
	cp -P libpystring/*.so* $(RELEASE)/Templates/Qt/LinuxDesktopTemplate
	for f in $(QT5DLLS); do cp -P $(QTLIBS)/lib$$f.so* $(RELEASE)/Templates/Qt/LinuxDesktopTemplate; done
	for a in $(QT5PLUGINS); do mkdir -p $(RELEASE)/Templates/Qt/LinuxDesktopTemplate/$$(dirname $$a); cp -P $(QTPLUGINS)/$$(dirname $$a)/lib$$(basename $$a).so* $(RELEASE)/Templates/Qt/LinuxDesktopTemplate/$$(dirname $$a)/; done
	for f in $(QT5DEPSDLLS); do cp -P $(LINUX_SYSLIBS)/lib$$f* $(RELEASE)/Templates/Qt/LinuxDesktopTemplate; done
	
buildqtplugins: 
	$(SUBMAKE) $(addsuffix .qtplugin,$(PLUGINS_QT) $(PLUGINS_QTLINUXONLY))

qtplugins.clean: 
	$(SUBMAKE)  $(addsuffix .qtplugin.clean,$(PLUGINS_QT) $(PLUGINS_QTLINUXONLY)) 

qtplugins.install: buildqtplugins 
	$(SUBMAKE)  $(addsuffix .qtplugin.install,$(PLUGINS_QT) $(PLUGINS_QTLINUXONLY))

%.qmake.clean:
	cd $(ROOT)/$*; if [ -f Makefile ]; then $(MAKE) clean; fi

%.qmake.rel:
	cd $(ROOT)/$*; $(QMAKE) $(notdir $*).pro
	cd $(ROOT)/$*; $(MAKE) $(MAKEJOBS)

%.qmake5.rel:
	cd $(ROOT)/$*; $(QMAKE) $(notdir $*)_qt5.pro
	cd $(ROOT)/$*; $(MAKE) $(MAKEJOBS) 

tools:
	mkdir -p $(BUILDTOOLS)
	cd $(ROOT)/luau; g++ -std=c++17 -Wno-attributes -IVM/include -ICompiler/include -IAst/include -DNO_CODEGEN -Iextern -ICommon/include -Iextern/isocline/include -DDESKTOP_TOOLS -o../$(BUILDTOOLS)/luauc $(addsuffix .cpp,\
		$(addprefix CLI/,Coverage FileUtils Flags Profiler Repl ReplEntry) \
		$(addprefix VM/src/,lapi laux lbaselib lbitlib lbuiltins lcorolib ldblib ldebug ldo lfunc lgc\
    	lgcdebug linit lint64lib liolib lmathlib lmem lnumprint lobject loslib lperf lstate lstring lstrlib ltable ltablib ltm\
        ludata lutf8lib lvmexecute lvmload lvmutils) \
		$(addprefix Compiler/src/,Builtins BuiltinFolding BytecodeBuilder ConstantFolding Compiler CostModel lcode PseudoCode TableShape ValueTracking) \
		$(addprefix Ast/src/,Ast Confusables Lexer Location Parser StringUtils TimeTrace)) extern/isocline/src/isocline.c -lpthread -lm

	cd $(ROOT)/lua/src; gcc -I. -DDESKTOP_TOOLS -o ../../$(BUILDTOOLS)/luac $(addsuffix .c,print lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio luac lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib lutf8lib lint64) -lm
	cd $(ROOT)/lua/src; gcc -I. -DDESKTOP_TOOLS -o ../../$(BUILDTOOLS)/lua $(addsuffix .c,lapi lauxlib lcode ldebug ldo ldump\
			 lfunc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lua lgc\
			 linit lbaselib ldblib liolib lmathlib loslib ltablib lstrlib loadlib lutf8lib lint64) -lm
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
	cd $(RELEASE).Final; if [ -f ../$(notdir $(RELEASE))/BuildMac.zip ]; then unzip -o ../$(notdir $(RELEASE))/BuildMac.zip; fi
	#Use our local version of that file due to line endings change
	cp $(ROOT)/ui/Templates/AndroidStudio/Android\ Template/gradlew $(RELEASE).Final/Templates/AndroidStudio/Android\ Template
	cd plugins; git archive master | tar -x -C ../$(RELEASE).Final/All\ Plugins

bundle.linux:
	cd plugins; git archive $(CURRENT_GIT_BRANCH) | tar -x -C ../$(RELEASE).Final/All\ Plugins
	cd plugins; git archive master | tar -x -C ../$(RELEASE)/All\ Plugins

bundle.installer: bundle
	rm -rf $(ROOT)/ROOTLINUX
	mkdir  -p $(ROOT)/ROOTLINUX
	mv $(RELEASE).Final $(ROOT)/ROOTLINUX/Gideros\ Studio
	rm -f ../Gideros.tar.xz
	cd $(ROOT)/ROOTLINUX; tar -cJf ../Gideros.tar.xz Gideros\ Studio
	rm -rf $(ROOT)/ROOTLINUX
