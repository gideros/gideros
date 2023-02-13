TARGET=microphone
SRCS=gmicrophone-openal gsoundencoder-wav gmicrophonebinder
CFLGS+=-I$(ROOT)/libgid/external/openal-soft-1.13/include/AL
CFLGS+=-I$(ROOT)/libgid/external/pthreads-w32-2-9-1-release/Pre-built.2/include
CFLGS+=-I$(ROOT)/Sdk/include/gideros -DUSE_PTHREADS
LFLGS+=-L$(ROOT)/libgid/external/pthreads-w32-2-9-1-release/Pre-built.2/lib/x86/ -lpthreadGC2
LFLGS+=-L$(ROOT)/libgid/external/openal-soft-1.13/build/mingw48_32/ -lOpenAL32.dll
include $(ROOT)/scripts/GidWin32Plugin.mk
