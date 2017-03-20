WINRT_SHADERS=Basic Color Texture TextureColor Particle Particles PathFillC PathStrokeC PathStrokeLC
WINRT_SHADERS_PATH=2dsg/gfxbackends/dx11
WINRT_SHADERS_FILE=dx11_shaders.c
BIN2C=$(ROOT)/scripts/bin2c

WINRT_PLAYERDIR=winrt_xaml
WINRT_PLAYERSUBDIR=giderosgame

#Macros
#$(call WINRT_PROJECT basepath name target)
WINRT_PROJECT=$(1)/$(2)/$(2).$(3)/$(2).$(3).vcxproj
#$(call WINRT_MANIFEST basepath name target)
WINRT_MANIFEST=$(1)/$(2)/$(2).$(3)/$(2).$(3).Package.appxmanifest
#$(call WINRT_BUILD_WIN basepath name)
WINRT_BUILD_WIN=$(MSBUILD) $(call WINRT_PROJECT,$(1),$(2),Windows) //p:Configuration=Release //p:Platform=Win32 //v:m
#$(call WINRT_BUILD_WP basepath name)
WINRT_BUILD_WP=$(MSBUILD) $(call WINRT_PROJECT,$(1),$(2),WindowsPhone) //p:Configuration=Release //p:Platform=ARM //v:m


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
	$(call WINRT_BUILD_WIN,lua/luawinrt,luawinrt)
	$(call WINRT_BUILD_WP,lua/luawinrt,luawinrt)

winrt.gvfs:
	$(call WINRT_BUILD_WIN,libgvfs/libgvfswinrt,libgvfswinrt)
	$(call WINRT_BUILD_WP,libgvfs/libgvfswinrt,libgvfswinrt)

winrt.libs: winrt.lua winrt.gvfs

winrt.plugins:
	$(call WINRT_BUILD_WIN,plugins/luasocket/source/winrt/luasocket,luasocket)
	$(call WINRT_BUILD_WP,plugins/luasocket/source/winrt/luasocket,luasocket)
	mkdir -p $(RELEASE)/All\ Plugins/luasocket/bin/WinRT
	cp Release/All\ Plugins/luasocket/bin/WinRT/Release/ARM/*.lib $(RELEASE)/All\ Plugins/luasocket/bin/WinRT/
	cp Release/All\ Plugins/luasocket/bin/WinRT/Release/Win32/*.lib $(RELEASE)/All\ Plugins/luasocket/bin/WinRT/
	
winrt.core: winrt.libs winrt.shaders
	$(call WINRT_BUILD_WIN,winrt,gideros)
	#X86 Release version for Windows
	mkdir -p winrt/Release/gideros.Windows
	cp winrt/gideros/gideros.Windows/Release/gideros.Windows/gideros.Windows.lib winrt/Release/gideros.Windows
	mkdir -p winrt/Release/luawinrt.Windows
	cp lua/luawinrt/luawinrt/luawinrt.Windows/Release/luawinrt.Windows/luawinrt.Windows.lib winrt/Release/luawinrt.Windows
	mkdir -p winrt/Release/libgvfswinrt.Windows
	cp libgvfs/libgvfswinrt/libgvfswinrt/libgvfswinrt.Windows/Release/libgvfswinrt.Windows/libgvfswinrt.Windows.lib winrt/Release/libgvfswinrt.Windows
	$(call WINRT_BUILD_WP,winrt,gideros)
	#ARM release version for WinPhone
	mkdir -p winrt/ARM/Release/gideros.WindowsPhone
	cp winrt/gideros/gideros.WindowsPhone/ARM/Release/gideros.WindowsPhone/gideros.WindowsPhone.lib winrt/ARM/Release/gideros.WindowsPhone
	mkdir -p winrt/ARM/Release/luawinrt.WindowsPhone
	cp lua/luawinrt/luawinrt/luawinrt.WindowsPhone/ARM/Release/luawinrt.WindowsPhone/luawinrt.WindowsPhone.lib winrt/ARM/Release/luawinrt.WindowsPhone
	mkdir -p winrt/ARM/Release/libgvfswinrt.WindowsPhone
	cp libgvfs/libgvfswinrt/libgvfswinrt/libgvfswinrt.WindowsPhone/ARM/Release/libgvfswinrt.WindowsPhone/libgvfswinrt.WindowsPhone.lib winrt/ARM/Release/libgvfswinrt.WindowsPhone
	 
winrt.template: winrt.core winrt.plugins 
	rm -rf $(RELEASE)/Templates/VisualStudio
	mkdir -p $(RELEASE)/Templates
	#Non XAML
	#cp -r ui/Templates/VisualStudio $(RELEASE)/Templates
	#cp $(WINRT_PLAYERDIR)/*.cpp "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	#cp winrt/gideros/gideros.Shared/giderosapi.h "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	#XAML
	mkdir -p "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	cd $(WINRT_PLAYERDIR); git archive master | tar -x -C "../$(RELEASE)/Templates/VisualStudio/WinRT Template"
	sed -e ':1;s/PLUGINS-START/ /;t2;:3;n;b1;:2;g;n;s/PLUGINS-END/ /;t3;b2' $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Shared/$(WINRT_PLAYERSUBDIR).Shared.vcxitems | sed -e 's/\$\(GidLibsPath\)/\.\.\\\.\./' >"$(RELEASE)/Templates/VisualStudio/WinRT Template/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Shared/$(WINRT_PLAYERSUBDIR).Shared.vcxitems"
	sed -e ':1;s/PLUGINS-START/ /;t2;:3;n;b1;:2;g;n;s/PLUGINS-END/ /;t3;b2' $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/$(WINRT_PLAYERSUBDIR).Windows.vcxproj >"$(RELEASE)/Templates/VisualStudio/WinRT Template/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/$(WINRT_PLAYERSUBDIR).Windows.vcxproj"
	sed -e ':1;s/PLUGINS-START/ /;t2;:3;n;b1;:2;g;n;s/PLUGINS-END/ /;t3;b2' $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).WindowsPhone/$(WINRT_PLAYERSUBDIR).WindowsPhone.vcxproj >"$(RELEASE)/Templates/VisualStudio/WinRT Template/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).WindowsPhone/$(WINRT_PLAYERSUBDIR).WindowsPhone.vcxproj"
	cp winrt/gideros/gideros.Shared/giderosapi.h "$(RELEASE)/Templates/VisualStudio/WinRT Template/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Shared/"
	
	#X86 Release version for Windows
	cp winrt/gideros/gideros.Windows/Release/gideros.Windows/gideros.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	cp lua/luawinrt/luawinrt/luawinrt.Windows/Release/luawinrt.Windows/luawinrt.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	cp libgvfs/libgvfswinrt/libgvfswinrt/libgvfswinrt.Windows/Release/libgvfswinrt.Windows/libgvfswinrt.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	#ARM release version for WinPhone
	cp winrt/gideros/gideros.WindowsPhone/ARM/Release/gideros.WindowsPhone/gideros.WindowsPhone.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	cp lua/luawinrt/luawinrt/luawinrt.WindowsPhone/ARM/Release/luawinrt.WindowsPhone/luawinrt.WindowsPhone.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	cp libgvfs/libgvfswinrt/libgvfswinrt/libgvfswinrt.WindowsPhone/ARM/Release/libgvfswinrt.WindowsPhone/libgvfswinrt.WindowsPhone.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	#Plugins libs
	#cp $(RELEASE)/All\ Plugins/luasocket/bin/WinRT/*.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"

winrt.player: winrt.template
	@echo "VERSION" $(GIDEROS_VERSION)
	cp winrt/gideros/gideros.Shared/giderosapi.h $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Shared/
	rm -rf /c/winrt_player
	cp $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).WindowsPhone/Package.appxmanifest $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).WindowsPhone/Package.appxmanifest.bak
	sed -e 's/Version="[^"]*"/Version="$(WINRT_APPX_GIDVERSION)"/'	$(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).WindowsPhone/Package.appxmanifest.bak >$(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).WindowsPhone/Package.appxmanifest
	$(MSBUILD) $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).WindowsPhone/$(WINRT_PLAYERSUBDIR).WindowsPhone.vcxproj //t:Publish //p:Configuration=Release //p:Platform=ARM //p:AppxBundle=Always //v:m
	cp $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).WindowsPhone/Package.appxmanifest.bak $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).WindowsPhone/Package.appxmanifest
	rm $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).WindowsPhone/Package.appxmanifest.bak
	cp $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest.bak
	sed -e 's/Version="[^"]*"/Version="$(WINRT_APPX_GIDVERSION)"/'	$(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest.bak >$(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest
	$(MSBUILD) $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/$(WINRT_PLAYERSUBDIR).Windows.vcxproj //t:Publish //p:Configuration=Release //p:Platform=Win32 //p:AppxBundle=Always //V:m
	cp $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest.bak $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest
	rm $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/Package.appxmanifest.bak
	mkdir -p $(RELEASE)/Players
	rm -rf $(RELEASE)/Players/WinRT
	#mv /c/winrt_player $(RELEASE)/Players/WinRT
	mkdir -p $(RELEASE)/Players/WinRT
	cp -r $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).WindowsPhone/AppPackages/$(WINRT_PLAYERSUBDIR).WindowsPhone/* $(RELEASE)/Players/WinRT
	cp -r $(WINRT_PLAYERDIR)/$(WINRT_PLAYERSUBDIR)/$(WINRT_PLAYERSUBDIR).Windows/AppPackages/$(WINRT_PLAYERSUBDIR).Windows/* $(RELEASE)/Players/WinRT
	
winrt.install: winrt.player

winrt.clean:
	#DUMMY

winrt.zip: winrt.player
	rm -f $@
	P=$(PWD); cd $(RELEASE); zip -r $$P/$@ "Templates/VisualStudio/WinRT Template" Players/WinRT "All Plugins"/*/bin/WinRT
			