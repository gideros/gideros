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
	 
build.mac.pkg:
	echo "\
	cd $(MAC_PATH);\
	make -f scripts/Makefile.gid;\
	exit;\
	" |	ssh $(MAC_HOST)
	
fetch.mac.pkg:
	echo "\
	cp -r $(MAC_PATH)/$(SDK) $(MAC_PATH)/Build.Mac;\
	cd $(MAC_PATH)/Build.Mac;\
	rm -f BuildMac.zip;\
	zip -r BuildMac.zip Sdk Players Templates All\\ Plugins;\
	exit;\
	" |	ssh $(MAC_HOST)
	scp -B $(MAC_HOST):$(MAC_PATH)/Build.Mac/BuildMac.zip $(RELEASE)/BuildMac.zip

fetchbundle.mac.pkg:
	scp -B $(MAC_HOST):$(MAC_PATH)/Gideros.pkg $(ROOT)/Gideros.pkg

push.mac.pkg:
	cp -r $(SDK) $(RELEASE);\
	cd $(RELEASE);\
	rm -f BuildWin.zip;\
	zip -r BuildWin.zip Sdk Players Templates "All Plugins" Addons Resources Documentation $(TXTFILES);\
	scp -B BuildWin.zip $(MAC_HOST):$(MAC_PATH)/Build.Mac/BuildWin.zip 

bundle.mac.pkg:
	echo "\
	cd $(MAC_PATH);\
	make -f scripts/Makefile.gid bundle.installer;\
	exit;\
	" |	ssh $(MAC_HOST)

sync.mac.pkg: fetch.mac.pkg push.mac.pkg

clean.mac.pkg:
	echo "\
	cd $(MAC_PATH);\
	git pull;\
	make -f scripts/Makefile.gid clean;\
	exit;\
	" |	ssh $(MAC_HOST)

clean.all.thrun : clean.subthr clean.mac.pkg.subthr

clean.all.pkg:	
	$(MAKE) -j2 -f scripts/Makefile.gid clean.all.thrun
	
%.subthr:
	$(MAKE) -j1 -f scripts/Makefile.gid $*
	
winpush: all fetchdoc push.mac.pkg

macpull: build.mac.pkg fetch.mac.pkg
	
build.all.thrun : winpush.subthr macpull.subthr 

build.all.thr:
	$(MAKE) -j2 -f scripts/Makefile.gid build.all.thrun

bundle.all.thrun : bundle.installer.subthr bundle.mac.pkg.subthr 

bundle.all.thr:
	$(MAKE) -j2 -f scripts/Makefile.gid bundle.all.thrun
	
all.pkg: start.pkg build.all.thr bundle.all.thr fetchbundle.mac.pkg
	echo -n "Finished on "; date

bundle.pkg: bundle.all.thr fetchbundle.mac.pkg

start.pkg:
	echo -n "Starting on "; date

full: clean.all.pkg all.pkg

addons.pkg: $(addsuffix .addons.pkg,$(ADDONS))

%.addons.pkg:
	mkdir -p $(RELEASE)/Addons/$*
	-$(RELEASE)/Tools/gdrexport -platform gapp $(PWD)/$(ROOT)/studio_addons/$*/*.gproj $(PWD)/$(RELEASE)/Addons/$* | grep -v :
	cp $(ROOT)/studio_addons/$*/manifest.lua $(RELEASE)/Addons/$*/manifest.lua
	
