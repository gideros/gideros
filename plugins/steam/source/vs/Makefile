OUTLIB=steamplugin/x64/Release/steamplugin.dll
STEAM_REDIST=$(STEAMSDK)/sdk/redistributable_bin

#dlltool.exe  -l gid64.lib -d gid.def -D libgid.dll

build:
	STEAMSDK=$(STEAMSDK) $(MSBUILD) steamplugin/steamplugin.vcxproj //p:Configuration=Release //p:Platform=x64 //v:m;
	cp $(OUTLIB) . 
	mkdir -p deps
	cp $(STEAM_REDIST)/*.dll ./deps

clean:
	$(MSBUILD) steamplugin/steamplugin.vcxproj //t:Clean //p:Configuration=Release //p:Platform=x64 //v:m;
	rm -f *.dll
	rm -rf deps

