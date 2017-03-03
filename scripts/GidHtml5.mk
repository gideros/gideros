.PHONY: html5.clean html5.libs html5.install html5

HTML5_PLUGINS=bitop luasocket json ads Facebook lsqlite3 lfs

export EMSDK_PREFIX
export CRUNCHME
export HTML5_PLUGINS
export DEBUG

html5.clean:
	cd emscripten; $(MAKE) clean plugins.clean

html5.libs:
	cd emscripten; $(MAKE) -j 4

html5.install: html5.libs
	cp -r emscripten/Build/Html5 $(RELEASE)/Templates
	for p in $(HTML5_PLUGINS); do mkdir -p $(RELEASE)/All\ Plugins/$$p/bin/Html5; cp plugins/$$p/source/emscripten/Build/*.js $(RELEASE)/All\ Plugins/$$p/bin/Html5; done	

html5: html5.install

CRUNCHME_SRCS=$(addprefix src/liblzg/lib/,checksum decode encode version)
CRUNCHME_SRCS+=$(addprefix src/zlib/,adler32 compress crc32 deflate inftrees trees zutil)
CRUNCHME_SRCS+=$(addprefix src/,crunchme png)

crunchme.html5: $(addprefix emscripten/crunchme-0.4/,$(addsuffix .co,$(CRUNCHME_SRCS)))
	$(CXX)  -o emscripten/crunchme-0.4/bin/crunchme-qt-win $^
	
%.co: %.cpp
	$(CXX) -c $< -Iemscripten/crunchme-0.4/src/liblzg/include -o $@

%.co: %.c
	$(CC) -c $< -o $@

	