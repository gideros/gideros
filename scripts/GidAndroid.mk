#ANDROID
android.clean: androidlibs.clean androidso.clean
	cd $(ROOT)/android/GiderosAndroidPlayer; $(ANT) clean

android: androidlibs androidso androidplugins
	rm -rf $(ROOT)/android/GiderosAndroidPlayer/src/com/android
	rm -rf $(ROOT)/android/GiderosAndroidPlayer/src/com/giderosmobile/android/plugins
	cd $(ROOT)/android/GiderosAndroidPlayer; $(ANT) debug
	rm -f $(ROOT)/android/GiderosAndroidPlayer/bin/classes/com/giderosmobile/android/player/BuildConfig.class
	rm -f $(ROOT)/android/GiderosAndroidPlayer/bin/classes/com/giderosmobile/android/player/GiderosAndroidPlayerActivity.class
	rm -f $(ROOT)/android/GiderosAndroidPlayer/bin/classes/com/giderosmobile/android/player/GiderosGLSurfaceView.class
	rm -f $(ROOT)/android/GiderosAndroidPlayer/bin/classes/com/giderosmobile/android/player/GiderosRenderer.class
	rm -f $(ROOT)/android/GiderosAndroidPlayer/bin/classes/com/giderosmobile/android/player/R.class
	rm -f $(ROOT)/android/GiderosAndroidPlayer/bin/classes/com/giderosmobile/android/player/R*.class
	cd $(ROOT)/android/GiderosAndroidPlayer/bin/classes; $(JAR) cvf gideros.jar com
	mv $(ROOT)/android/GiderosAndroidPlayer/bin/classes/gideros.jar $(ROOT)/android/GiderosAndroidPlayer/

android.install: android androidlibs.install androidso.install androidplugins.install
	cp $(ROOT)/android/GiderosAndroidPlayer/gideros.jar $(RELEASE)/Templates/Eclipse/Android\ Template
	rm -rf $(ROOT)/android/GiderosAndroidPlayer/libs
	cp -R $(RELEASE)/Templates/Eclipse/Android\ Template/libs $(ROOT)/android/GiderosAndroidPlayer
	cd $(ROOT)/android/GiderosAndroidPlayer; $(ANT) debug;
	mv $(ROOT)/android/GiderosAndroidPlayer/bin/GiderosAndroidPlayer-debug.apk $(RELEASE)/GiderosAndroidPlayer.apk
	#cp -R $(ROOT)/android/GiderosAndroidPlayer/assets $(RELEASE)/Templates/Eclipse/Android\ Template
	#cp $(ROOT)/android/GiderosAndroidPlayer/AndroidManifest.xml $(RELEASE)/Templates/Eclipse/Android\ Template

androidlibs: libgvfs.androidlib lua.androidlib

androidlibs.clean: libgvfs.androidlib.clean lua.androidlib.clean

androidso: androidso.prep
	cd $(ROOT)/android/lib;$(NDKBUILD)
	rm -rf $(ROOT)/Sdk/lib/android
	mkdir -p $(ROOT)/Sdk/lib/android
	cp -R $(ROOT)/android/lib/libs/* $(ROOT)/Sdk/lib/android 

androidplugins: $(addsuffix .androidplugin,$(PLUGINS_ANDROID))

androidso.prep:

androidso.clean:
	rm -rf $(ROOT)/android/lib/libs
	rm -rf $(ROOT)/android/lib/obj

%.androidlib:
	cd $(ROOT)/$*; $(NDKBUILD)

%.androidplugin:
	cd $(ROOT)/plugins/$*/source; if [ -d "Android" ]; then cd Android; fi;	$(NDKBUILD)

%.androidlib.clean:
	rm -rf $(ROOT)/$*/libs $(ROOT)/$*/obj

androidso.install: androidso
	mkdir -p $(RELEASE)/Templates/Eclipse/Android\ Template/jni
	cp $(ROOT)/android/lib/jni/Application.mk $(RELEASE)/Templates/Eclipse/Android\ Template/jni
	cp -R $(ROOT)/android/lib/libs $(RELEASE)/Templates/Eclipse/Android\ Template/

androidplugins.install: androidplugins $(addsuffix .androidplugin.install,$(PLUGINS_ANDROID))

androidlibs.install: androidlibs

%.androidplugin.install:
	cd $(ROOT)/plugins/$*/source; if [ -d "Android" ]; then cd Android; fi;	\
	cp -R libs $(CURDIR)/$(RELEASE)/Templates/Eclipse/Android\ Template/; \
	if [ -d "com" ]; then \
	cp -R com $(CURDIR)/$(ROOT)/android/GiderosAndroidPlayer/src;\
	fi
	#cp -R com $(CURDIR)/$(RELEASE)/Templates/Eclipse/Android\ Template/src;\

		