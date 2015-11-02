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
	
winrt.core: winrt.shaders
	$(MSBUILD) winrt/gideros.sln //p:Configuration=Release //p:Platform=ARM 
	$(MSBUILD) winrt/gideros.sln //p:Configuration=Release //p:Platform=Win32
	 
		