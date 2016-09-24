SRCS=$(addprefix 2dsg/,$(notdir $(basename $(wildcard ../2dsg/*.cpp))))
SRCS+=$(addprefix 2dsg/gfxbackends/,$(notdir $(basename $(wildcard ../2dsg/gfxbackends/*.cpp))))
SRCS+=$(addprefix 2dsg/gfxbackends/gl2/,$(notdir $(basename $(wildcard ../2dsg/gfxbackends/gl2/*.cpp))))
SRCS+=$(addprefix 2dsg/paths/,$(notdir $(basename $(wildcard ../2dsg/paths/*.cpp)) svg-path ft-path))
SRCS+=$(addprefix 2dsg/paths/,$(notdir $(basename $(wildcard ../2dsg/paths/*.cpp))))

LIQUIDFUN=external/liquidfun-1.0.0/liquidfun/Box2D/Box2D
SRCS+=$(addprefix $(LIQUIDFUN)/Common/,$(notdir $(basename $(wildcard ../$(LIQUIDFUN)/Common/*.cpp))))
SRCS+=$(addprefix $(LIQUIDFUN)/Collision/,$(notdir $(basename $(wildcard ../$(LIQUIDFUN)/Collision/*.cpp))))
SRCS+=$(addprefix $(LIQUIDFUN)/Collision/Shapes/,$(notdir $(basename $(wildcard ../$(LIQUIDFUN)/Collision/Shapes/*.cpp))))
SRCS+=$(addprefix $(LIQUIDFUN)/Dynamics/,$(notdir $(basename $(wildcard ../$(LIQUIDFUN)/Dynamics/*.cpp))))
SRCS+=$(addprefix $(LIQUIDFUN)/Dynamics/Contacts/,$(notdir $(basename $(wildcard ../$(LIQUIDFUN)/Dynamics/Contacts/*.cpp))))
SRCS+=$(addprefix $(LIQUIDFUN)/Dynamics/Joints/,$(notdir $(basename $(wildcard ../$(LIQUIDFUN)/Dynamics/Joints/*.cpp))))
SRCS+=$(addprefix $(LIQUIDFUN)/Rope/,$(notdir $(basename $(wildcard ../$(LIQUIDFUN)/Rope/*.cpp))))
SRCS+=$(addprefix $(LIQUIDFUN)/Particle/,$(notdir $(basename $(wildcard ../$(LIQUIDFUN)/Particle/*.cpp))))

SRCS+=$(addprefix luabinding/,$(notdir $(basename $(wildcard ../luabinding/*.cpp)))) luabinding/tlsf
SRCS+=$(addprefix libnetwork/,$(notdir $(basename $(wildcard ../libnetwork/*.cpp))))
SRCS+=$(addprefix libpvrt/,$(notdir $(basename $(wildcard ../libpvrt/*.cpp))))
SRCS+=$(addprefix libgid/src/,aes md5 platformutil utf8 drawinfo gtimer \
	gimage gimage-png gimage-jpg gtexture gevent glog gglobal gaudio \
	gaudio-loader-wav gaudio-loader-mp3 \
	gvfs-native)
SRCS+=$(addprefix libgideros/,event eventdispatcher eventvisitor refptr stringid luautil pluginmanager binderutil)
SRCS+=$(addprefix external/glu/libtess/,$(notdir $(basename $(wildcard ../external/glu/libtess/*.c))))
SRCS+=$(addprefix external/minizip-1.1/source/,ioapi unzip)


#    ../libgid/src/qt/platform-qt.cpp \

SRCS+=$(addprefix libgvfs/,$(notdir $(basename $(wildcard ../libgvfs/*.c))))
SRCS+=$(addprefix libgvfs/,$(notdir $(basename $(wildcard ../libgvfs/*.cpp))))
SRCS+=lua/etc/all_lua
SRCS+=libpystring/pystring

#SRCS+=$(addprefix libgid/external/freetype-2.6.1/src/,base/ftbbox base/ftbitmap base/ftglyph base/ftlcdfil \
#		base/ftstroke base/ftxf86 base/ftbase base/ftsystem base/ftinit base/ftgasp \
#		raster/raster sfnt/sfnt smooth/smooth autofit/autofit truetype/truetype \
#		cff/cff psnames/psnames pshinter/pshinter)

SRCS+=$(addprefix libgid/external/zlib-1.2.8/,$(notdir $(basename $(wildcard ../libgid/external/zlib-1.2.8/*.c))))
PNGFILES=png pngerror pngget pngmem pngpread pngread pngrio pngrtran pngrutil pngset pngtrans \
	pngwio pngwrite pngwtran pngwutil 
SRCS+=$(addprefix libgid/external/libpng-1.6.2/,$(PNGFILES))
JPEGFILES=jaricom jdapimin jdapistd jdarith jdatadst jdatasrc jdcoefct jdcolor jddctmgr jdhuff jdinput \
	jdmainct jdmarker jdmaster jdmerge jdpostct jdsample jdtrans jerror jfdctflt jfdctfst jfdctint \
	jidctflt jidctfst jidctint jquant1 jquant2 jutils jmemmgr jmemnobs jcomapi \
	jcapimin jcapistd jcarith jccoefct jccolor jcdctmgr jchuff jcinit jcmainct \
	jcmaster jcparam jcprepct jcsample jctrans
SRCS+=$(addprefix libgid/external/jpeg-9/,$(JPEGFILES))
SRCS+=$(addprefix libgid/external/snappy-1.1.0/,snappy snappy-c snappy-sinksource snappy-stubs-internal)
MP3FILES=compat dct64 dither equalizer feature format frame icy icy2utf8 id3 index layer1 layer2 layer3 \
	libmpg123 ntom optimize parse readers stringbuf synth synth_8bit synth_real synth_s32 tabinit
SRCS+=$(addprefix libgid/external/mpg123-1.15.3/src/libmpg123/,$(MP3FILES))
SRCS+=$(addprefix emscripten/,applicationmanager gapplication-js \
	ginput-js gaudio-js gaudio-stream-openal-js gaudio-sample-openal-js platform-js ghttp-js \
	gui-js ggeolocation-js main jsnative)   


INCS=2dsg libgideros libgid/include libgvfs libpystring
INCS+=2dsg/gfxbackends 2dsg/gfxbackends/gl2 2dsg/paths
INCS+=luabinding libnetwork
INCS+=$(addprefix libgid/external/freetype-2.6.1/,include src)
INCS+=$(LIQUIDFUN)/.. external/glu
INCS+=libgid/external/zlib-1.2.8 libgid/external/libpng-1.6.2 \
	$(addprefix libgid/external/openal-soft-1.13/,include/AL OpenAL32/Include) \
	libgid/external/snappy-1.1.0 libgid/external/jpeg-9 \
	libgid/external/mpg123-1.15.3/src \
	libgid/external/mpg123-1.15.3/src/libmpg123
INCS+=lua/src
INCS+=libgvfs/private
INCS+=emscripten

LIBS+=prebuilt/libfreetype.so

EMCC=emcc
BUILD=Build

OPTS+=-O2 --bind

EOPTS+=MAIN_MODULE=1 
EOPTS+=FULL_ES2=1 
EOPTS+=DISABLE_EXCEPTION_CATCHING=0
#EOPTS+=ALLOW_MEMORY_GROWTH=1

ifneq ($(DEBUG),)
OPTS+=-g4
EOPTS+=ASSERTIONS=2
endif

OBJS=$(addprefix $(BUILD)/,$(addsuffix .em.o,$(SRCS)))
CINCS=$(addprefix -I../,$(INCS))
CFLGS=-DFT2_BUILD_LIBRARY -DDARWIN_NO_CARBON -DHAVE_UNISTD_H \
	-DOPT_GENERIC -DREAL_IS_FLOAT \
	$(OPTS)
TEMPLATE=$(BUILD)/Html5/Html5


PLUGINS=bitop luasocket json

template: main plugins
	cd $(BUILD); rm -f Html5.zip; zip -r Html5.zip Html5/; 

plugins:
	R=$(PWD); for p in $(PLUGINS); do cd ../plugins/$$p/source/emscripten; $(MAKE); cp Build/*.js* $$R/$(TEMPLATE)/; cd -; done

plugins.clean:
	R=$(PWD); for p in $(PLUGINS); do cd ../plugins/$$p/source/emscripten; $(MAKE) clean; cd -; done
	
libs:

$(TEMPLATE)/gideros.js: libs $(OBJS)
	emcc $(OBJS) $(LIBS) $(addprefix -s ,$(EOPTS)) $(OPTS) -o $(TEMPLATE)/gideros.html --pre-js gidjs.js --pre-js gui.js --shell-file gidshell.html

main: path libs $(TEMPLATE)/gideros.js
	cp gideros.png $(TEMPLATE)
	cp gideros.css $(TEMPLATE)
	cp pace.* $(TEMPLATE)

path:
	mkdir -p $(sort $(dir $(OBJS)))
	mkdir -p $(TEMPLATE)

clean:
	rm -rf $(BUILD)

$(BUILD)/%.em.o: ../%.cpp
	$(EMCC) $(CINCS) $(CFLGS) -c $< --default-obj-ext .em.o -o $@

$(BUILD)/%.em.o: ../%.c
	$(EMCC) $(CINCS) $(CFLGS) -c $< --default-obj-ext .em.o -o $@

