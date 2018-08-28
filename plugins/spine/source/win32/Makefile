TARGET=spine

vpath %.c ../../spine-runtimes/spine-c/spine-c/src/spine
vpath %.c ../../../../luabinding
vpath %.cpp ../../../../luabinding

INCS += ../../../Sdk/include
INCS += ../../../Sdk/include/gideros
INCS += ../../../2dsg
INCS += ../../../2dsg/gfxbackends
INCS += ../../../libgideros
INCS += ../../../libgid/include
INCS += ../../../luabinding
INCS += ../../../lua/src
INCS += ../spine-runtimes/spine-c/spine-c/include
SRCS += $(addprefix Common/,spinebinder)
SRCS+=  binder 
SRCS+= $(notdir $(patsubst ../%.c,%,$(wildcard ../../spine-runtimes/spine-c/spine-c/src/spine/*.c)))

include $(ROOT)/scripts/GidWin32Plugin.mk
