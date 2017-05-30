#Standalone build definitions
GIDEROS_SDK="/c/Program Files (x86)/Gideros/Sdk"
NDKBUILD=cmd //c /c/Applications/android-ndk-r14b/ndk-build.cmd

#Compute plugin name and build paths
PLUGIN:=$(lastword $(subst /, ,$(dir $(PWD)/$(strip $(MAKEFILE_LIST)))))
PROOT:=$(patsubst %/,%,$(dir $(strip $(MAKEFILE_LIST))))
CURDIR:=$(PWD)
PBUILD=$(PROOT)/Build

#Standalone build
standalone:
	rm -rf $(PBUILD)
	mkdir -p $(PBUILD)
	cp -r $(GIDEROS_SDK) $(PBUILD)
	@cd source; BDIR=../..; if [ -d "Android" ]; then BDIR=../$$BDIR; cd Android; fi;\
	GIDEROS_SDK=$$BDIR/$(PBUILD)/Sdk $(NDKBUILD);\
	rm -f libs/armeabi/libgideros.so libs/armeabi/liblua.so libs/armeabi/libgvfs.so;\
	rm -f libs/armeabi-v7a/libgideros.so libs/armeabi-v7a/liblua.so libs/armeabi-v7a/libgvfs.so;\
	rm -f libs/x86/libgideros.so libs/x86/liblua.so libs/x86/libgvfs.so
	@mkdir -p $(PBUILD)/$(PLUGIN)/bin/Android
	@cd source;  \
	if [ -d "Android" ]; then cd Android; fi;	\
	cp -r libs $(CURDIR)/$(PBUILD)/$(PLUGIN)/bin/Android/; \
	if [ -d "res" ]; then \
	cp -r res $(CURDIR)/$(PBUILD)/$(PLUGIN)/bin/Android/; \
	fi;\
	if [ -d "assets" ]; then \
	cp -r assets $(CURDIR)/$(PBUILD)/$(PLUGIN)/bin/Android/; \
	fi;\
	if [ -d "src" ]; then \
	cp -r src $(CURDIR)/$(PBUILD)/$(PLUGIN)/bin/Android/; \
	fi;	
	cp -r $(PROOT)/source $(PBUILD)/$(PLUGIN) 
	if [ -d "doc" ]; then \
	cp -r $(PROOT)/doc $(PBUILD)/$(PLUGIN); \
	fi; 
	rm -rf $(PBUILD)/$(PLUGIN)/source/Android
	cp $(PROOT)/$(PLUGIN).gplugin $(PBUILD)/$(PLUGIN)

#Gideros source tree build
gideros:
	rm -rf $(PBUILD)
	mkdir -p $(PBUILD)
	$(MAKE) -f scripts/Makefile.gid ../$(PROOT).androidplugin
	$(MAKE) -f scripts/Makefile.gid ../$(PROOT).androidplugin.install
	cp -r Build.Win/All\ Plugins/$(PLUGIN) $(PBUILD)
	cp -r $(PROOT)/source $(PBUILD)/$(PLUGIN) 
	#cp -r $(PROOT)/doc $(PBUILD)/$(PLUGIN) 
	rm -rf $(PBUILD)/$(PLUGIN)/source/Android
	cp $(PROOT)/$(PLUGIN).gplugin $(PBUILD)/$(PLUGIN)
	rm -rf Build.Win/All\ Plugins/$(PLUGIN)
	cp -r $(PBUILD)/$(PLUGIN) Build.Win/All\ Plugins

#Cleanup
clean:
	rm -rf $(PBUILD)
	rm -rf $(PROOT)/source/Android/obj
	rm -rf $(PROOT)/source/Android/libs
	