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
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk iphonesimulator$$IOS_SDK -configuration Release -project $*.xcodeproj
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk iphoneos$$IOS_SDK -configuration Release -project $*.xcodeproj
	@cd $(IOSLIBPATH); $(LIPO) build/Release-iphoneos/lib$*.a build/Release-iphonesimulator/lib$*.a -create -output lib$*.ios.a

%.atv.libs: 
	#BUILDING $*
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk appletvsimulator$$TVOS_SDK -configuration Release -project $*.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode"
	@cd $(IOSLIBPATH); $(XCODEBUILD) -alltargets -sdk appletvos$$TVOS_SDK -configuration Release -project $*.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode"
	@cd $(IOSLIBPATH); $(LIPO) build/Release-appletvos/lib$*.a build/Release-appletvsimulator/lib$*.a -create -output lib$*.atv.a

ios.libs: gvfs.ios.libs lua.ios.libs iosplayer.ios.libs
atv.libs: gvfs.atv.libs lua.atv.libs iosplayer.atv.libs


ios.app: player.ios.app

ios.libs.install: ios.libs
	mkdir -p $(IOS_TEMPLATE)
	cp -R $(ROOT)/ui/Templates/Xcode4/iOS\ Template/iOS\ Template/* $(IOS_TEMPLATE)
	cp $(ROOT)/lua/liblua.ios.a $(IOS_TEMPLATE)/liblua.a
	cp $(ROOT)/libgvfs/libgvfs.ios.a $(IOS_TEMPLATE)/libgvfs.a
	cp $(ROOT)/ios/iosplayer/libiosplayer.ios.a $(IOS_TEMPLATE)/libgideros.a
	cp $(ROOT)/ios/iosplayer/iosplayer/giderosapi.h $(IOS_TEMPLATE)

atv.libs.install: atv.libs
	mkdir -p $(ATV_TEMPLATE)
	cp -R $(ROOT)/ui/Templates/Xcode4/iOS\ Template/AppleTV/* $(ATV_TEMPLATE)
	cp $(ROOT)/lua/liblua.atv.a $(ATV_TEMPLATE)/liblua.a
	cp $(ROOT)/libgvfs/libgvfs.atv.a $(ATV_TEMPLATE)/libgvfs.a
	cp $(ROOT)/ios/iosplayer/libiosplayer.atv.a $(ATV_TEMPLATE)/libgideros.a
	cp $(ROOT)/ios/iosplayer/iosplayer/giderosapi.h $(ATV_TEMPLATE)

%.ios.plugin:

%.ios.plugin.clean:

%.ios.plugin.install:

ios.install: ios.libs.install atv.libs.install ios.plugins.install #ios.app

ios.clean: ios.plugins.clean
		
ios.plugins: $(addsuffix .ios.plugin,$(PLUGINS_IOS))

ios.plugins.clean: $(addsuffix .ios.plugin.clean,$(PLUGINS_IOS))

ios.plugins.install: ios.plugins $(addsuffix .ios.plugin.install,$(PLUGINS_WIN32))


		