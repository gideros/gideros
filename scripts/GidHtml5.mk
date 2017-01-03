.PHONY: html5.clean html5.libs html5.install html5

export EMSDK_PREFIX
export CRUNCHME

html5.clean:
	cd emscripten; $(MAKE) clean

html5.libs:
	cd emscripten; $(MAKE) -j 4

html5.install: html5.libs
	cp -r emscripten/Build/Html5 $(RELEASE)/Templates

html5: html5.install
