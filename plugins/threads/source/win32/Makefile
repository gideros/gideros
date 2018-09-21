TARGET=threads

vpath %.cpp ../../../../luabinding

INCS += ../../../luabinding

CXXINCS += ../../../../lua/src

SRCS = threads_entry luathread StateToState binder
LFLGS = -lws2_32 -llua -lgideros

include $(ROOT)/scripts/GidWin32Plugin.mk
    
