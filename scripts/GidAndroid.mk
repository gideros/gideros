#ANDROID
ANDROID_ARCHS=armeabi-v7a x86 x86_64 arm64-v8a

export JAVA_HOME

android.clean: androidlibs.clean androidso.clean androidplugins.clean
	cd $(ROOT)/android/GiderosAndroidPlayer; echo "sdk.dir=$(ANDROID_HOME)" >local.properties; ./gradlew clean; rm -f local.properties *.aar *.jar
	

android: androidlibs androidso androidplugins
	cd $(ROOT)/android/GiderosAndroidPlayer; echo "sdk.dir=$(ANDROID_HOME)" >local.properties; ./gradlew assembleRelease
	mv $(ROOT)/android/GiderosAndroidPlayer/app/build/outputs/aar/app-release.aar $(ROOT)/android/GiderosAndroidPlayer/gideros.aar

android.aar: 
	cd $(ROOT)/android/GiderosAndroidPlayer; echo "sdk.dir=$(ANDROID_HOME)" >local.properties; ./gradlew assembleRelease
	mv $(ROOT)/android/GiderosAndroidPlayer/app/build/outputs/aar/app-release.aar $(ROOT)/android/GiderosAndroidPlayer/gideros.aar

android.install: android androidlibs.install androidso.install oculusso.install androidplugins.install
	cp $(ROOT)/android/GiderosAndroidPlayer/gideros.aar $(RELEASE)/Templates/AndroidStudio/Android\ Template/app/libs	
	mkdir -p $(RELEASE)/Players
	cd $(RELEASE); Tools/gdrexport.exe -platform APK Examples/Misc/GiderosPlayer/GiderosPlayer.gproj Players
	mv $(RELEASE)/Players/GiderosPlayer/GiderosPlayer-debug.apk $(RELEASE)/Players/GiderosAndroidPlayer.apk
	rm -rf $(RELEASE)/Players/GiderosPlayer

androidlibs: libgvfs.androidlib lua.androidlib

androidlibs.clean: libgvfs.androidlib.clean lua.androidlib.clean

androidso: versioning androidso.prep
	cd $(ROOT)/android/lib;$(NDKBUILD) $(MAKEJOBS)
	rm -rf $(ROOT)/Sdk/lib/android
	mkdir -p $(ROOT)/Sdk/lib/android
	cp -R $(ROOT)/android/lib/libs/* $(ROOT)/Sdk/lib/android 

oculusso: versioning androidso.prep
	cd $(ROOT)/android/lib;OCULUS=y $(NDKBUILD) $(MAKEJOBS)
	rm -rf $(ROOT)/Sdk/lib/android
	mkdir -p $(ROOT)/Sdk/lib/android
	cp -R $(ROOT)/android/lib/libs/* $(ROOT)/Sdk/lib/android 

androidplugins: $(addsuffix .androidplugin,$(PLUGINS_ANDROID))

androidplugins.clean: $(addsuffix .androidplugin.clean,$(PLUGINS_ANDROID))

androidso.prep:

androidso.clean:
	rm -rf $(ROOT)/android/lib/libs
	rm -rf $(ROOT)/android/lib/obj

%.androidlib:
	cd $(ROOT)/$*; $(NDKBUILD) $(MAKEJOBS)

%.androidplugin:
	@cd $(ROOT)/plugins/$*/source; if [ -d "Android" ]; then cd Android; fi;\
	$(NDKBUILD) $(MAKEJOBS);\
	for a in $(ANDROID_ARCHS); do \
	rm -f libs/$$a/libgideros.so libs/$$a/liblua.so libs/$$a/libgvfs.so; done; 

%.androidlib.clean:
	rm -rf $(ROOT)/$*/libs $(ROOT)/$*/obj

androidso.install: androidso
	cp -R $(ROOT)/android/lib/libs/. $(RELEASE)/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/

oculusso.install: oculusso
	cp -R $(ROOT)/android/lib/libs/. $(RELEASE)/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/

androidplugins.install: androidplugins $(addsuffix .androidplugin.install,$(PLUGINS_ANDROID))

androidlibs.install: androidlibs

%.androidplugin.install: %.plugin.install
	@mkdir -p $(RELEASE)/All\ Plugins/$(notdir $*)/bin/Android
	@cd $(ROOT)/plugins/$*/source; echo "Installing $*"; \
	if [ -d "Android" ]; then cd Android; fi;	\
	cp -r libs $(CURDIR)/$(RELEASE)/All\ Plugins/$(notdir $*)/bin/Android/; \
	if [ -d "res" ]; then \
	cp -r res $(CURDIR)/$(RELEASE)/All\ Plugins/$(notdir $*)/bin/Android/; \
	fi;\
	if [ -d "assets" ]; then \
	cp -r assets $(CURDIR)/$(RELEASE)/All\ Plugins/$(notdir $*)/bin/Android/; \
	fi;\
	if [ -d "src" ]; then \
	cp -r src $(CURDIR)/$(RELEASE)/All\ Plugins/$(notdir $*)/bin/Android/; \
	fi;

%.androidplugin.clean:
	@cd $(ROOT)/plugins/$*/source; if [ -d "Android" ]; then cd Android; fi; for l in $(ANDROID_ARCHS); do rm -rf libs/$$l/*.so; done; rm -rf obj
	@rm -rf $(RELEASE)/All\ Plugins/$(notdir $*)/bin/Android
		