XCODEBUILD=xcodebuild
LIPO=lipo

IOS_TEMPLATE=$(RELEASE)/Templates/Xcode4/iOS\ Template/iOS\ Template
ATV_TEMPLATE=$(RELEASE)/Templates/Xcode4/iOS\ Template/AppleTV

lua.ios.libs: IOSLIBPATH=$(ROOT)/lua
gvfs.ios.libs: IOSLIBPATH=$(ROOT)/libgvfs
iosplayer.ios.libs: IOSLIBPATH=$(ROOT)/ios/iosplayer
lua.atv.libs: IOSLIBPATH=$(ROOT)/lua
gvfs.atv.libs: IOSLIBPATH=$(ROOT)/libgvfs
iosplayer.atv.libs: IOSLIBPATH=$(ROOT)/ios/iosplayer

##RULES
%.ios.libs: 
	#BUILDING $*
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk iphonesimulator$$IOS_SDK -configuration Release -project $*.xcodeproj OTHER_CFLAGS="-fembed-bitcode"
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk iphoneos$$IOS_SDK -configuration Release -project $*.xcodeproj OTHER_CFLAGS="-fembed-bitcode"
	@cd $(IOSLIBPATH); $(LIPO) build/Release-iphoneos/lib$*.a build/Release-iphonesimulator/lib$*.a -create -output lib$*.ios.a

%.atv.libs: 
	#BUILDING $*
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk appletvsimulator$$TVOS_SDK -configuration Release -project $*.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode"
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk appletvos$$TVOS_SDK -configuration Release -project $*.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode"
	@cd $(IOSLIBPATH); $(LIPO) build/Release-appletvos/lib$*.a build/Release-appletvsimulator/lib$*.a -create -output lib$*.atv.a

ios.libs: gvfs.ios.libs lua.ios.libs iosplayer.ios.libs
atv.libs: gvfs.atv.libs lua.atv.libs iosplayer.atv.libs


ios.app: player.ios.app

ios.libs.install: ios.libs
	mkdir -p $(IOS_TEMPLATE)
	cp -R $(ROOT)/ui/Templates/Xcode4/iOS\ Template/* $(IOS_TEMPLATE)/..
	cp $(ROOT)/lua/liblua.ios.a $(IOS_TEMPLATE)/liblua.a
	cp $(ROOT)/libgvfs/libgvfs.ios.a $(IOS_TEMPLATE)/libgvfs.a
	cp $(ROOT)/ios/iosplayer/libiosplayer.ios.a $(IOS_TEMPLATE)/libgideros.a
	cp $(ROOT)/ios/iosplayer/iosplayer/giderosapi.h $(IOS_TEMPLATE)

atv.libs.install: atv.libs
	mkdir -p $(ATV_TEMPLATE)
	cp $(ROOT)/lua/liblua.atv.a $(ATV_TEMPLATE)/liblua.a
	cp $(ROOT)/libgvfs/libgvfs.atv.a $(ATV_TEMPLATE)/libgvfs.a
	cp $(ROOT)/ios/iosplayer/libiosplayer.atv.a $(ATV_TEMPLATE)/libgideros.a
	cp $(ROOT)/ios/iosplayer/iosplayer/giderosapi.h $(ATV_TEMPLATE)

PLUGINS_IOS=luasocket camera
PLUGINS_ATV=luasocket

luasocket.%: PLUGINDIR=LuaSocket
camera.%: PLUGINDIR=camera

%.iosplugin: PLUGINPATH=$(ROOT)/plugins/$(PLUGINDIR)/source

%.ios.iosplugin:
	@echo $(PLUGINDIR) $(PLUGINPATH)
	cd $(PLUGINPATH); $(XCODEBUILD) -project $*.xcodeproj -alltargets -sdk iphonesimulator$$IOS_SDK -configuration Release  OTHER_CFLAGS="-fembed-bitcode"
	cd $(PLUGINPATH); $(XCODEBUILD) -project $*.xcodeproj -alltargets -sdk iphoneos$$IOS_SDK -configuration Release OTHER_CFLAGS="-fembed-bitcode"
	cd $(PLUGINPATH); $(LIPO) build/Release-iphoneos/lib$*.a build/Release-iphonesimulator/lib$*.a -create -output lib$*.ios.a


%.ios.clean.iosplugin:
	rm -rf $(PLUGINPATH)/build
	rm -f $(PLUGINPATH)/lib*.ios.a

%.ios.install.iosplugin:
	mkdir -p $(RELEASE)/All\ Plugins/$(PLUGINDIR)/bin/iOS
	cp $(PLUGINPATH)/lib$*.ios.a $(RELEASE)/All\ Plugins/$(PLUGINDIR)/bin/iOS/

%.atv.iosplugin:
	@echo $(PLUGINDIR) $(PLUGINPATH)
	@cd $(PLUGINPATH); $(XCODEBUILD) -alltargets -sdk appletvsimulator$$TVOS_SDK -configuration Release -project $*.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode"
	@cd $(PLUGINPATH); $(XCODEBUILD) -alltargets -sdk appletvos$$TVOS_SDK -configuration Release -project $*.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode"
	@cd $(PLUGINPATH); $(LIPO) build/Release-appletvos/lib$*.a build/Release-appletvsimulator/lib$*.a -create -output lib$*.atv.a

%.atv.clean.iosplugin:
	rm -rf $(PLUGINPATH)/build
	rm -f $(PLUGINPATH)/lib*.atv.a

%.atv.install.iosplugin:
	mkdir -p $(RELEASE)/All\ Plugins/$(PLUGINDIR)/bin/iOS
	cp $(PLUGINPATH)/lib$*.atv.a $(RELEASE)/All\ Plugins/$(PLUGINDIR)/bin/iOS/

ios.install: ios.libs.install atv.libs.install ios.plugins.install ios.app

ios.clean: ios.plugins.clean
		
ios.plugins: $(addsuffix .ios.iosplugin,$(PLUGINS_IOS)) $(addsuffix .atv.iosplugin,$(PLUGINS_ATV))

ios.plugins.clean: $(addsuffix .ios.clean.iosplugin,$(PLUGINS_IOS)) $(addsuffix .atv.clean.iosplugin,$(PLUGINS_ATV))

PLUGINS_IOS_DEFFILES=$(ROOT)/Sdk/include/*.h \
	$(addprefix plugins/, \
		gamekit/source/iOS/gamekit.mm	storekit/source/iOS/storekit.mm mficontroller/source/iOS/mficontroller.mm \
		iad/source/iOS/iad.mm LuaSocket/source/luasocket_stub.cpp \
		$(addprefix lsqlite3/source/,lsqlite3.c lsqlite3_stub.cpp) \
		$(addprefix lfs/source/,lfs.h lfs.c lfs_stub.cpp) \
		$(addprefix BitOp/source/,bit.c bit_stub.cpp) \
		$(addprefix JSON/source/,fpconv.c fpconv.h strbuf.c strbuf.h lua_cjson.c lua_cjson_stub.cpp) \
	)

IOS_PLAYER_DIR=$(ROOT)/ios/GiderosiOSPlayer
		
ios.plugins.install: ios.plugins $(addsuffix .ios.install.iosplugin,$(PLUGINS_IOS)) $(addsuffix .atv.install.iosplugin,$(PLUGINS_ATV))
	mkdir -p $(IOS_TEMPLATE)/Plugins
	mkdir -p $(ATV_TEMPLATE)/Plugins
	cp $(PLUGINS_IOS_DEFFILES) $(IOS_TEMPLATE)/Plugins
	cp $(PLUGINS_IOS_DEFFILES) $(ATV_TEMPLATE)/Plugins
	cp $(RELEASE)/All\ Plugins/LuaSocket/bin/iOS/libluasocket.ios.a $(IOS_TEMPLATE)/Plugins/libluasocket.a
	cp $(RELEASE)/All\ Plugins/LuaSocket/bin/iOS/libluasocket.atv.a $(ATV_TEMPLATE)/Plugins/libluasocket.a

player.ios.app: 
	rm -rf $(IOS_PLAYER_DIR)/GiderosiOSPlayer/Plugins
	cp -R $(IOS_TEMPLATE)/Plugins $(IOS_PLAYER_DIR)/GiderosiOSPlayer/
	cp $(IOS_TEMPLATE)/*.a $(IOS_PLAYER_DIR)/GiderosiOSPlayer/
	cp $(IOS_TEMPLATE)/giderosapi.h $(IOS_PLAYER_DIR)/GiderosiOSPlayer/
	mkdir -p $(RELEASE)/Players
	#cd $(IOS_PLAYER_DIR); $(XCODEBUILD) -sdk iphoneos$$IOS_SDK -configuration Release IPHONEOS_DEPLOYMENT_TARGET=6.0 -project GiderosiOSPlayer.xcodeproj -scheme GiderosiOSPlayer -archivePath GiderosiOSPlayer.xcarchive archive
	#cd $(IOS_PLAYER_DIR); $(XCODEBUILD) -exportArchive -exportPath ../../$(RELEASE)/Players/GiderosiOSPlayer.ipa -exportFormat ipa -archivePath GiderosiOSPlayer.xcarchive
	R=$(PWD);cd $(IOS_PLAYER_DIR); zip -r $$R/$(RELEASE)/Players/GiderosiOSPlayer.zip GiderosiOSPlayer 
	
