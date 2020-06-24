XCODEBUILD=xcodebuild
LIPO=lipo

MAC_TEMPLATE=$(RELEASE)/Templates/Xcode4/Mac\ Template

lua.mac.libs: MACLIBPATH=$(ROOT)/lua
gvfs.mac.libs: MACLIBPATH=$(ROOT)/libgvfs
iosplayer.mac.libs: MACLIBPATH=$(ROOT)/ios/iosplayer
lua.mac.libs.clean: MACLIBPATH=$(ROOT)/lua
gvfs.mac.libs.clean: MACLIBPATH=$(ROOT)/libgvfs
iosplayer.mac.libs.clean: MACLIBPATH=$(ROOT)/ios/iosplayer

##RULES
%.mac.libs: 
	#BUILDING $*
	@cd $(MACLIBPATH); $(XCODEBUILD) -alltargets -sdk macosx$$MACOSX_SDK -configuration Release -project $*.xcodeproj OTHER_CFLAGS="-fembed-bitcode"
	@cd $(MACLIBPATH); cp build/Release/lib$*.a lib$*.mac.a

%.mac.libs.clean:
	cd $(MACLIBPATH); rm -rf build


mac.libs: versioning  gvfs.mac.libs lua.mac.libs iosplayer.mac.libs
mac.libs.clean : gvfs.mac.libs.clean lua.mac.libs.clean iosplayer.mac.libs.clean


mac.app: player.mac.app

mac.libs.install: mac.libs
	mkdir -p $(MAC_TEMPLATE)
	cp -R $(ROOT)/ui/Templates/Xcode4/Mac\ Template/* $(MAC_TEMPLATE)/..
	cp $(ROOT)/lua/liblua.mac.a $(MAC_TEMPLATE)/liblua.a
	cp $(ROOT)/libgvfs/libgvfs.mac.a $(MAC_TEMPLATE)/libgvfs.a
	cp $(ROOT)/ios/iosplayer/libiosplayer.mac.a $(MAC_TEMPLATE)/libgideros.a
	cp $(ROOT)/ios/iosplayer/iosplayer/giderosapi.h $(MAC_TEMPLATE)


luasocket.%: PLUGINDIR=LuaSocket
camera.%: PLUGINDIR=camera
ogg.%: PLUGINDIR=ogg

%.iosplugin: PLUGINDIR?=$*
%.iosplugin: PLUGINPATH=$(ROOT)/plugins/$(PLUGINDIR)/source

%.mac.iosplugin:
	@echo $(PLUGINDIR) $(PLUGINPATH)
	@cd $(PLUGINPATH); $(XCODEBUILD) -project $*.xcodeproj -alltargets -sdk macosx$$MACOSX_SDK -configuration Release OTHER_CFLAGS="-fembed-bitcode"
	@cd $(PLUGINPATH); cp build/Release/lib$*.a lib$*.mac.a


%.mac.clean.iosplugin:
	rm -rf $(PLUGINPATH)/build
	rm -f $(PLUGINPATH)/lib*.mac.a

%.mac.install.iosplugin:
	mkdir -p $(RELEASE)/All\ Plugins/$(PLUGINDIR)/bin/mac
	cp $(PLUGINPATH)/lib$*.mac.a $(RELEASE)/All\ Plugins/$(PLUGINDIR)/bin/mac/

mac.install: mac.libs.install mac.plugins.install mac.app

mac.clean: mac.plugins.clean mac.libs.clean
		
mac.plugins: $(addsuffix .mac.iosplugin,$(PLUGINS_MAC)) 

mac.plugins.clean: $(addsuffix .mac.clean.iosplugin,$(PLUGINS_MAC))

PLUGINS_MAC_DEFFILES=$(ROOT)/Sdk/include/*.h
PLUGINS_MAC_PLAYER=$(addprefix plugins/, \
		LuaSocket/source/luasocket_stub.cpp \
		$(addprefix lsqlite3/source/,lsqlite3.c lsqlite3_stub.cpp) \
		$(addprefix lfs/source/,lfs.h lfs.cpp lfs_stub.cpp) \
		$(addprefix BitOp/source/,bit.c bit_stub.cpp) \
		$(addprefix JSON/source/,fpconv.c fpconv.h strbuf.c strbuf.h lua_cjson.c lua_cjson_stub.cpp) \
	)


MAC_PLAYER_DIR=$(ROOT)/osx/MacPlayer
		
mac.plugins.install: mac.plugins $(addsuffix .mac.install.iosplugin,$(PLUGINS_MAC))
	mkdir -p $(MAC_TEMPLATE)/Plugins
	cp $(PLUGINS_MAC_DEFFILES) $(MAC_TEMPLATE)/Plugins

player.mac.app: 
	rm -rf $(MAC_PLAYER_DIR)/MacPlayer/Plugins
	cp -R $(MAC_TEMPLATE)/Plugins $(MAC_PLAYER_DIR)/MacPlayer/
	cp $(PLUGINS_MAC_PLAYER) $(MAC_TEMPLATE)/Plugins
	cp $(RELEASE)/All\ Plugins/LuaSocket/bin/MAC/libluasocket.mac.a $(MAC_PLAYER_DIR)/MacPlayer/Plugins/libluasocket.a
	cp $(MAC_TEMPLATE)/*.a $(MAC_PLAYER_DIR)/MacPlayer/
	cp $(MAC_TEMPLATE)/giderosapi.h $(MAC_PLAYER_DIR)/MacPlayer/
	mkdir -p $(RELEASE)/Players
	#cd $(MAC_PLAYER_DIR); $(XCODEBUILD) -sdk macosx$$MAC_SDK -configuration Release IPHONEOS_DEPLOYMENT_TARGET=6.0 -project GiderosMACPlayer.xcodeproj -scheme GiderosMACPlayer -archivePath GiderosMACPlayer.xcarchive archive
	#cd $(MAC_PLAYER_DIR); $(XCODEBUILD) -exportArchive -exportPath ../../$(RELEASE)/Players/GiderosMACPlayer.ipa -exportFormat ipa -archivePath GiderosMACPlayer.xcarchive
	#R=$(PWD);cd $(MAC_PLAYER_DIR)/..; zip -r $$R/$(RELEASE)/Players/GiderosMACPlayer.zip GiderosMACPlayer 
	
