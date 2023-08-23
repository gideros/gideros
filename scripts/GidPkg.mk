.PHONY: build.mac.pkg fetch.mac.pkg push.mac.pkg sync.mac.pkg clean.pkg build.all.pkg all full

TXTFILES=donors.txt licenses.txt

gendoc:
	-echo "\
	cd /opt/gid_docs;\
	./sync_docs.sh;\
	exit;\
	" |	ssh root@giderosmobile.com

updatedoc:
	-echo "\
	cd /opt/gid_docs;\
	./update_docs.sh;\
	exit;\
	" |	ssh root@giderosmobile.com

fetchdoc:
	rm -rf docs.giderosmobile.com _docs.zip
	#-wget -nv --recursive --page-requisites --html-extension --convert-links --restrict-file-names=windows --domains docs.giderosmobile.com --no-parent http://docs.giderosmobile.com/
	#-wget -nv "http://docs.giderosmobile.com/docs.zip" -O _docs.zip
	#unzip _docs.zip
	mkdir -p $(RELEASE)
	rm -rf $(RELEASE)/Documentation _docs.zip
	#mv docs.giderosmobile.com $(RELEASE)/Documentation
	#-wget -nv "http://docs.giderosmobile.com/reference/autocomplete.php" -O $(RELEASE)/Resources/gideros_annot.api
	cp $(addprefix $(ROOT)/,$(TXTFILES)) $(RELEASE)

#MAC remote build
build.mac.pkg:
	echo "\
	source /etc/profile;\
	cd $(MAC_PATH);\
	make -f scripts/Makefile.gid;\
	exit;\
	" |	ssh $(MAC_HOST)

fetch.mac.pkg:
	echo "\
	source /etc/profile;\
	cp -r $(MAC_PATH)/$(SDK) $(MAC_PATH)/Build.Mac;\
	cd $(MAC_PATH)/Build.Mac;\
	rm -f BuildMac.zip;\
	zip -r BuildMac.zip Sdk Players Templates All\\ Plugins;\
	exit;\
	" |	ssh $(MAC_HOST)
	scp -B $(MAC_HOST):$(MAC_PATH)/Build.Mac/BuildMac.zip $(RELEASE)/BuildMac.zip

fetchbundle.mac.pkg:
	scp -B $(MAC_HOST):$(MAC_PATH)/Gideros.pkg $(ROOT)/Gideros.pkg

bundle.mac.pkg:
	echo "\
	source /etc/profile;\
	cd $(MAC_PATH);\
	make -f scripts/Makefile.gid bundle.installer;\
	exit;\
	" |	ssh $(MAC_HOST)

clean.mac.pkg:
	echo "\
	source /etc/profile;\
	cd $(MAC_PATH);\
	git pull;\
	make -f scripts/Makefile.gid clean;\
	exit;\
	" |	ssh $(MAC_HOST)

#Linux remote build
build.linux.pkg:
	echo "\
	source /etc/profile;\
	cd $(LINUX_PATH);\
	make -f scripts/Makefile.gid;\
	exit;\
	" |	ssh $(LINUX_HOST)

fetch.linux.pkg:
	echo "\
	source /etc/profile;\
	cp -r $(LINUX_PATH)/$(SDK) $(LINUX_PATH)/Build.Linux;\
	cd $(LINUX_PATH)/Build.Linux;\
	rm -f BuildLinux.zip;\
	zip -r BuildLinux.zip Sdk Players Templates All\\ Plugins;\
	exit;\
	" |	ssh $(LINUX_HOST)
	scp -B $(LINUX_HOST):$(LINUX_PATH)/Build.Linux/BuildLinux.zip $(RELEASE)/BuildLinux.zip

fetchbundle.linux.pkg:
	scp -B $(LINUX_HOST):$(LINUX_PATH)/Gideros.tgz $(ROOT)/Gideros.tgz

bundle.linux.pkg:
	echo "\
	source /etc/profile;\
	cd $(LINUX_PATH);\
	make -f scripts/Makefile.gid bundle.installer;\
	exit;\
	" |	ssh $(LINUX_HOST)

clean.linux.pkg:
	echo "\
	source /etc/profile;\
	cd $(LINUX_PATH);\
	git pull;\
	make -f scripts/Makefile.gid clean;\
	exit;\
	" |	ssh $(LINUX_HOST)

#Syncing rules
push.remote.pkg:
	cp -r $(SDK) $(RELEASE);\
	cd $(RELEASE);\
	rm -f BuildWin.zip;\
	zip -r BuildWin.zip Sdk Players Templates "All Plugins" Addons Resources Documentation $(TXTFILES);\
	[ ! -z "$(MAC_HOST)"] && scp -B BuildWin.zip $(MAC_HOST):$(MAC_PATH)/Build.Mac/BuildWin.zip; \
	[ ! -z "$(LINUX_HOST)"] && scp -B BuildWin.zip $(LINUX_HOST):$(LINUX_PATH)/Build.Linux/BuildWin.zip; \
	[ ! -z "$(MAC_HOST)"] && [ -f BuildLinux.zip] && scp -B BuildWin.zip $(MAC_HOST):$(MAC_PATH)/Build.Mac/BuildLinux.zip; \
	[ ! -z "$(LINUX_HOST)"] && [ -f BuildMac.zip] && scp -B BuildMac.zip $(LINUX_HOST):$(LINUX_PATH)/Build.Linux/BuildMac.zip; \
	echo "done";

sync.remote.pkg: fetch.mac.pkg fetch.linux.pkg push.remote.pkg

#General build
clean.all.thrun : clean.subthr clean.mac.pkg.subthr clean.linux.pkg.subthr

clean.all.pkg:	
	$(MAKE) -j2 -f scripts/Makefile.gid clean.all.thrun
	
%.subthr:
	$(MAKE) -j1 -f scripts/Makefile.gid $*
	
winpush: all fetchdoc push.remote.pkg

macpull: build.mac.pkg fetch.mac.pkg

linuxpull: build.linux.pkg fetch.linux.pkg
	
build.all.thrun : winpush.subthr macpull.subthr linuxpull.thr

build.all.thr:
	$(MAKE) -j2 -f scripts/Makefile.gid build.all.thrun

bundle.all.thrun : bundle.installer.subthr bundle.mac.pkg.subthr bundle.linux.pkg.subthr 

bundle.all.thr:
	$(MAKE) -j2 -f scripts/Makefile.gid bundle.all.thrun
	
all.pkg: start.pkg build.all.thr bundle.all.thr fetchbundle.mac.pkg fetchbundle.linux.pkg
	echo -n "Finished on "; date

bundle.pkg: bundle.all.thr fetchbundle.mac.pkg fetchbundle.linux.pkg

start.pkg:
	echo -n "Starting on "; date

full: clean.all.pkg all.pkg

addons.pkg: $(addsuffix .addons.pkg,$(ADDONS))

%.addons.pkg:
	mkdir -p $(RELEASE)/Addons/$*
	-$(RELEASE)/Tools/gdrexport -platform gapp $(PWD)/$(ROOT)/studio_addons/$*/*.gproj $(PWD)/$(RELEASE)/Addons/$* | grep -v :
	cp $(ROOT)/studio_addons/$*/manifest.lua $(RELEASE)/Addons/$*/manifest.lua

%.macmake.pkg:
	echo "\
	source /etc/profile;\
	cd $(MAC_PATH);\
	make -f scripts/Makefile.gid $*;\
	exit;\
	" |	ssh $(MAC_HOST)

%.linuxmake.pkg:
	echo "\
	source /etc/profile;\
	cd $(LINUX_PATH);\
	make -f scripts/Makefile.gid $*;\
	exit;\
	" |	ssh $(LINUX_HOST)

%.fetchplugin.mac.pkg:
	echo "\
	source /etc/profile;\
	cp -r $(MAC_PATH)/$(SDK) $(MAC_PATH)/Build.Mac;\
	cd $(MAC_PATH)/Build.Mac;\
	rm -f _MacPlugin.zip;\
	zip -r _MacPlugin.zip All\\ Plugins/$(notdir $*);\
	exit;\
	" |	ssh $(MAC_HOST)
	scp -B $(MAC_HOST):$(MAC_PATH)/Build.Mac/_MacPlugin.zip $(RELEASE)/_MacPlugin.zip
	cd $(RELEASE); unzip -o _MacPlugin.zip
	rm -f $(RELEASE)/_MacPlugin.zip	
	
	
