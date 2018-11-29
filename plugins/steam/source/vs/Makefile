OUTLIB=steamplugin/Release/steamplugin.dll
STEAM_REDIST=$(STEAMSDK)/sdk/redistributable_bin

build:
	STEAMSDK=$(STEAMSDK) $(MSBUILD) steamplugin/steamplugin.vcxproj //p:Configuration=Release //p:Platform=Win32 //v:m;
	cp $(OUTLIB) . 
	mkdir deps
	cp $(STEAM_REDIST)/*.dll ./deps

clean:
	$(MSBUILD) steamplugin/steamplugin.vcxproj //t:Clean //p:Configuration=Release //p:Platform=Win32 //v:m;
	rm -f *.dll
	rm -rf deps

