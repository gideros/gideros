XCODEBUILD=xcodebuild
LIPO=lipo
PRETTY=| xcpretty -c

XCODE_CONFIG?=Release

IOS_TEMPLATE=$(RELEASE)/Templates/Xcode4/iOS\ Template/iOS\ Template
ATV_TEMPLATE=$(RELEASE)/Templates/Xcode4/iOS\ Template/AppleTV
MAC_TEMPLATE=$(RELEASE)/Templates/Xcode4/iOS\ Template/Mac

iosplayer.%.libs: IOSLUA_INCLUDES=$(addprefix ../../,$(LUA_INCLUDE) $(LUA_INCLUDE_CORE))
lua.ios.libs: IOSLIBPATH=$(ROOT)/$(LUA_ENGINE)
gvfs.ios.libs: IOSLIBPATH=$(ROOT)/libgvfs
iosplayer.ios.libs: IOSLIBPATH=$(ROOT)/ios/iosplayer
openal.ios.libs: IOSLIBPATH=$(ROOT)/ios/iosplayer
lua.ios.libs.clean: IOSLIBPATH=$(ROOT)/$(LUA_ENGINE)
gvfs.ios.libs.clean: IOSLIBPATH=$(ROOT)/libgvfs
iosplayer.ios.libs.clean: IOSLIBPATH=$(ROOT)/ios/iosplayer
open.ios.libs.clean: IOSLIBPATH=$(ROOT)/ios/iosplayer

lua.atv.libs: IOSLIBPATH=$(ROOT)/$(LUA_ENGINE)
gvfs.atv.libs: IOSLIBPATH=$(ROOT)/libgvfs
iosplayer.atv.libs: IOSLIBPATH=$(ROOT)/ios/iosplayer
openal.atv.libs: IOSLIBPATH=$(ROOT)/ios/iosplayer
lua.atv.libs.clean: IOSLIBPATH=$(ROOT)/$(LUA_ENGINE)
gvfs.atv.libs.clean: IOSLIBPATH=$(ROOT)/libgvfs
iosplayer.atv.libs.clean: IOSLIBPATH=$(ROOT)/ios/iosplayer
openal.atv.libs.clean: IOSLIBPATH=$(ROOT)/ios/iosplayer

lua.mac.libs: IOSLIBPATH=$(ROOT)/$(LUA_ENGINE)
gvfs.mac.libs: IOSLIBPATH=$(ROOT)/libgvfs
iosplayer.mac.libs: IOSLIBPATH=$(ROOT)/ios/iosplayer
openal.mac.libs: IOSLIBPATH=$(ROOT)/ios/iosplayer
iosplayer.mac.dbg.libs: IOSLIBPATH=$(ROOT)/ios/iosplayer
lua.mac.libs.clean: IOSLIBPATH=$(ROOT)/$(LUA_ENGINE)
gvfs.mac.libs.clean: IOSLIBPATH=$(ROOT)/libgvfs
iosplayer.mac.libs.clean: IOSLIBPATH=$(ROOT)/ios/iosplayer
openal.mac.libs.clean: IOSLIBPATH=$(ROOT)/ios/iosplayer

##RULES
%.ios.libs: 
	#BUILDING $*
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk iphonesimulator$$IOS_SDK -configuration $(XCODE_CONFIG) -arch x86_64 -project $*.xcodeproj LUA_INCLUDES="$(IOSLUA_INCLUDES)" $(PRETTY)
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk iphoneos$$IOS_SDK -configuration $(XCODE_CONFIG) -project $*.xcodeproj LUA_INCLUDES="$(IOSLUA_INCLUDES)" OTHER_CFLAGS="-fembed-bitcode" $(PRETTY)
	@cd $(IOSLIBPATH); $(LIPO) build/$(XCODE_CONFIG)-iphoneos/lib$*.a build/$(XCODE_CONFIG)-iphonesimulator/lib$*.a -create -output lib$*.ios.a

%.ios.libs.clean:
	cd $(IOSLIBPATH); rm -rf build

%.atv.libs: 
	#BUILDING $*
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk appletvsimulator$$TVOS_SDK -configuration $(XCODE_CONFIG) -arch x86_64 -project $*.xcodeproj LUA_INCLUDES="$(IOSLUA_INCLUDES)" GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' $(PRETTY)
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk appletvos$$TVOS_SDK -configuration $(XCODE_CONFIG) -project $*.xcodeproj LUA_INCLUDES="$(IOSLUA_INCLUDES)" GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode" $(PRETTY)
	@cd $(IOSLIBPATH); $(LIPO) build/$(XCODE_CONFIG)-appletvos/lib$*.a build/$(XCODE_CONFIG)-appletvsimulator/lib$*.a -create -output lib$*.atv.a

%.mac.libs: 
	#BUILDING $*
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk macosx$$MACOSX_SDK -configuration $(XCODE_CONFIG) -project $*.xcodeproj LUA_INCLUDES="$(IOSLUA_INCLUDES)" OTHER_CFLAGS="-fembed-bitcode" $(PRETTY)
	@cd $(IOSLIBPATH); cp build/$(XCODE_CONFIG)/lib$*.a lib$*.mac.a


ios.libs: gvfs.ios.libs lua.ios.libs iosplayer.ios.libs openal.ios.libs
ios.libs.clean : gvfs.ios.libs.clean lua.ios.libs.clean iosplayer.ios.libs.clean openal.ios.libs.clean
atv.libs: gvfs.atv.libs lua.atv.libs iosplayer.atv.libs openal.atv.libs
atv.libs.clean : gvfs.atv.libs.clean lua.atv.libs.clean iosplayer.atv.libs.clean openal.atv.libs.clean
mac.libs: gvfs.mac.libs lua.mac.libs iosplayer.mac.libs openal.mac.libs
mac.libs.clean : gvfs.mac.libs.clean lua.mac.libs.clean iosplayer.mac.libs.clean openal.mac.libs.clean


ios.app: player.ios.app

ios.libs.install: ios.libs
	mkdir -p $(IOS_TEMPLATE)
	cp -R $(ROOT)/ui/Templates/Xcode4/iOS\ Template/* $(IOS_TEMPLATE)/..
	cp $(ROOT)/$(LUA_ENGINE)/liblua.ios.a $(IOS_TEMPLATE)/liblua.a
	cp $(ROOT)/libgvfs/libgvfs.ios.a $(IOS_TEMPLATE)/libgvfs.a
	cp $(ROOT)/ios/iosplayer/libiosplayer.ios.a $(IOS_TEMPLATE)/libgideros.a
	cp $(ROOT)/ios/iosplayer/libopenal.ios.a $(IOS_TEMPLATE)/libopenal.a
	cp $(ROOT)/ios/iosplayer/build/Release-iphoneos/default.metallib $(IOS_TEMPLATE)
	cp $(ROOT)/ios/iosplayer/build/Release-iphonesimulator/default.metallib $(IOS_TEMPLATE)/default-sim.metallib
	cp $(ROOT)/ios/iosplayer/iosplayer/giderosapi.h $(IOS_TEMPLATE)

atv.libs.install: atv.libs
	mkdir -p $(ATV_TEMPLATE)
	cp $(ROOT)/$(LUA_ENGINE)/liblua.atv.a $(ATV_TEMPLATE)/liblua.a
	cp $(ROOT)/libgvfs/libgvfs.atv.a $(ATV_TEMPLATE)/libgvfs.a
	cp $(ROOT)/ios/iosplayer/libiosplayer.atv.a $(ATV_TEMPLATE)/libgideros.a
	cp $(ROOT)/ios/iosplayer/libopenal.atv.a $(ATV_TEMPLATE)/libopenal.a
	cp $(ROOT)/ios/iosplayer/build/Release-appletvos/default.metallib $(ATV_TEMPLATE)
	cp $(ROOT)/ios/iosplayer/build/Release-appletvsimulator/default.metallib $(ATV_TEMPLATE)/default-sim.metallib
	cp $(ROOT)/ios/iosplayer/iosplayer/giderosapi.h $(ATV_TEMPLATE)

mac.libs.install: mac.libs
	mkdir -p $(MAC_TEMPLATE)
	cp $(ROOT)/$(LUA_ENGINE)/liblua.mac.a $(MAC_TEMPLATE)/liblua.a
	cp $(ROOT)/libgvfs/libgvfs.mac.a $(MAC_TEMPLATE)/libgvfs.a
	cp $(ROOT)/ios/iosplayer/libiosplayer.mac.a $(MAC_TEMPLATE)/libgideros.a
	cp $(ROOT)/ios/iosplayer/libopenal.mac.a $(MAC_TEMPLATE)/libopenal.a
	cp $(ROOT)/ios/iosplayer/build/Release/default.metallib $(MAC_TEMPLATE)
	cp $(ROOT)/ios/iosplayer/iosplayer/giderosapi.h $(MAC_TEMPLATE)

luasocket.%: PLUGINDIR=LuaSocket

%.iosplugin: PLUGINDIR?=$*
%.iosplugin: PLUGINPATH=$(ROOT)/plugins/$(PLUGINDIR)/source

%.ios.iosplugin:
	@echo $(PLUGINDIR) $(PLUGINPATH)
	@cd $(PLUGINPATH); $(XCODEBUILD) -project $(notdir $*).xcodeproj -alltargets -sdk iphonesimulator$$IOS_SDK -configuration $(XCODE_CONFIG) -arch x86_64 LUA_INCLUDES="$(LUA_INCLUDE)" OTHER_CFLAGS="-fembed-bitcode" $(PRETTY)
	@cd $(PLUGINPATH); $(XCODEBUILD) -project $(notdir $*).xcodeproj -alltargets -sdk iphoneos$$IOS_SDK -configuration $(XCODE_CONFIG) LUA_INCLUDES="$(LUA_INCLUDE)" OTHER_CFLAGS="-fembed-bitcode" $(PRETTY)
	@cd $(PLUGINPATH); $(LIPO) build/$(XCODE_CONFIG)-iphoneos/lib$(notdir $*).a build/$(XCODE_CONFIG)-iphonesimulator/lib$(notdir $*).a -create -output lib$(notdir $*).ios.a


%.ios.clean.iosplugin:
	rm -rf $(PLUGINPATH)/build
	rm -f $(PLUGINPATH)/lib*.ios.a

%.ios.install.iosplugin:
	mkdir -p $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS
	cp $(PLUGINPATH)/lib$(notdir $*).ios.a $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS/

%.atv.iosplugin:
	@echo $(PLUGINDIR) $(PLUGINPATH)
	@cd $(PLUGINPATH); $(XCODEBUILD) -alltargets -sdk appletvsimulator$$TVOS_SDK -configuration $(XCODE_CONFIG) -arch x86_64 -project $(notdir $*).xcodeproj LUA_INCLUDES="$(LUA_INCLUDE)" GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode" $(PRETTY)
	@cd $(PLUGINPATH); $(XCODEBUILD) -alltargets -sdk appletvos$$TVOS_SDK -configuration $(XCODE_CONFIG) -project $(notdir $*).xcodeproj LUA_INCLUDES="$(LUA_INCLUDE)" GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode" $(PRETTY)
	@cd $(PLUGINPATH); $(LIPO) build/$(XCODE_CONFIG)-appletvos/lib$(notdir $*).a build/$(XCODE_CONFIG)-appletvsimulator/lib$(notdir $*).a -create -output lib$(notdir $*).atv.a

%.atv.clean.iosplugin:
	rm -rf $(PLUGINPATH)/build
	rm -f $(PLUGINPATH)/lib*.atv.a

%.atv.install.iosplugin:
	mkdir -p $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS
	cp $(PLUGINPATH)/lib$(notdir $*).atv.a $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS/

%.mac.iosplugin:
	@echo $(PLUGINDIR) $(PLUGINPATH)
	@cd $(PLUGINPATH); $(XCODEBUILD) -project $(notdir $*).xcodeproj -alltargets -sdk macosx$$MACOSX_SDK -configuration $(XCODE_CONFIG) LUA_INCLUDES="$(LUA_INCLUDE)" OTHER_CFLAGS="-fembed-bitcode" $(PRETTY)
	@cd $(PLUGINPATH); cp build/$(XCODE_CONFIG)/lib$(notdir $*).a lib$(notdir $*).mac.a


%.mac.clean.iosplugin:
	rm -rf $(PLUGINPATH)/build
	rm -f $(PLUGINPATH)/lib*.mac.a

%.mac.install.iosplugin:
	mkdir -p $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS
	cp $(PLUGINPATH)/lib$(notdir $*).mac.a $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS/

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

	
