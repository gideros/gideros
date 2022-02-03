
#####LIBS
INCLUDEPATHS_gvfs = libgvfs libgvfs/private
OBJFILES_gvfs=$(addprefix libgvfs/,wsetup wbuf vfscanf vfprintf ungetc tmpfile stdio setvbuf rget refill \
    putc makebuf getc fwrite fwalk fvwrite ftell fseek fscanf freopen fread fputs fputc \
    fprintf fopen flockfile flags findfp fileops fgets fgetc fflush ferror feof fclose \
    extra clrerr gfile gpath)
DEFINES_gvfs=GIDEROS_LIBRARY

INCLUDEPATHS_pystring = libpystring
OBJFILES_pystring = $(addprefix libpystring/,pystring)
DEFINES_pystring=PYSTRING_LIBRARY

DEFINES_lua=LUA_BUILD_AS_DLL LUA_CORE
ifeq ($(LUA_ENGINE),luau)
INCLUDEPATHS_lua=luau/VM/src luau/VM/include luau/Compiler/include luau/Ast/include libgvfs
OBJFILES_lua=$(addprefix luau/VM/src/,lapi laux lbaselib lbitlib lbuiltins lcorolib ldblib ldebug ldo lfunc lgc lgcdebug linit lint64lib liolib lmathlib lmem lnumprint lobject loslib lperf lstate lstring lstrlib \
         ltable ltablib ltm ludata lutf8lib lvmexecute lvmload lvmutils) \
        $(addprefix luau/Compiler/src/,Builtins BytecodeBuilder ConstantFolding Compiler lcode PseudoCode TableShape ValueTracking) \
        $(addprefix luau/Ast/src/,Ast Confusables Lexer Location Parser StringUtils TimeTrace)
else
INCLUDEPATHS_lua=lua/src libgvfs
OBJFILES_lua=lua/etc/all_lua
endif


XMP_SRC=virtual period player read_event dataio lfo envelope \
		scan control filter effects mixer mix_all load_helpers load \
		hio smix memio
XMP_HDR=common effects envelope format lfo list mixer period player \
		virtual precomp_lut hio memio mdataio tempfile 
XMP_LOADERS=xm_load s3m_load it_load \
			common itsex sample
XMP_LOADERS_HDR=it loader mod s3m xm

INCLUDEPATHS_gid = libgid/external/glew-1.10.0/include \
	libgid/external/snappy-1.1.0 \
	libgid/external/libpng-1.6.2 \
	libgid/external/jpeg-9 \
	libgid/external/openal-soft-1.13/include/AL \
	libgid/external/zlib-1.2.8 \
	libgid/external/libxmp-4.3/include \
	libgid/external/libxmp-4.3/src \
	libgid/external/libxmp-4.3/src/loaders \
	libgid/external/mpg123-1.15.3/src/libmpg123 \
	libgid/external/mpg123-1.15.3/src \
	external/glu \
	libpystring \
	libgvfs \
	libgideros \
	2dsg 2dsg/gfxbackends \
	libgid/include
OBJFILES_gid = $(addprefix libgid/src/,\
				 gaudio-loader-mp3 gaudio-loader-wav gaudio-loader-xmp gaudio-sample-openal gaudio-stream-openal \
				 gaudio gevent gglobal gimage-jpg gimage-png gimage glog gtexture gtimer \
				 gvfs-native) \
				 $(addprefix libgid/external/snappy-1.1.0/,snappy snappy-c snappy-sinksource snappy-stubs-internal)
OBJFILES_gid += $(addprefix libgid/external/libxmp-4.3/src/,$(XMP_SRC))
OBJFILES_gid += $(addprefix libgid/external/libxmp-4.3/src/loaders/,$(XMP_LOADERS))
OBJFILES_gid += \
	libgid/external/libxmp-4.3/lite/src/format \
	libgid/external/libxmp-4.3/lite/src/loaders/mod_load

DEFINES_gid=GIDEROS_LIBRARY _REENTRANT LIBXMP_CORE_PLAYER

INCLUDEPATHS_gideros = libgideros $(LUA_INCLUDE) $(LUA_INCLUDE_CORE) libpystring libgid/include
OBJFILES_gideros = $(addprefix libgideros/,binderutil stringid eventdispatcher \
	event refptr eventvisitor pluginmanager luautil)
DEFINES_gideros=GIDEROS_LIBRARY

##PLAYER
FREETYPE_VER=2.7.1
INCLUDEPATHS_player = libgvfs libgideros $(LUA_INCLUDE) $(LUA_INCLUDE_CORE) libpystring libgid/include \
	libnetwork libpvrt luabinding \
	2dsg 2dsg/gfxbackends 2dsg/paths \
	libgid/external/freetype-$(FREETYPE_VER)/src \
	libgid/external/freetype-$(FREETYPE_VER)/include \
	libgid/external/mpg123-1.15.3/src \
	libgid/external/snappy-1.1.0 \
	libgid/external/libpng-1.6.2 \
	libgid/external/jpeg-9 \
	libgid/external/openal-soft-1.13/include/AL \
	libgid/external/mpg123-1.15.3/src/libmpg123 \
	libgid/external/zlib-1.2.8 \
	libgid/external/libxmp-4.3/include \
	external/liquidfun-1.0.0/liquidfun/Box2D \
	external/glu 

DEFINES_player=FT2_BUILD_LIBRARY DARWIN_NO_CARBON
	
OBJFILES_player= $(basename $(wildcard luabinding/*.cpp luabinding/*.c))
OBJFILES_player+= $(basename $(wildcard libpvrt/*.cpp))
OBJFILES_player+= $(basename $(wildcard libnetwork/*.cpp))
OBJFILES_player+= $(basename $(wildcard 2dsg/*.cpp 2dsg/*.c 2dsg/gfxbackends/*.cpp))
OBJFILES_player+= $(addprefix 2dsg/paths/,path ft-path svg-path)
OBJFILES_player+= $(basename $(wildcard $(addprefix external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/, \
					$(addsuffix /*.cpp,Common Collision Collision/Shapes Dynamics Dynamics/Contacts Dynamics/Joints Rope Particle))))
OBJFILES_player+= $(basename $(wildcard external/glu/libtess/*.c))
OBJFILES_player+= $(addprefix libgid/src/,aes md5 platformutil utf8 drawinfo gtimer)
OBJFILES_player+= $(addprefix external/minizip-1.1/source/,ioapi unzip)
OBJFILES_player+= $(addprefix libgid/external/freetype-$(FREETYPE_VER)/src/, \
	$(addprefix base/,ftbbox ftbitmap ftglyph ftlcdfil ftstroke ftbase ftsystem ftinit ftgasp) \
	raster/raster sfnt/sfnt smooth/smooth autofit/autofit truetype/truetype cff/cff gzip/ftgzip  \
	psnames/psnames pshinter/pshinter)
#2.4.12  base/ftxf86.c
