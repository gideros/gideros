XCODEBUILD=xcodebuild
LIPO=lipo
PRETTY=| xcpretty -c

IOS_TEMPLATE=$(RELEASE)/Templates/Xcode4/iOS\ Template/iOS\ Template
ATV_TEMPLATE=$(RELEASE)/Templates/Xcode4/iOS\ Template/AppleTV
MAC_TEMPLATE=$(RELEASE)/Templates/Xcode4/iOS\ Template/Mac

lua.ios.libs: IOSLIBPATH=$(ROOT)/lua
gvfs.ios.libs: IOSLIBPATH=$(ROOT)/libgvfs
iosplayer.ios.libs: IOSLIBPATH=$(ROOT)/ios/iosplayer
lua.ios.libs.clean: IOSLIBPATH=$(ROOT)/lua
gvfs.ios.libs.clean: IOSLIBPATH=$(ROOT)/libgvfs
iosplayer.ios.libs.clean: IOSLIBPATH=$(ROOT)/ios/iosplayer

lua.atv.libs: IOSLIBPATH=$(ROOT)/lua
gvfs.atv.libs: IOSLIBPATH=$(ROOT)/libgvfs
iosplayer.atv.libs: IOSLIBPATH=$(ROOT)/ios/iosplayer
lua.atv.libs.clean: IOSLIBPATH=$(ROOT)/lua
gvfs.atv.libs.clean: IOSLIBPATH=$(ROOT)/libgvfs
iosplayer.atv.libs.clean: IOSLIBPATH=$(ROOT)/ios/iosplayer

lua.mac.libs: IOSLIBPATH=$(ROOT)/lua
gvfs.mac.libs: IOSLIBPATH=$(ROOT)/libgvfs
iosplayer.mac.libs: IOSLIBPATH=$(ROOT)/ios/iosplayer
lua.mac.libs.clean: IOSLIBPATH=$(ROOT)/lua
gvfs.mac.libs.clean: IOSLIBPATH=$(ROOT)/libgvfs
iosplayer.mac.libs.clean: IOSLIBPATH=$(ROOT)/ios/iosplayer

##RULES
%.ios.libs: 
	#BUILDING $*
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk iphonesimulator$$IOS_SDK -configuration Release -arch x86_64 -project $*.xcodeproj $(PRETTY)
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk iphoneos$$IOS_SDK -configuration Release -project $*.xcodeproj OTHER_CFLAGS="-fembed-bitcode" $(PRETTY)
	@cd $(IOSLIBPATH); $(LIPO) build/Release-iphoneos/lib$*.a build/Release-iphonesimulator/lib$*.a -create -output lib$*.ios.a

%.ios.libs.clean:
	cd $(IOSLIBPATH); rm -rf build

%.atv.libs: 
	#BUILDING $*
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk appletvsimulator$$TVOS_SDK -configuration Release -arch x86_64 -project $*.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' $(PRETTY)
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk appletvos$$TVOS_SDK -configuration Release -project $*.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode" $(PRETTY)
	@cd $(IOSLIBPATH); $(LIPO) build/Release-appletvos/lib$*.a build/Release-appletvsimulator/lib$*.a -create -output lib$*.atv.a

%.mac.libs: 
	#BUILDING $*
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk macosx$$MACOSX_SDK -configuration Release -project $*.xcodeproj OTHER_CFLAGS="-fembed-bitcode"
	@cd $(IOSLIBPATH); cp build/Release/lib$*.a lib$*.mac.a

ios.libs: versioning  gvfs.ios.libs lua.ios.libs iosplayer.ios.libs
ios.libs.clean : gvfs.ios.libs.clean lua.ios.libs.clean iosplayer.ios.libs.clean
atv.libs: versioning gvfs.atv.libs lua.atv.libs iosplayer.atv.libs
atv.libs.clean : gvfs.atv.libs.clean lua.atv.libs.clean iosplayer.atv.libs.clean
mac.libs: versioning gvfs.mac.libs lua.mac.libs iosplayer.mac.libs
mac.libs.clean : gvfs.mac.libs.clean lua.mac.libs.clean iosplayer.mac.libs.clean


ios.app: player.ios.app

ios.libs.install: ios.libs
	mkdir -p $(IOS_TEMPLATE)
	cp -R $(ROOT)/ui/Templates/Xcode4/iOS\ Template/* $(IOS_TEMPLATE)/..
	cp $(ROOT)/lua/liblua.ios.a $(IOS_TEMPLATE)/liblua.a
	cp $(ROOT)/libgvfs/libgvfs.ios.a $(IOS_TEMPLATE)/libgvfs.a
	cp $(ROOT)/ios/iosplayer/libiosplayer.ios.a $(IOS_TEMPLATE)/libgideros.a
	cp $(ROOT)/ios/iosplayer/build/Release-iphoneos/default.metallib $(IOS_TEMPLATE)
	cp $(ROOT)/ios/iosplayer/iosplayer/giderosapi.h $(IOS_TEMPLATE)

atv.libs.install: atv.libs
	mkdir -p $(ATV_TEMPLATE)
	cp $(ROOT)/lua/liblua.atv.a $(ATV_TEMPLATE)/liblua.a
	cp $(ROOT)/libgvfs/libgvfs.atv.a $(ATV_TEMPLATE)/libgvfs.a
	cp $(ROOT)/ios/iosplayer/libiosplayer.atv.a $(ATV_TEMPLATE)/libgideros.a
	cp $(ROOT)/ios/iosplayer/build/Release-appletvos/default.metallib $(ATV_TEMPLATE)
	cp $(ROOT)/ios/iosplayer/iosplayer/giderosapi.h $(ATV_TEMPLATE)

mac.libs.install: mac.libs
	mkdir -p $(MAC_TEMPLATE)
	cp $(ROOT)/lua/liblua.mac.a $(MAC_TEMPLATE)/liblua.a
	cp $(ROOT)/libgvfs/libgvfs.mac.a $(MAC_TEMPLATE)/libgvfs.a
	cp $(ROOT)/ios/iosplayer/libiosplayer.mac.a $(MAC_TEMPLATE)/libgideros.a
	cp $(ROOT)/ios/iosplayer/build/Release/default.metallib $(MAC_TEMPLATE)
	cp $(ROOT)/ios/iosplayer/iosplayer/giderosapi.h $(MAC_TEMPLATE)

luasocket.%: PLUGINDIR=LuaSocket

%.iosplugin: PLUGINDIR?=$*
%.iosplugin: PLUGINPATH=$(ROOT)/plugins/$(PLUGINDIR)/source

%.ios.iosplugin:
	@echo $(PLUGINDIR) $(PLUGINPATH)
	@cd $(PLUGINPATH); $(XCODEBUILD) -project $(notdir $*).xcodeproj -alltargets -sdk iphonesimulator$$IOS_SDK -configuration Release -arch x86_64 OTHER_CFLAGS="-fembed-bitcode" $(PRETTY)
	@cd $(PLUGINPATH); $(XCODEBUILD) -project $(notdir $*).xcodeproj -alltargets -sdk iphoneos$$IOS_SDK -configuration Release OTHER_CFLAGS="-fembed-bitcode" $(PRETTY)
	@cd $(PLUGINPATH); $(LIPO) build/Release-iphoneos/lib$(notdir $*).a build/Release-iphonesimulator/lib$(notdir $*).a -create -output lib$(notdir $*).ios.a


%.ios.clean.iosplugin:
	rm -rf $(PLUGINPATH)/build
	rm -f $(PLUGINPATH)/lib*.ios.a

%.ios.install.iosplugin:
	mkdir -p $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS
	cp $(PLUGINPATH)/lib$(notdir $*).ios.a $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS/

%.atv.iosplugin:
	@echo $(PLUGINDIR) $(PLUGINPATH)
	@cd $(PLUGINPATH); $(XCODEBUILD) -alltargets -sdk appletvsimulator$$TVOS_SDK -configuration Release -arch x86_64 -project $(notdir $*).xcodeproj GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode" $(PRETTY)
	@cd $(PLUGINPATH); $(XCODEBUILD) -alltargets -sdk appletvos$$TVOS_SDK -configuration Release -project $(notdir $*).xcodeproj GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode" $(PRETTY)
	@cd $(PLUGINPATH); $(LIPO) build/Release-appletvos/lib$(notdir $*).a build/Release-appletvsimulator/lib$(notdir $*).a -create -output lib$(notdir $*).atv.a

%.atv.clean.iosplugin:
	rm -rf $(PLUGINPATH)/build
	rm -f $(PLUGINPATH)/lib*.atv.a

%.atv.install.iosplugin:
	mkdir -p $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS
	cp $(PLUGINPATH)/lib$(notdir $*).atv.a $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS/

%.mac.iosplugin:
	@echo $(PLUGINDIR) $(PLUGINPATH)
	@cd $(PLUGINPATH); $(XCODEBUILD) -project $*.xcodeproj -alltargets -sdk macosx$$MACOSX_SDK -configuration Release OTHER_CFLAGS="-fembed-bitcode"
	@cd $(PLUGINPATH); cp build/Release/lib$*.a lib$*.mac.a


%.mac.clean.iosplugin:
	rm -rf $(PLUGINPATH)/build
	rm -f $(PLUGINPATH)/lib*.mac.a

%.mac.install.iosplugin:
	mkdir -p $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/mac
	cp $(PLUGINPATH)/lib$(notdir $*).mac.a $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/mac/

ios.install: ios.libs.install atv.libs.install mac.libs.install ios.plugins.install ios.app

ios.clean: ios.plugins.clean ios.libs.clean
		
ios.plugins: $(addsuffix .ios.iosplugin,$(PLUGINS_IOS)) $(addsuffix .atv.iosplugin,$(PLUGINS_ATV)) $(addsuffix .mac.iosplugin,$(PLUGINS_MAC))

ios.plugins.clean: $(addsuffix .ios.clean.iosplugin,$(PLUGINS_IOS)) $(addsuffix .atv.clean.iosplugin,$(PLUGINS_ATV)) $(addsuffix .mac.clean.iosplugin,$(PLUGINS_MAC))

PLUGINS_IOS_DEFFILES=$(ROOT)/Sdk/include/*.h
PLUGINS_IOS_PLAYER=$(addprefix plugins/, \
		LuaSocket/source/luasocket_stub.cpp \
		$(addprefix lsqlite3/source/,lsqlite3.c lsqlite3_stub.cpp) \
		$(addprefix lfs/source/,lfs.h lfs.cpp lfs_stub.cpp) \
		$(addprefix BitOp/source/,bit.c bit_stub.cpp) \
		$(addprefix JSON/source/,fpconv.c fpconv.h strbuf.c strbuf.h lua_cjson.c lua_cjson_stub.cpp) \
		gamekit/source/iOS/gamekit.mm \
		storekit/source/iOS/storekit.mm \
		iad/source/iOS/iad.mm \
		mficontroller/source/iOS/mficontroller.mm \
	)


IOS_PLAYER_DIR=$(ROOT)/ios/GiderosiOSPlayer
		
ios.plugins.install: ios.plugins $(addsuffix .ios.install.iosplugin,$(PLUGINS_IOS)) $(addsuffix .atv.install.iosplugin,$(PLUGINS_ATV)) $(addsuffix .mac.install.iosplugin,$(PLUGINS_MAC))
	mkdir -p $(IOS_TEMPLATE)/Plugins
	mkdir -p $(ATV_TEMPLATE)/Plugins
	mkdir -p $(MAC_TEMPLATE)/Plugins
	cp $(PLUGINS_IOS_DEFFILES) $(IOS_TEMPLATE)/Plugins
	cp $(PLUGINS_IOS_DEFFILES) $(ATV_TEMPLATE)/Plugins
	cp $(PLUGINS_IOS_DEFFILES) $(MAC_TEMPLATE)/Plugins

player.ios.app: 
	rm -rf $(IOS_PLAYER_DIR)/GiderosiOSPlayer/Plugins
	cp -R $(IOS_TEMPLATE)/Plugins $(IOS_PLAYER_DIR)/GiderosiOSPlayer/
	cp $(PLUGINS_IOS_PLAYER) $(IOS_TEMPLATE)/Plugins
	cp $(RELEASE)/All\ Plugins/LuaSocket/bin/iOS/libluasocket.ios.a $(IOS_PLAYER_DIR)/GiderosiOSPlayer/Plugins/libluasocket.a
	cp $(IOS_TEMPLATE)/*.a $(IOS_PLAYER_DIR)/GiderosiOSPlayer/
	cp $(IOS_TEMPLATE)/*.metallib $(IOS_PLAYER_DIR)/GiderosiOSPlayer/
	cp $(IOS_TEMPLATE)/giderosapi.h $(IOS_PLAYER_DIR)/GiderosiOSPlayer/
	mkdir -p $(RELEASE)/Players
	#cd $(IOS_PLAYER_DIR); $(XCODEBUILD) -sdk iphoneos$$IOS_SDK -configuration Release IPHONEOS_DEPLOYMENT_TARGET=6.0 -project GiderosiOSPlayer.xcodeproj -scheme GiderosiOSPlayer -archivePath GiderosiOSPlayer.xcarchive archive
	#cd $(IOS_PLAYER_DIR); $(XCODEBUILD) -exportArchive -exportPath ../../$(RELEASE)/Players/GiderosiOSPlayer.ipa -exportFormat ipa -archivePath GiderosiOSPlayer.xcarchive
	R=$(PWD);cd $(IOS_PLAYER_DIR)/..; zip -r $$R/$(RELEASE)/Players/GiderosiOSPlayer.zip GiderosiOSPlayer 
	
