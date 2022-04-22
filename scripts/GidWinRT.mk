#MSBuild has issues with parallel uses, stick to single thread
SUBMAKE=$(MAKE) -f scripts/Makefile.gid 

WINRT_SHADERS=Basic Color Texture TextureAlpha TextureColor TextureAlphaColor Particle Particles Particles3 PathFillC PathStrokeC PathStrokeLC
WINRT_SHADERS_PATH=2dsg/gfxbackends/dx11
WINRT_SHADERS_FILE=dx11_shaders.c
BIN2C=$(ROOT)/scripts/bin2c

WINRT_PLAYERDIR=winrt_xaml
WINRT_PLAYERSUBDIR=giderosgame

export MSBUILDDISABLENODEREUSE=1
ifneq ($(DEBUG),)
WINRT_CONF=Debug
else
WINRT_CONF=Release
endif

#Macros
#$(call WINRT_PROJECT basepath name target)
WINRT_PROJECT=$(1)/$(2)/$(2).$(3)/$(2).$(3).vcxproj
#$(call WINRT_MANIFEST basepath name target)
WINRT_MANIFEST=$(1)/$(2)/$(2).$(3)/$(2).$(3).Package.appxmanifest
#$(call WINRT_BUILD_WIN basepath name)
WINRT_BUILD_WIN=\
	$(MSBUILD) $(call WINRT_PROJECT,$(1),$(2),Windows) //p:Configuration=$(WINRT_CONF) //p:Platform=Win32 //v:m;\
	$(MSBUILD) $(call WINRT_PROJECT,$(1),$(2),Windows) //p:Configuration=$(WINRT_CONF) //p:Platform=ARM //v:m;\
	$(MSBUILD) $(call WINRT_PROJECT,$(1),$(2),Windows) //p:Configuration=$(WINRT_CONF) //p:Platform=x64 //v:m;\

WINRT_CLEAN=\
 	$(MSBUILD) $(call WINRT_PROJECT,$(1),$(2),Windows) //t:Clean //p:Configuration=Release //p:Platform=Win32 //v:m;\
 	$(MSBUILD) $(call WINRT_PROJECT,$(1),$(2),Windows) //t:Clean //p:Configuration=Release //p:Platform=ARM //v:m;\
 	$(MSBUILD) $(call WINRT_PROJECT,$(1),$(2),Windows) //t:Clean //p:Configuration=Debug //p:Platform=Win32 //v:m;\
 	$(MSBUILD) $(call WINRT_PROJECT,$(1),$(2),Windows) //t:Clean //p:Configuration=Debug //p:Platform=ARM //v:m;\
 	$(MSBUILD) $(call WINRT_PROJECT,$(1),$(2),Windows) //t:Clean //p:Configuration=Release //p:Platform=x64 //v:m;\
 	$(MSBUILD) $(call WINRT_PROJECT,$(1),$(2),Windows) //t:Clean //p:Configuration=Debug //p:Platform=x64 //v:m;\


WINRT_APPX_GIDVERSION_LIST:=$(subst ., ,$(GIDEROS_VERSION)) 0 0 0 0
WINRT_APPX_GIDVERSION:=$(word 1,$(WINRT_APPX_GIDVERSION_LIST)).$(word 2,$(WINRT_APPX_GIDVERSION_LIST)).$(word 3,$(WINRT_APPX_GIDVERSION_LIST)).$(word 4,$(WINRT_APPX_GIDVERSION_LIST))

$(WINRT_SHADERS_PATH)/$(WINRT_SHADERS_FILE): $(BIN2C) $(addsuffix .hlsl,$(addprefix $(WINRT_SHADERS_PATH)/,$(WINRT_SHADERS)))
	rm -f $(WINRT_SHADERS_PATH)/$(WINRT_SHADERS_FILE)
	for s in $(WINRT_SHADERS); do $(FXC) //T vs_4_0_level_9_3 //E VShader //Fo $(WINRT_SHADERS_PATH)/v$$s.cso $(WINRT_SHADERS_PATH)/$$s.hlsl; done
	for s in $(WINRT_SHADERS); do $(FXC) //T ps_4_0_level_9_3 //E PShader //Fo $(WINRT_SHADERS_PATH)/p$$s.cso $(WINRT_SHADERS_PATH)/$$s.hlsl; done
	P=$(PWD); cd $(WINRT_SHADERS_PATH); for s in $(WINRT_SHADERS); do $$P/$(BIN2C) p$$s.cso >> $(WINRT_SHADERS_FILE); $$P/$(BIN2C) v$$s.cso >> $(WINRT_SHADERS_FILE); done
	rm -f $(WINRT_SHADERS_PATH)/*.cso	

winrt.shaders: $(WINRT_SHADERS_PATH)/$(WINRT_SHADERS_FILE)

winrt.lua:
	$(call WINRT_BUILD_WIN,$(LUA_ENGINE)/luawinrt,luawinrt)

winrt.lua.clean:
	$(call WINRT_CLEAN,$(LUA_ENGINE)/luawinrt,luawinrt)

winrt.gvfs:
	$(call WINRT_BUILD_WIN,libgvfs/libgvfswinrt,libgvfswinrt)

winrt.gvfs.clean:
	$(call WINRT_CLEAN,libgvfs/libgvfswinrt,libgvfswinrt)

winrt.libs: winrt.lua winrt.gvfs

winrt.libs.clean: winrt.lua.clean winrt.gvfs.clean

%.plugin.winrt:
	$(call WINRT_BUILD_WIN,plugins/$*/source/winrt,$*)
	mkdir -p $(RELEASE)/All\ Plugins/$*/bin/WinRT/Win32
	mkdir -p $(RELEASE)/All\ Plugins/$*/bin/WinRT/ARM
	mkdir -p $(RELEASE)/All\ Plugins/$*/bin/WinRT/x64
	cp plugins/$*/source/winrt/$*/$*.Windows/ARM/$(WINRT_CONF)/$*.Windows/*.Windows.lib $(RELEASE)/All\ Plugins/$*/bin/WinRT/ARM/
	cp plugins/$*/source/winrt/$*/$*.Windows/$(WINRT_CONF)/$*.Windows/*.Windows.lib $(RELEASE)/All\ Plugins/$*/bin/WinRT/Win32/
	cp plugins/$*/source/winrt/$*/$*.Windows/x64/$(WINRT_CONF)/$*.Windows/*.Windows.lib $(RELEASE)/All\ Plugins/$*/bin/WinRT/x64

luasocket.plugin.winrt:
	$(call WINRT_BUILD_WIN,plugins/luasocket/source/winrt/luasocket,luasocket)
	mkdir -p $(RELEASE)/All\ Plugins/luasocket/bin/WinRT/Win32
	mkdir -p $(RELEASE)/All\ Plugins/luasocket/bin/WinRT/ARM
	mkdir -p $(RELEASE)/All\ Plugins/luasocket/bin/WinRT/x64
	cp Release/All\ Plugins/luasocket/bin/WinRT/$(WINRT_CONF)/ARM/*.Windows.lib $(RELEASE)/All\ Plugins/luasocket/bin/WinRT/ARM/
	cp Release/All\ Plugins/luasocket/bin/WinRT/$(WINRT_CONF)/x64/*.Windows.lib $(RELEASE)/All\ Plugins/luasocket/bin/WinRT/x64/
	cp Release/All\ Plugins/luasocket/bin/WinRT/$(WINRT_CONF)/Win32/*.Windows.lib $(RELEASE)/All\ Plugins/luasocket/bin/WinRT/Win32/

%.plugin.winrt.clean:
	$(call WINRT_CLEAN,plugins/$*/source/winrt,$*)

luasocket.plugin.winrt.clean:
	$(call WINRT_CLEAN,plugins/luasocket/source/winrt/luasocket,luasocket)

winrt.plugins: 
	$(SUBMAKE) $(addsuffix .plugin.winrt,$(PLUGINS_WINRT))
	
winrt.plugins.clean: 
	$(SUBMAKE) $(addsuffix .plugin.winrt.clean,$(PLUGINS_WINRT))
	
winrt.core: winrt.libs winrt.shaders
	$(call WINRT_BUILD_WIN,winrt,gideros)
	#X86 Release version for Windows
	mkdir -p winrt/$(WINRT_CONF)/gideros.Windows
	cp winrt/gideros/gideros.Windows/$(WINRT_CONF)/gideros.Windows/gideros.Windows.lib winrt/$(WINRT_CONF)/gideros.Windows
	mkdir -p winrt/$(WINRT_CONF)/luawinrt.Windows
	cp $(LUA_ENGINE)/luawinrt/luawinrt/luawinrt.Windows/$(WINRT_CONF)/luawinrt.Windows/luawinrt.Windows.lib winrt/$(WINRT_CONF)/luawinrt.Windows
	mkdir -p winrt/$(WINRT_CONF)/libgvfswinrt.Windows
	cp libgvfs/libgvfswinrt/libgvfswinrt/libgvfswinrt.Windows/$(WINRT_CONF)/libgvfswinrt.Windows/libgvfswinrt.Windows.lib winrt/$(WINRT_CONF)/libgvfswinrt.Windows
	#ARM release version for WinPhone
	mkdir -p winrt/ARM/$(WINRT_CONF)/gideros.Windows
	cp winrt/gideros/gideros.Windows/ARM/$(WINRT_CONF)/gideros.Windows/gideros.Windows.lib winrt/ARM/$(WINRT_CONF)/gideros.Windows
	mkdir -p winrt/ARM/$(WINRT_CONF)/luawinrt.Windows
	cp $(LUA_ENGINE)/luawinrt/luawinrt/luawinrt.Windows/ARM/$(WINRT_CONF)/luawinrt.Windows/luawinrt.Windows.lib winrt/ARM/$(WINRT_CONF)/luawinrt.Windows
	mkdir -p winrt/ARM/$(WINRT_CONF)/libgvfswinrt.Windows
	cp libgvfs/libgvfswinrt/libgvfswinrt/libgvfswinrt.Windows/ARM/$(WINRT_CONF)/libgvfswinrt.Windows/libgvfswinrt.Windows.lib winrt/ARM/$(WINRT_CONF)/libgvfswinrt.Windows
	#x64 release version for Windows
	mkdir -p winrt/x64/$(WINRT_CONF)/gideros.Windows
	cp winrt/gideros/gideros.Windows/x64/$(WINRT_CONF)/gideros.Windows/gideros.Windows.lib winrt/x64/$(WINRT_CONF)/gideros.Windows
	mkdir -p winrt/x64/$(WINRT_CONF)/luawinrt.Windows
	cp $(LUA_ENGINE)/luawinrt/luawinrt/luawinrt.Windows/x64/$(WINRT_CONF)/luawinrt.Windows/luawinrt.Windows.lib winrt/x64/$(WINRT_CONF)/luawinrt.Windows
	mkdir -p winrt/x64/$(WINRT_CONF)/libgvfswinrt.Windows
	cp libgvfs/libgvfswinrt/libgvfswinrt/libgvfswinrt.Windows/x64/$(WINRT_CONF)/libgvfswinrt.Windows/libgvfswinrt.Windows.lib winrt/x64/$(WINRT_CONF)/libgvfswinrt.Windows

winrt.core.clean: winrt.libs.clean
	$(call WINRT_CLEAN,winrt,gideros) 
	 
winrt.template: winrt.core winrt.plugins 
	rm -rf $(RELEASE)/Templates/VisualStudio
	mkdir -p $(RELEASE)/Templates
	#Non XAML
	#cp -r ui/Templates/VisualStudio $(RELEASE)/Templates
	#cp $(WINRT_PLAYERDIR)/*.cpp "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	#cp winrt/gideros/gideros.Shared/giderosapi.h "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	#XAML
	mkdir -p "$(RELEASE)/Templates/VisualStudio/WinRT Template/Win32"
	mkdir -p "$(RELEASE)/Templates/VisualStudio/WinRT Template/ARM"
	mkdir -p "$(RELEASE)/Templates/VisualStudio/WinRT Template/x64"
	cd $(WINRT_PLAYERDIR); git archive $(CURRENT_GIT_BRANCH) | tar -x -C "../$(RELEASE)/Templates/VisualStudio/WinRT Template"
	rm -rf "$(RELEASE)/Templates/VisualStudio/WinRT Template/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Shared/Plugins/"*
	cp $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Shared/Plugins/plugins.cpp "$(RELEASE)/Templates/VisualStudio/WinRT Template/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Shared/Plugins/" 
	sed -e ':1;s/PLUGINS-START/ /;t2;:3;n;b1;:2;g;n;s/PLUGINS-END/ /;t3;b2' $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Shared/$(WINRT_PLAYERSUBDIR).Shared.vcxitems | sed -e 's/$$(GidLibsPath)/\.\.\\\.\.\\$$(Platform)/' >"$(RELEASE)/Templates/VisualStudio/WinRT Template/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Shared/$(WINRT_PLAYERSUBDIR).Shared.vcxitems"
	sed -e ':1;s/PLUGINS-START/ /;t2;:3;n;b1;:2;g;n;s/PLUGINS-END/ /;t3;b2' $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/$(WINRT_PLAYERSUBDIR).Windows.vcxproj >"$(RELEASE)/Templates/VisualStudio/WinRT Template/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/$(WINRT_PLAYERSUBDIR).Windows.vcxproj"
	sed -e 's/ Version="[^"]*"/ Version="1.0.0.0"/' $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest >"$(RELEASE)/Templates/VisualStudio/WinRT Template/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest"
	cp winrt/gideros/gideros.Shared/giderosapi.h "$(RELEASE)/Templates/VisualStudio/WinRT Template/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Shared/"
	cp $(SDK)/include/*.h "$(RELEASE)/Templates/VisualStudio/WinRT Template/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Shared/"
	
	#X86 Release version for Windows
	cp winrt/gideros/gideros.Windows/$(WINRT_CONF)/gideros.Windows/gideros.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template/Win32"
	cp $(LUA_ENGINE)/luawinrt/luawinrt/luawinrt.Windows/$(WINRT_CONF)/luawinrt.Windows/luawinrt.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template/Win32"
	cp libgvfs/libgvfswinrt/libgvfswinrt/libgvfswinrt.Windows/$(WINRT_CONF)/libgvfswinrt.Windows/libgvfswinrt.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template/Win32"
	#ARM release version for WinPhone
	cp winrt/gideros/gideros.Windows/ARM/$(WINRT_CONF)/gideros.Windows/gideros.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template/ARM"
	cp $(LUA_ENGINE)/luawinrt/luawinrt/luawinrt.Windows/ARM/$(WINRT_CONF)/luawinrt.Windows/luawinrt.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template/ARM"
	cp libgvfs/libgvfswinrt/libgvfswinrt/libgvfswinrt.Windows/ARM/$(WINRT_CONF)/libgvfswinrt.Windows/libgvfswinrt.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template/ARM"
	#x64 Release version for Windows
	cp winrt/gideros/gideros.Windows/x64/$(WINRT_CONF)/gideros.Windows/gideros.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template/x64"
	cp $(LUA_ENGINE)/luawinrt/luawinrt/luawinrt.Windows/x64/$(WINRT_CONF)/luawinrt.Windows/luawinrt.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template/x64"
	cp libgvfs/libgvfswinrt/libgvfswinrt/libgvfswinrt.Windows/x64/$(WINRT_CONF)/libgvfswinrt.Windows/libgvfswinrt.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template/x64"
	#Plugins libs
	#cp $(RELEASE)/All\ Plugins/luasocket/bin/WinRT/*.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"

winrt.player: winrt.template
	@echo "VERSION" $(GIDEROS_VERSION)
	mkdir -p $(RELEASE)/Players
	cd $(RELEASE); Tools/gdrexport.exe -platform winrt -organization "Gideros Mobile" -package "com.giderosmobile.player" Examples/Misc/GiderosPlayer/GiderosPlayer.gproj Players
	rm -f $(RELEASE)/Players/GiderosPlayer/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Assets/LargeTile.scale-400.png
	cp $(RELEASE)/Players/GiderosPlayer/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest $(RELEASE)/Players/GiderosPlayer/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest.bak
	sed -e 's/ Version="[^"]*"/ Version="$(WINRT_APPX_GIDVERSION)"/'	$(RELEASE)/Players/GiderosPlayer/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest.bak >$(RELEASE)/Players/GiderosPlayer/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest
	$(MSBUILD) $(RELEASE)/Players/GiderosPlayer/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/$(WINRT_PLAYERSUBDIR).Windows.vcxproj //t:Publish //p:Configuration=$(WINRT_CONF) //p:AppxBundlePlatforms="x86|ARM|x64" //p:AppxBundle=Always //v:m
	mkdir -p $(RELEASE)/Players
	rm -rf $(RELEASE)/Players/WinRT
	mkdir -p $(RELEASE)/Players/WinRT
	cp -r $(RELEASE)/Players/GiderosPlayer/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/AppPackages/$(WINRT_PLAYERSUBDIR).Windows/* $(RELEASE)/Players/WinRT
	rm -rf cp -r $(RELEASE)/Players/GiderosPlayer
	
winrt.player_old: winrt.template
	cp winrt/gideros/gideros.Shared/giderosapi.h $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Shared/
	rm -rf /c/winrt_player
	cp $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest.bak
	sed -e 's/ Version="[^"]*"/ Version="$(WINRT_APPX_GIDVERSION)"/'	$(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest.bak >$(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest
	$(MSBUILD) $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/$(WINRT_PLAYERSUBDIR).Windows.vcxproj //t:Publish //p:Configuration=$(WINRT_CONF) //p:AppxBundlePlatforms="x86|ARM|x64" //p:AppxBundle=Always //v:m
	cp $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest.bak $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest
	rm $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest.bak
	mkdir -p $(RELEASE)/Players
	rm -rf $(RELEASE)/Players/WinRT
	#mv /c/winrt_player $(RELEASE)/Players/WinRT
	mkdir -p $(RELEASE)/Players/WinRT
	cp -r $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/AppPackages/$(WINRT_PLAYERSUBDIR).Windows/* $(RELEASE)/Players/WinRT

winrt.player.clean:
	$(call WINRT_CLEAN,$(WINRT_PLAYERDIR),$(WINRT_PLAYERSUBDIR)) 
	
winrt.install: winrt.player

winrt.clean: winrt.core.clean winrt.player.clean

winrt.zip: winrt.player
	rm -f $@
	P=$(PWD); cd $(RELEASE); zip -r $$P/$@ "Templates/VisualStudio/WinRT Template" Players/WinRT "All Plugins"/*/bin/WinRT
			