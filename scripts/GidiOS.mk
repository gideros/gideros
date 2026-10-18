XCODEBUILD=xcodebuild
LIPO=lipo
# Only pipe through xcpretty when it is installed; the pipe's status is checked with pipefail in the rules below.
PRETTY?=$(if $(shell command -v xcpretty 2>/dev/null),| xcpretty -c,)

XCODE_CONFIG?=Release

# Deployment floor (IOS_MIN, TVOS_MIN, MAC_MIN) is defined once in apple-targets.mk, together with the other places that carry it.
# The archs shipped in each slice: device slices are arm64 only, simulator and macOS slices are fat. ONLY_ACTIVE_ARCH=NO
# because the .pbxproj settings vary. The lib and plugin .pbxproj files still carry older deployment targets, hence the
# command line overrides.
include $(dir $(abspath $(lastword $(MAKEFILE_LIST))))apple-targets.mk
DEV_ARCHS?=arm64
SIM_ARCHS?=arm64 x86_64
MAC_ARCHS?=arm64 x86_64
APPLE_DEPLOY=IPHONEOS_DEPLOYMENT_TARGET=$(IOS_MIN) TVOS_DEPLOYMENT_TARGET=$(TVOS_MIN) MACOSX_DEPLOYMENT_TARGET=$(MAC_MIN) ONLY_ACTIVE_ARCH=NO

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
openal.ios.libs.clean: IOSLIBPATH=$(ROOT)/ios/iosplayer

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

# The .a inside each XCFramework slice must carry the install name: gdrexport patches keys into files named libgideros.a
iosplayer.%.libs: IOSLIBNAME=gideros
# iosplayer needs the generated luabinding/luabindings.luac.c (versioning) and Sdk/include/AL (sdk.headers)
iosplayer.ios.libs iosplayer.atv.libs iosplayer.mac.libs: versioning sdk.headers

##RULES
# Do not run these with make -j: iosplayer and openal build in the same ios/iosplayer/build directory.
%.ios.libs: 
	#BUILDING $*
	@cd $(IOSLIBPATH); set -o pipefail; $(XCODEBUILD) -alltargets -sdk iphonesimulator$$IOS_SDK -configuration $(XCODE_CONFIG) ARCHS="$(SIM_ARCHS)" $(APPLE_DEPLOY) -project $*.xcodeproj LUA_INCLUDES="$(IOSLUA_INCLUDES)" $(PRETTY)
	@cd $(IOSLIBPATH); set -o pipefail; $(XCODEBUILD) -alltargets -sdk iphoneos$$IOS_SDK -configuration $(XCODE_CONFIG) ARCHS="$(DEV_ARCHS)" $(APPLE_DEPLOY) -project $*.xcodeproj LUA_INCLUDES="$(IOSLUA_INCLUDES)" $(PRETTY)
	@cd $(IOSLIBPATH); rm -rf build/xcf-ios lib$*.ios.xcframework && mkdir -p build/xcf-ios/dev build/xcf-ios/sim && cp build/$(XCODE_CONFIG)-iphoneos/lib$*.a build/xcf-ios/dev/lib$(or $(IOSLIBNAME),$*).a && cp build/$(XCODE_CONFIG)-iphonesimulator/lib$*.a build/xcf-ios/sim/lib$(or $(IOSLIBNAME),$*).a && $(XCODEBUILD) -create-xcframework -library build/xcf-ios/dev/lib$(or $(IOSLIBNAME),$*).a -library build/xcf-ios/sim/lib$(or $(IOSLIBNAME),$*).a -output lib$*.ios.xcframework

%.ios.libs.clean:
	cd $(IOSLIBPATH) && rm -rf build lib*.ios.xcframework

%.atv.libs: 
	#BUILDING $*
	@cd $(IOSLIBPATH); set -o pipefail; $(XCODEBUILD) -alltargets -sdk appletvsimulator$$TVOS_SDK -configuration $(XCODE_CONFIG) ARCHS="$(SIM_ARCHS)" $(APPLE_DEPLOY) -project $*.xcodeproj LUA_INCLUDES="$(IOSLUA_INCLUDES)" GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' $(PRETTY)
	@cd $(IOSLIBPATH); set -o pipefail; $(XCODEBUILD) -alltargets -sdk appletvos$$TVOS_SDK -configuration $(XCODE_CONFIG) ARCHS="$(DEV_ARCHS)" $(APPLE_DEPLOY) -project $*.xcodeproj LUA_INCLUDES="$(IOSLUA_INCLUDES)" GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' $(PRETTY)
	@cd $(IOSLIBPATH); rm -rf build/xcf-atv lib$*.atv.xcframework && mkdir -p build/xcf-atv/dev build/xcf-atv/sim && cp build/$(XCODE_CONFIG)-appletvos/lib$*.a build/xcf-atv/dev/lib$(or $(IOSLIBNAME),$*).a && cp build/$(XCODE_CONFIG)-appletvsimulator/lib$*.a build/xcf-atv/sim/lib$(or $(IOSLIBNAME),$*).a && $(XCODEBUILD) -create-xcframework -library build/xcf-atv/dev/lib$(or $(IOSLIBNAME),$*).a -library build/xcf-atv/sim/lib$(or $(IOSLIBNAME),$*).a -output lib$*.atv.xcframework

%.atv.libs.clean:
	cd $(IOSLIBPATH) && rm -rf build lib*.atv.xcframework

%.mac.libs: 
	#BUILDING $*
	@cd $(IOSLIBPATH); set -o pipefail; $(XCODEBUILD) -alltargets -sdk macosx$$MACOSX_SDK -configuration $(XCODE_CONFIG) ARCHS="$(MAC_ARCHS)" $(APPLE_DEPLOY) -project $*.xcodeproj LUA_INCLUDES="$(IOSLUA_INCLUDES)" $(PRETTY)
	@cd $(IOSLIBPATH); cp build/$(XCODE_CONFIG)/lib$*.a lib$*.mac.a

%.mac.libs.clean:
	cd $(IOSLIBPATH) && rm -rf build lib*.mac.a


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
	rm -rf $(IOS_TEMPLATE)/liblua.xcframework
	cp -R $(ROOT)/$(LUA_ENGINE)/liblua.ios.xcframework $(IOS_TEMPLATE)/liblua.xcframework
	rm -rf $(IOS_TEMPLATE)/libgvfs.xcframework
	cp -R $(ROOT)/libgvfs/libgvfs.ios.xcframework $(IOS_TEMPLATE)/libgvfs.xcframework
	rm -rf $(IOS_TEMPLATE)/libgideros.xcframework
	cp -R $(ROOT)/ios/iosplayer/libiosplayer.ios.xcframework $(IOS_TEMPLATE)/libgideros.xcframework
	rm -rf $(IOS_TEMPLATE)/libopenal.xcframework
	cp -R $(ROOT)/ios/iosplayer/libopenal.ios.xcframework $(IOS_TEMPLATE)/libopenal.xcframework
	cp $(ROOT)/ios/iosplayer/build/Release-iphoneos/default.metallib $(IOS_TEMPLATE)
	cp $(ROOT)/ios/iosplayer/build/Release-iphonesimulator/default.metallib $(IOS_TEMPLATE)/default-sim.metallib
	cp $(ROOT)/ios/iosplayer/iosplayer/giderosapi.h $(IOS_TEMPLATE)


atv.libs.install: atv.libs
	mkdir -p $(ATV_TEMPLATE)
	rm -rf $(ATV_TEMPLATE)/liblua.xcframework
	cp -R $(ROOT)/$(LUA_ENGINE)/liblua.atv.xcframework $(ATV_TEMPLATE)/liblua.xcframework
	rm -rf $(ATV_TEMPLATE)/libgvfs.xcframework
	cp -R $(ROOT)/libgvfs/libgvfs.atv.xcframework $(ATV_TEMPLATE)/libgvfs.xcframework
	rm -rf $(ATV_TEMPLATE)/libgideros.xcframework
	cp -R $(ROOT)/ios/iosplayer/libiosplayer.atv.xcframework $(ATV_TEMPLATE)/libgideros.xcframework
	rm -rf $(ATV_TEMPLATE)/libopenal.xcframework
	cp -R $(ROOT)/ios/iosplayer/libopenal.atv.xcframework $(ATV_TEMPLATE)/libopenal.xcframework
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
	cp $(ROOT)/ios/iosplayer/build/Release/default.metallib $(MAC_TEMPLATE)/default-sim.metallib
	cp $(ROOT)/ios/iosplayer/iosplayer/giderosapi.h $(MAC_TEMPLATE)

# Build + install all Apple libs, then check slices (arch/platform/minos/names) and that they link together
apple.libs.check: ios.libs.install atv.libs.install mac.libs.install
	$(ROOT)/scripts/apple-libs-verify.sh "$(RELEASE)/Templates/Xcode4/iOS Template"
	$(ROOT)/scripts/apple-libs-linktest.sh "$(RELEASE)/Templates/Xcode4/iOS Template"

# Build + install all Apple plugins, then check their slices (arch/platform/minos/names)
apple.plugins.check: ios.plugins.install
	$(ROOT)/scripts/apple-plugins-verify.sh "$(RELEASE)/All Plugins"

luasocket.%: PLUGINDIR=LuaSocket

# The plugins compile against Sdk/include (lua.h for the selected LUA_ENGINE), which sdk.headers writes, hence the prerequisite
%.iosplugin: PLUGINDIR?=$*
%.iosplugin: PLUGINPATH=$(ROOT)/plugins/$(PLUGINDIR)/source

# Same slices as the core libs: iOS/tvOS = device arm64 + fat simulator in an XCFramework, macOS = fat .a.
# No LUA_INCLUDES here: the plugins read lua.h from Sdk/include (written by sdk.headers from the selected LUA_ENGINE).

%.ios.iosplugin: sdk.headers
	@echo $(PLUGINDIR) $(PLUGINPATH)
	@cd $(PLUGINPATH); set -o pipefail; $(XCODEBUILD) -project $(notdir $*).xcodeproj -alltargets -sdk iphonesimulator$$IOS_SDK -configuration $(XCODE_CONFIG) ARCHS="$(SIM_ARCHS)" $(APPLE_DEPLOY) $(PRETTY)
	@cd $(PLUGINPATH); set -o pipefail; $(XCODEBUILD) -project $(notdir $*).xcodeproj -alltargets -sdk iphoneos$$IOS_SDK -configuration $(XCODE_CONFIG) ARCHS="$(DEV_ARCHS)" $(APPLE_DEPLOY) $(PRETTY)
	@cd $(PLUGINPATH); rm -rf lib$(notdir $*).ios.xcframework && $(XCODEBUILD) -create-xcframework -library build/$(XCODE_CONFIG)-iphoneos/lib$(notdir $*).a -library build/$(XCODE_CONFIG)-iphonesimulator/lib$(notdir $*).a -output lib$(notdir $*).ios.xcframework

%.ios.clean.iosplugin:
	rm -rf $(PLUGINPATH)/build
	rm -rf $(PLUGINPATH)/lib*.ios.xcframework
	rm -f $(PLUGINPATH)/lib*.ios.a

%.ios.install.iosplugin:
	mkdir -p $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS
	rm -rf $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS/lib$(notdir $*).ios.xcframework
	cp -R $(PLUGINPATH)/lib$(notdir $*).ios.xcframework $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS/

%.atv.iosplugin: sdk.headers
	@echo $(PLUGINDIR) $(PLUGINPATH)
	@cd $(PLUGINPATH); set -o pipefail; $(XCODEBUILD) -project $(notdir $*).xcodeproj -alltargets -sdk appletvsimulator$$TVOS_SDK -configuration $(XCODE_CONFIG) ARCHS="$(SIM_ARCHS)" $(APPLE_DEPLOY) GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' $(PRETTY)
	@cd $(PLUGINPATH); set -o pipefail; $(XCODEBUILD) -project $(notdir $*).xcodeproj -alltargets -sdk appletvos$$TVOS_SDK -configuration $(XCODE_CONFIG) ARCHS="$(DEV_ARCHS)" $(APPLE_DEPLOY) GCC_PREPROCESSOR_DEFINITIONS='$${inherited} TARGET_OS_TV=1' $(PRETTY)
	@cd $(PLUGINPATH); rm -rf lib$(notdir $*).atv.xcframework && $(XCODEBUILD) -create-xcframework -library build/$(XCODE_CONFIG)-appletvos/lib$(notdir $*).a -library build/$(XCODE_CONFIG)-appletvsimulator/lib$(notdir $*).a -output lib$(notdir $*).atv.xcframework

%.atv.clean.iosplugin:
	rm -rf $(PLUGINPATH)/build
	rm -rf $(PLUGINPATH)/lib*.atv.xcframework
	rm -f $(PLUGINPATH)/lib*.atv.a

%.atv.install.iosplugin:
	mkdir -p $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS
	rm -rf $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS/lib$(notdir $*).atv.xcframework
	cp -R $(PLUGINPATH)/lib$(notdir $*).atv.xcframework $(RELEASE)/All\ Plugins/$(notdir $(PLUGINDIR))/bin/iOS/

%.mac.iosplugin: sdk.headers
	@echo $(PLUGINDIR) $(PLUGINPATH)
	@cd $(PLUGINPATH); set -o pipefail; $(XCODEBUILD) -project $(notdir $*).xcodeproj -alltargets -sdk macosx$$MACOSX_SDK -configuration $(XCODE_CONFIG) ARCHS="$(MAC_ARCHS)" $(APPLE_DEPLOY) $(PRETTY)
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
PLUGIN_PREREQUISITE+= %.ios.iosplugin %.atv.iosplugin %.mac.iosplugin

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

	
