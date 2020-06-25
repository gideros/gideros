TARGET=threads

vpath %.cpp ../../../../luabinding
vpath %.c ../../../luasocket/source
vpath %.cpp ../../../lfs/source
vpath %.c ../../../json/source

INCS += ../../../luabinding ../../../libgid/external/zlib-1.2.8

CXXINCS += -I../../../../lua/src -I"../../../../libgid/external/zlib-1.2.8"

CFLGS += -DWIN32 -DLUASOCKET_INET_PTON -DLUA_NOCOMPAT_MODULE -DUNICODE -D_UNICODE

SRCS = threads_entry luathread threadtimedluahook StateToState binder auxiliar except buffer
SRCS += zlibbinder
SRCS += inet io luasocket options select tcp timeout udp mime wsocket 
SRCS += lfs 
SRCS += strbuf fpconv lua_cjson  
LFLGS += -lws2_32 -L"../../../../libgid/external/zlib-1.2.8/build/mingw48_32" -lzlibx

include $(ROOT)/scripts/GidWin32Plugin.mk
    
