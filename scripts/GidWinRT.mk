WINRT_SHADERS=Basic Color Texture TextureColor Particle PathFillC PathStrokeC PathStrokeLC
WINRT_SHADERS_PATH=2dsg/gfxbackends/dx11
WINRT_SHADERS_FILE=dx11_shaders.c
BIN2C=$(ROOT)/scripts/bin2c


winrt.shaders: $(BIN2C)
	rm -f $(WINRT_SHADERS_PATH)/$(WINRT_SHADERS_FILE)
	for s in $(WINRT_SHADERS); do $(FXC) //T vs_4_0_level_9_3 //E VShader //Fo $(WINRT_SHADERS_PATH)/v$$s.cso $(WINRT_SHADERS_PATH)/$$s.hlsl; done
	for s in $(WINRT_SHADERS); do $(FXC) //T ps_4_0_level_9_3 //E PShader //Fo $(WINRT_SHADERS_PATH)/p$$s.cso $(WINRT_SHADERS_PATH)/$$s.hlsl; done
	P=$(PWD); cd $(WINRT_SHADERS_PATH); for s in $(WINRT_SHADERS); do $$P/$(BIN2C) p$$s.cso >> $(WINRT_SHADERS_FILE); $$P/$(BIN2C) v$$s.cso >> $(WINRT_SHADERS_FILE); done
	rm -f $(WINRT_SHADERS_PATH)/*.cso	
	#cp winrt/*.cso winrt_example/giderosgame/giderosgame.Windows/Assets
	#cp winrt/*.cso winrt_example/giderosgame/giderosgame.WindowsPhone/Assets
	#cp winrt/*.cso "ui/Templates/VisualStudio/WinRT Template/giderosgame/giderosgame.Windows/Assets"
	#cp winrt/*.cso "ui/Templates/VisualStudio/WinRT Template/giderosgame/giderosgame.WindowsPhone/Assets"

winrt.lua:
	$(MSBUILD) lua/luawinrt/luawinrt/luawinrt.WindowsPhone/luawinrt.WindowsPhone.vcxproj //p:Configuration=Release //p:Platform=ARM 
	$(MSBUILD) lua/luawinrt/luawinrt/luawinrt.Windows/luawinrt.Windows.vcxproj //p:Configuration=Release //p:Platform=Win32 

winrt.gvfs:
	$(MSBUILD) libgvfs/libgvfswinrt/libgvfswinrt/libgvfswinrt.WindowsPhone/libgvfswinrt.WindowsPhone.vcxproj //p:Configuration=Release //p:Platform=ARM 
	$(MSBUILD) libgvfs/libgvfswinrt/libgvfswinrt/libgvfswinrt.Windows/libgvfswinrt.Windows.vcxproj //p:Configuration=Release //p:Platform=Win32 

winrt.libs: winrt.lua winrt.gvfs

winrt.plugins:
	$(MSBUILD) plugins/luasocket/source/winrt/luasocket/luasocket/luasocket.WindowsPhone/luasocket.WindowsPhone.vcxproj //p:Configuration=Release //p:Platform=ARM 
	$(MSBUILD) plugins/luasocket/source/winrt/luasocket/luasocket/luasocket.Windows/luasocket.Windows.vcxproj //p:Configuration=Release //p:Platform=Win32 
	
winrt.core: winrt.libs winrt.shaders
	$(MSBUILD) winrt/gideros/gideros.WindowsPhone/gideros.WindowsPhone.vcxproj //p:Configuration=Release //p:Platform=ARM 
	$(MSBUILD) winrt/gideros/gideros.Windows/gideros.Windows.vcxproj //p:Configuration=Release //p:Platform=Win32 
	 
winrt.template: winrt.core winrt.plugins 
	rm -rf $(RELEASE)/Templates/VisualStudio
	mkdir -p $(RELEASE)/Templates
	cp -r ui/Templates/VisualStudio $(RELEASE)/Templates
	#X86 Release version for Windows
	cp winrt/gideros/gideros.Windows/Release/gideros.Windows/gideros.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	cp lua/luawinrt/luawinrt/luawinrt.Windows/Release/luawinrt.Windows/luawinrt.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	cp libgvfs/libgvfswinrt/libgvfswinrt/libgvfswinrt.Windows/Release/libgvfswinrt.Windows/libgvfswinrt.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	cp $(RELEASE)/AllPlugins/WinRT/Release/Win32/*.Windows.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	#ARM release version for WinPhone
	cp winrt/gideros/gideros.WindowsPhone/ARM/Release/gideros.WindowsPhone/gideros.WindowsPhone.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	cp lua/luawinrt/luawinrt/luawinrt.WindowsPhone/ARM/Release/luawinrt.WindowsPhone/luawinrt.WindowsPhone.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	cp libgvfs/libgvfswinrt/libgvfswinrt/libgvfswinrt.WindowsPhone/ARM/Release/libgvfswinrt.WindowsPhone/libgvfswinrt.WindowsPhone.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"
	cp $(RELEASE)/AllPlugins/WinRT/Release/ARM/*.WindowsPhone.lib "$(RELEASE)/Templates/VisualStudio/WinRT Template"

winrt.tgz: winrt.template
	tar -czf $@ "$(RELEASE)/Templates/VisualStudio/WinRT Template"
			