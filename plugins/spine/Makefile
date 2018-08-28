SPINE_REPO=spine-runtimes

check-repo:
	if [ ! -d $(SPINE_REPO) ]; then git clone https://github.com/gideros/$(SPINE_REPO); fi
	cd $(SPINE_REPO); git pull

	