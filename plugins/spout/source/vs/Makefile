OUTLIB=spoutplugin/spoutplugin/Release/spoutplugin.dll

build:
	$(MSBUILD) spoutplugin/spoutplugin/spoutplugin.vcxproj //p:Configuration=Release //p:Platform=Win32 //v:m;
	cp $(OUTLIB) .
	cd qt; qmake spout.pro; make release
	cp qt/release/spout.dll .

clean:
	$(MSBUILD) spoutplugin/spoutplugin/spoutplugin.vcxproj //t:Clean //p:Configuration=Release //p:Platform=Win32 //v:m;
	cd qt; make clean
	rm -f *.dll

