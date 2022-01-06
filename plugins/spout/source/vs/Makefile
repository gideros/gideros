OUTLIB=spoutplugin/spoutplugin/x64/Release/spoutplugin.dll

build:
	$(MSBUILD) spoutplugin/spoutplugin/spoutplugin.vcxproj //p:Configuration=Release //p:Platform=x64 //v:m;
	cp $(OUTLIB) .
	cd qt; qmake spout.pro; make release
	cp qt/release/spout.dll .
	mkdir -p deps
	mv spoutplugin.dll deps/

clean:
	$(MSBUILD) spoutplugin/spoutplugin/spoutplugin.vcxproj //t:Clean //p:Configuration=Release //p:Platform=x64 //v:m;
	cd qt; make clean
	rm -f *.dll
	rm -rf deps

