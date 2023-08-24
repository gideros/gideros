SUBMAKE=$(MAKE) -f scripts/Makefile.gid $(MAKEJOBS)
LINUX_CHAIN=$(LINUX_BIN)
LINUX_CC=$(LINUX_CHAIN)gcc
LINUX_CXX=$(LINUX_CHAIN)g++

linux.install: linux.libs.install linux.plugins.install


sdk.linuxlibs.dir:
	mkdir -p $(SDK)/lib/linux	

sdk.linuxlibs: sdk.headers sdk.linuxlibs.dir #COPY libs

LINUX_BUILDDIR=linux/build
LINUX_RELEASE=$(RELEASE)/Templates/linux/LinuxTemplate

#####LIBS
#INCLUDEPATHS_gvfs+=libgid/external/pthreads-w32-2-9-1-release/Pre-built.2/include
#LIBS_gvfs+= libgid/external/pthreads-w32-2-9-1-release/Pre-built.2/lib/x86/libpthreadGC2.a

LIBS_lua+=-lgvfs

INCLUDEPATHS_gid+= libgid/include/linux
DEFINES_gid+=$(LINUX_TGTVERDEF)
OBJFILES_gid+= $(addprefix libgid/src/linux/,gapplication-linux gaudio-linux ggeolocation-linux ghttp-linux \
				 ginput-linux gui-linux)
LIBS_gid+= -lopenal -lmp3 -lgvfs
#LIBS_gid+=libgid/external/pthreads-w32-2-9-1-release/Pre-built.2/lib/x86/libpthreadGC2.a \

LIBS_gideros+= -lgid -llua -lpystring

DEFINES_openal+=HAVE_ALSA  HAVE_UNISTD_H "ALC_API=" "AL_API="
OBJFILES_openal+=$(addprefix libgid/external/openal-soft/alc/backends/,alsa)
LIBS_openal+=-lasound

##PLAYER
INCLUDEPATHS_player+=libgid/include/linux
OBJFILES_player+= $(basename $(wildcard 2dsg/gfxbackends/gl2/*.cpp))
OBJFILES_player+= linux/main linux/applicationmanager libgid/src/linux/platform-linux
INCLUDEPATHS_player+=linux
INCLUDEPATHS_player+=2dsg/gfxbackends/gl2
LIBS_player = -lgvfs -lgid -llua -lpystring -lgideros \
	-lGL -lglfw -lpthread -lGLEW -ldl -lcurl


##RULES
%.linux.libs: $(OBJFILES_%) $(addprefix $(LINUX_BUILDDIR)/,$(addsuffix .o,$(OBJFILES_%))) $(LIBS_%)
	#BUILDING $*
	@mkdir -p $(addprefix $(LINUX_BUILDDIR)/,$(dir $(sort $(OBJFILES_$*))))
	@OBJFILES="$(OBJFILES_$*)" LIBS="$(LIBS_$*)" INCLUDEPATHS="$(INCLUDEPATHS_$*)" DEFINES="$(DEFINES_$*)" LIBNAME=$* $(MAKE) $(MAKEJOBS) -f $(firstword $(MAKEFILE_LIST)) linux.libs.build

linux.libs.build: CXXFLAGS = -g -O2 $(addprefix -D,$(DEFINES)) $(addprefix -I,$(INCLUDEPATHS))
linux.libs.build: $(addprefix $(LINUX_BUILDDIR)/,$(addsuffix .o,$(OBJFILES)))
	#LINK $(LIBNAME).so
	@$(LINUX_CXX) -g -o $(LINUX_BUILDDIR)/lib$(LIBNAME).so -shared $^ -Wl,-rpath,'$$ORIGIN' -Wl,-rpath,'$$ORIGIN/syslib' -L$(LINUX_BUILDDIR) $(LIBS) 
	@cp $(LINUX_BUILDDIR)/lib$(LIBNAME).so $(SDK)/lib/linux

%.linux.app: $(OBJFILES_%) $(addprefix $(LINUX_BUILDDIR)/,$(addsuffix .o,$(OBJFILES_%))) $(LIBS_%)
	#BUILDING $*
	@mkdir -p $(addprefix $(LINUX_BUILDDIR)/,$(dir $(sort $(OBJFILES_$*))))
	@OBJFILES="$(OBJFILES_$*)" LIBS="$(LIBS_$*)" INCLUDEPATHS="$(INCLUDEPATHS_$*)" DEFINES="$(DEFINES_$*)" APPNAME=$* $(MAKE) $(MAKEJOBS) -f $(firstword $(MAKEFILE_LIST)) linux.app.build

linux.app.build: CXXFLAGS = -g -O2 $(addprefix -D,$(DEFINES)) $(addprefix -I,$(INCLUDEPATHS))
linux.app.build: $(addprefix $(LINUX_BUILDDIR)/,$(addsuffix .o,$(OBJFILES)))
	#EXE $(APPNAME) $(LIBS)
	@$(LINUX_CXX) -g -o $(LINUX_BUILDDIR)/$(APPNAME) $^ -Wl,-rpath,'$$ORIGIN' -Wl,-rpath,'$$ORIGIN/syslib' -Wl,-rpath-link,$(LINUX_BUILDDIR) -L$(LINUX_BUILDDIR) $(LIBS)


$(LINUX_BUILDDIR)/%.o : %.cpp
	#C+ $(basename $(notdir $@))
	@$(LINUX_CXX) -g -std=gnu++17 -fPIC -DSTRICT_LINUX $(CXXFLAGS) -c $< -o $@

$(LINUX_BUILDDIR)/%.o : %.c
	#CC $(basename $(notdir $@))
	@$(LINUX_CC) -g -fPIC -DSTRICT_LINUX $(CXXFLAGS) -c $< -o $@

	
#-include libgvfs.dep

.PHONY : depend
depend:
	g++ $(INCLUDEPATHS) -MM ../libgvfs/*.cpp ../libgvfs/*.c > libgvfs.dep
			
linux.libs: sdk.linuxlibs.dir gvfs.linux.libs pystring.linux.libs lua.linux.libs mp3.linux.libs openal.linux.libs gid.linux.libs gideros.linux.libs

linux.app: player.linux.app

linux.libs.install: linux.libs
	mkdir -p $(LINUX_RELEASE)
	cp $(LINUX_BUILDDIR)/libgid.so $(LINUX_RELEASE)
	cp $(LINUX_BUILDDIR)/libgvfs.so $(LINUX_RELEASE)
	cp $(LINUX_BUILDDIR)/liblua.so $(LINUX_RELEASE)
	cp $(LINUX_BUILDDIR)/libgideros.so $(LINUX_RELEASE)
	cp $(LINUX_BUILDDIR)/libpystring.so $(LINUX_RELEASE)
	cp $(LINUX_BUILDDIR)/libmp3.so $(LINUX_RELEASE)
	cp $(LINUX_BUILDDIR)/libopenal.so $(LINUX_RELEASE)

%.linux.plugin:
	R=`pwd`; cd $(ROOT)/plugins/$*/source; if [ -d "linux" ]; then cd linux; ROOT=$$R/$(ROOT) RELEASE=$$R/$(RELEASE) $(MAKE) $(MAKEJOBS); fi

%.linux.plugin.clean:
	R=`pwd`; cd $(ROOT)/plugins/$*/source; if [ -d "linux" ]; then cd linux; ROOT=$$R/$(ROOT) RELEASE=$$R/$(RELEASE) $(MAKE) clean; fi

%.linux.plugin.install:
	@if [ -d "$(ROOT)/plugins/$*/source/linux" ]; then echo -n "Installing" $*; \
		mkdir -p $(RELEASE)/"All Plugins"/$*/bin/linux; \
		cp $(ROOT)/plugins/$*/source/linux/Build/*.so $(RELEASE)/"All Plugins"/$*/bin/linux; \
		strip $(RELEASE)/"All Plugins"/$*/bin/linux/*.so; \
		if [ -n "$(findstring $(notdir $*),$(PLUGINS_DEFAULT))" ]; then \
			echo " DEFAULT"; mkdir -p $(LINUX_BUILDDIR)/plugins; \
			cp $(ROOT)/plugins/$*/source/linux/Build/*.so $(LINUX_BUILDDIR)/plugins; \
		else echo ""; fi \
	fi

linux.install: linux.libs.install linux.app linux.plugins.install
	cp $(LINUX_BUILDDIR)/player $(LINUX_RELEASE)/LinuxTemplate
	cp linux/cacert.pem $(LINUX_RELEASE)
	cp linux/cacert.pem $(LINUX_BUILDDIR)
	cp linux/app.desktop $(LINUX_RELEASE)/LinuxTemplate.desktop
	mkdir -p $(LINUX_RELEASE)/syslib
	for i in `ldd $(LINUX_RELEASE)/LinuxTemplate | grep '=> /lib' | cut -d' ' -f 3 `; do cp $$i $(LINUX_RELEASE)/syslib/; done
	cd $(LINUX_RELEASE)/syslib; rm $(addprefix lib,$(addsuffix .*,c dl m pthread rt rtmp GL GLX GLdispatch X11 Xau Xcb Xdmcp stdc++))
	cd $(LINUX_RELEASE)/syslib; for i in *; do patchelf --set-rpath '$$ORIGIN' $$i; done
	strip $(addprefix $(LINUX_RELEASE)/,LinuxTemplate libgid.so libgvfs.so liblua.so libpystring.so libgideros.so libopenal.so libmp3.so)
	mkdir -p $(LINUX_RELEASE)/plugins

linux.clean: linux.plugins.clean
	rm -rf $(LINUX_BUILDDIR) 
		
linux.plugins: 
	$(SUBMAKE) $(addsuffix .linux.plugin,$(PLUGINS_LINUX))

linux.plugins.clean: 
	$(SUBMAKE) $(addsuffix .linux.plugin.clean,$(PLUGINS_LINUX))

linux.plugins.install: linux.plugins 
	$(SUBMAKE) $(addsuffix .linux.plugin.install,$(PLUGINS_LINUX))


		
