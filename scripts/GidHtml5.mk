.PHONY: html5.clean html5.libs html5.install html5

HTML5_PLUGINS=bitop luasocket json ads Facebook

export EMSDK_PREFIX
export CRUNCHME
export HTML5_PLUGINS

html5.clean:
	cd emscripten; $(MAKE) clean plugins.clean

html5.libs:
	cd emscripten; $(MAKE) -j 4

html5.install: html5.libs
	cp -r emscripten/Build/Html5 $(RELEASE)/Templates
	for p in $(HTML5_PLUGINS); do mkdir -p $(RELEASE)/All\ Plugins/$$p/bin/Html5; cp plugins/$$p/source/emscripten/Build/*.js $(RELEASE)/All\ Plugins/$$p/bin/Html5; done	

html5: html5.install
