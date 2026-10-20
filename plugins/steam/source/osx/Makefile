OUTLIB=steamplugin/build/Release/libsteamplugin.dylib
STEAM_REDIST=$(STEAMSDK)/sdk/redistributable_bin/osx

build:
	cd steamplugin; STEAMSDK=$(STEAMSDK) xcodebuild STEAMSDK=$(STEAMSDK)
	install_name_tool -change libgideros.1.dylib @executable_path/../Frameworks/libgideros.1.dylib $(OUTLIB) 
	install_name_tool -change liblua.1.dylib @executable_path/../Frameworks/liblua.1.dylib $(OUTLIB)
	install_name_tool -change libgid.1.dylib @executable_path/../Frameworks/libgid.1.dylib $(OUTLIB)
	cp $(OUTLIB) . 
	cp $(STEAM_REDIST)/*.dylib ./libsteam_api.dylib

clean:
	cd steamplugin; xcodebuild clean
	rm -f *.dylib

