TARGET=microphone
SRCS=gmicrophone-openal gsoundencoder-wav gmicrophonebinder
CFLGS+=-I$(ROOT)/libgid/external/openal-soft/include/AL
CFLGS+=-I$(ROOT)/Sdk/include/gideros -DUSE_PTHREADS
LFLGS+=-L$(ROOT)/Sdk/lib/linux -lopenal
include $(ROOT)/scripts/GidLinuxPlugin.mk
