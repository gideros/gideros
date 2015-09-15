
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

INCLUDEPATHS_lua=lua/src libgvfs
OBJFILES_lua=lua/etc/all_lua
DEFINES_lua=LUA_BUILD_AS_DLL


INCLUDEPATHS_gid = libgid/external/glew-1.10.0/include \
	libgid/external/freetype-2.4.12/src \
	libgid/external/freetype-2.4.12/include \
	libgid/external/mpg123-1.15.3/src \
	libgid/external/snappy-1.1.0 \
	libgid/external/libpng-1.6.2 \
	libgid/external/jpeg-9 \
	libgid/external/openal-soft-1.13/include/AL \
	libgid/external/mpg123-1.15.3/src/libmpg123 \
	libgid/external/zlib-1.2.8 \
	external/glu \
	libpystring \
	libgvfs \
	libgideros \
	2dsg 2dsg/gfxbackends \
	libgid/include
OBJFILES_gid = $(addprefix libgid/src/,\
				 gaudio-loader-mp3 gaudio-loader-wav gaudio-sample-openal gaudio-stream-openal \
				 gaudio gevent gglobal gimage-jpg gimage-png gimage glog gtexture gtimer \
				 gvfs-native) \
				 $(addprefix libgid/external/snappy-1.1.0/,snappy snappy-c snappy-sinksource snappy-stubs-internal)
DEFINES_gid=GIDEROS_LIBRARY

INCLUDEPATHS_gideros = libgideros lua/src libpystring libgid/include
OBJFILES_gideros = $(addprefix libgideros/,binderutil stringid eventdispatcher \
	event refptr eventvisitor pluginmanager luautil)
DEFINES_gideros=GIDEROS_LIBRARY

##PLAYER
INCLUDEPATHS_player = libgvfs libgideros lua/src libpystring libgid/include \
	libnetwork libpvrt luabinding \
	2dsg 2dsg/gfxbackends \
	libgid/external/glew-1.10.0/include \
	libgid/external/freetype-2.4.12/src \
	libgid/external/freetype-2.4.12/include \
	libgid/external/mpg123-1.15.3/src \
	libgid/external/snappy-1.1.0 \
	libgid/external/libpng-1.6.2 \
	libgid/external/jpeg-9 \
	libgid/external/openal-soft-1.13/include/AL \
	libgid/external/mpg123-1.15.3/src/libmpg123 \
	libgid/external/zlib-1.2.8 \
	external/liquidfun-1.0.0/liquidfun/Box2D \
	external/glu 
	
OBJFILES_player= $(basename $(wildcard luabinding/*.cpp luabinding/*.c))
OBJFILES_player+= $(basename $(wildcard libpvrt/*.cpp))
OBJFILES_player+= $(basename $(wildcard libnetwork/*.cpp))
OBJFILES_player+= $(basename $(wildcard 2dsg/*.cpp 2dsg/*.c 2dsg/gfxbackends/*.cpp))
OBJFILES_player+= $(basename $(wildcard $(addprefix external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/, \
					$(addsuffix /*.cpp,Common Collision Collision/Shapes Dynamics Dynamics/Contacts Dynamics/Joints Rope Particle))))
OBJFILES_player+= $(basename $(wildcard external/glu/libtess/*.c))
OBJFILES_player+= $(addprefix libgid/src/,md5 platformutil utf8 drawinfo gtimer)
OBJFILES_player+= $(addprefix external/minizip-1.1/source/,ioapi unzip)
