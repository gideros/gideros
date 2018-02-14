OUTLIB=steamplugin/build/Release/libsteamplugin.dylib
STEAM_REDIST=$(STEAMSDK)/sdk/redistributable_bin/osx32

build:
	cd steamplugin; STEAMSDK=$(STEAMSDK) xcodebuild
	install_name_tool -change libgideros.1.dylib @executable_path/../Frameworks/libgideros.1.dylib $(OUTLIB) 
	install_name_tool -change liblua.1.dylib @executable_path/../Frameworks/liblua.1.dylib $(OUTLIB)
	cp $(OUTLIB) . 
	cp $(STEAM_REDIST)/*.dylib .

clean:
	cd steamplugin; xcodebuild clean
	rm -f *.dylib

