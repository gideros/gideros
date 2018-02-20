.PHONY: html5.clean html5.libs html5.install html5

HTML5_PLUGINS=$(PLUGINS_HTML5)

export EMSDK_PREFIX
export CRUNCHME
export HTML5_PLUGINS
export DEBUG

html5.setup:
	$(EMSDK_PREFIX) emsdk.bat activate latest
	
html5.clean:
	cd emscripten; $(MAKE) clean plugins.clean

html5.main:  
	cd emscripten; $(MAKE) -j 4 main

html5.libs: versioning 
	cd emscripten; $(MAKE) -j 4
	
html5.template: html5.libs
	cp -r emscripten/Build/Html5 $(RELEASE)/Templates
	for p in $(HTML5_PLUGINS); do mkdir -p $(RELEASE)/All\ Plugins/$$p/bin/Html5; cp plugins/$$p/source/emscripten/Build/*.js $(RELEASE)/All\ Plugins/$$p/bin/Html5; done	
	-for p in $(HTML5_PLUGINS); do cp plugins/$$p/source/emscripten/Build/*.wasm $(RELEASE)/All\ Plugins/$$p/bin/Html5; done	

html5.player: html5.libs
	mkdir -p $(RELEASE)/Players
	cp -r emscripten/Build/Html5/Html5 $(RELEASE)/Players
	for p in $(HTML5_PLUGINS); do cp plugins/$$p/source/emscripten/Build/*.js $(RELEASE)/Players/Html5; done
	-for p in $(HTML5_PLUGINS); do cp plugins/$$p/source/emscripten/Build/*.wasm $(RELEASE)/Players/Html5; done
	sed -e 's/\/\*GIDEROS_DYNLIB_PLUGIN\*\//"luasocket.js", "json.js", "bit.js", "lfs.js", "lsqlite3.js", /' emscripten/Build/Html5/Html5/index.html >$(RELEASE)/Players/Html5/index.html

html5.install: html5.template html5.player

html5: html5.install

html5.tools: html5.crunchme

CRUNCHME_SRCS=$(addprefix src/liblzg/lib/,checksum decode encode version)
CRUNCHME_SRCS+=$(addprefix src/zlib/,adler32 compress crc32 deflate inftrees trees zutil)
CRUNCHME_SRCS+=$(addprefix src/,crunchme png)

html5.crunchme: $(addprefix emscripten/crunchme-0.4/,$(addsuffix .co,$(CRUNCHME_SRCS)))
	$(CXX) -o $(ROOT)/ui/Tools/crunchme $^
	
%.co: %.cpp
	$(CXX) -c -o $@ -Iemscripten/crunchme-0.4/src/liblzg/include $< 

%.co: %.c
	$(CC) -c -o $@ $<

	