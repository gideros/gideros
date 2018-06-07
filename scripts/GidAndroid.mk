#ANDROID
ANDROID_ARCHS=armeabi-v7a x86 x86_64 arm64-v8a

android.clean: androidlibs.clean androidso.clean androidplugins.clean
	cd $(ROOT)/android/GiderosAndroidPlayer; $(ANT) clean

android: androidlibs androidso androidplugins
	rm -rf $(ROOT)/android/GiderosAndroidPlayer/src/com/android
	rm -rf $(ROOT)/android/GiderosAndroidPlayer/src/com/giderosmobile/android/plugins
	mkdir -p $(ROOT)/android/GiderosAndroidPlayer/libs
	cp $(ROOT)/android/GiderosAndroidPlayer/lib/*.jar $(ROOT)/android/GiderosAndroidPlayer/libs/
	cd $(ROOT)/android/GiderosAndroidPlayer; $(ANT) debug
	rm -f $(ROOT)/android/GiderosAndroidPlayer/bin/classes/com/giderosmobile/android/player/BuildConfig.class
	rm -f $(ROOT)/android/GiderosAndroidPlayer/bin/classes/com/giderosmobile/android/player/GiderosAndroidPlayerActivity.class
	rm -f $(ROOT)/android/GiderosAndroidPlayer/bin/classes/com/giderosmobile/android/GiderosSettings.class
	rm -f $(ROOT)/android/GiderosAndroidPlayer/bin/classes/com/giderosmobile/android/player/GiderosGLSurfaceView.class
	rm -f $(ROOT)/android/GiderosAndroidPlayer/bin/classes/com/giderosmobile/android/player/GiderosRenderer.class
	rm -f $(ROOT)/android/GiderosAndroidPlayer/bin/classes/com/giderosmobile/android/player/R.class
	rm -f $(ROOT)/android/GiderosAndroidPlayer/bin/classes/com/giderosmobile/android/player/R*.class
	cd $(ROOT)/android/GiderosAndroidPlayer/bin/classes; $(JAR) cvf gideros.jar com
	mv $(ROOT)/android/GiderosAndroidPlayer/bin/classes/gideros.jar $(ROOT)/android/GiderosAndroidPlayer/

android.install: android androidlibs.install androidso.install androidplugins.install
	cp $(ROOT)/android/GiderosAndroidPlayer/gideros.jar $(RELEASE)/Templates/Eclipse/Android\ Template
	cp $(ROOT)/android/GiderosAndroidPlayer/gideros.jar $(RELEASE)/Templates/AndroidStudio/Android\ Template/app/libs	
	rm -rf $(ROOT)/android/GiderosAndroidPlayer/libs
	cp -R $(RELEASE)/Templates/Eclipse/Android\ Template/libs $(ROOT)/android/GiderosAndroidPlayer
	for p in $(PLUGINS_DEFAULT); do \
	cd $(ROOT)/plugins/$$p/source; if [ -d "Android" ]; then cd Android; fi;	\
	cp -R libs $(CURDIR)/android/GiderosAndroidPlayer; \
	cd $(CURDIR); done
	cd $(ROOT)/android/GiderosAndroidPlayer; $(ANT) debug;
	mkdir -p $(RELEASE)/Players
	mv $(ROOT)/android/GiderosAndroidPlayer/bin/GiderosAndroidPlayer-debug.apk $(RELEASE)/Players/GiderosAndroidPlayer.apk
	#cp -R $(ROOT)/android/GiderosAndroidPlayer/assets $(RELEASE)/Templates/Eclipse/Android\ Template
	#cp $(ROOT)/android/GiderosAndroidPlayer/AndroidManifest.xml $(RELEASE)/Templates/Eclipse/Android\ Template

androidlibs: libgvfs.androidlib lua.androidlib

androidlibs.clean: libgvfs.androidlib.clean lua.androidlib.clean

androidso: versioning androidso.prep
	cd $(ROOT)/android/lib;$(NDKBUILD)
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
	cd $(ROOT)/$*; $(NDKBUILD)

%.androidplugin:
	@cd $(ROOT)/plugins/$*/source; if [ -d "Android" ]; then cd Android; fi;\
	$(NDKBUILD);\
	for a in $(ANDROID_ARCHS); do \
	rm -f libs/$$a/libgideros.so libs/$$a/liblua.so libs/$$a/libgvfs.so; done; 

%.androidlib.clean:
	rm -rf $(ROOT)/$*/libs $(ROOT)/$*/obj

androidso.install: androidso
	mkdir -p $(RELEASE)/Templates/Eclipse/Android\ Template/jni
	cp $(ROOT)/android/lib/jni/Application.mk $(RELEASE)/Templates/Eclipse/Android\ Template/jni
	cp -R $(ROOT)/android/lib/libs $(RELEASE)/Templates/Eclipse/Android\ Template/
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
		