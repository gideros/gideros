SUBMAKE=$(MAKE) -f scripts/Makefile.gid $(MAKEJOBS)

win32.install: win32.libs.install win32.plugins.install


sdk.win32libs.dir:
	mkdir -p $(SDK)/lib/win32	

sdk.win32libs: sdk.headers sdk.win32libs.dir #COPY libs

WIN32_BUILDDIR=win32_example/build
WIN32_RELEASE=$(RELEASE)/Templates/win32/WindowsDesktopTemplate

#####LIBS
#INCLUDEPATHS_gvfs+=libgid/external/pthreads-w32-2-9-1-release/Pre-built.2/include
#LIBS_gvfs+= libgid/external/pthreads-w32-2-9-1-release/Pre-built.2/lib/x86/libpthreadGC2.a

LIBS_lua+=$(WIN32_BUILDDIR)/gvfs.dll

INCLUDEPATHS_gid+=libgid/include/win32 libgid/external/curl-7.40.0-devel-mingw32/include
OBJFILES_gid+= $(addprefix libgid/src/win32/,gapplication-win32 gaudio-win32 ggeolocation-win32 ghttp-win32 \
				 ginput-win32 gui-win32)
LIBS_gid+= \
 libgid/external/jpeg-9/build/mingw48_32/libjpeg.a \
 libgid/external/libpng-1.6.2/build/mingw481_win32/libpng.a \
 libgid/external/mpg123-1.15.3/lib/mingw48_32/libmpg123.a \
 libgid/external/openal-soft-1.13/build/mingw48_32/libOpenAL32.dll.a \
 libgid/external/zlib-1.2.8/build/mingw48_32/libzlibx.a \
 libgid/external/curl-7.40.0-devel-mingw32/lib/libcurldll.a \
 -L"libgid/external/glew-1.10.0/lib/mingw48_32" -lglew32 -lopengl32 \
 $(WIN32_BUILDDIR)/gvfs.dll
#LIBS_gid+=libgid/external/pthreads-w32-2-9-1-release/Pre-built.2/lib/x86/libpthreadGC2.a \

LIBS_gideros+= $(addprefix $(WIN32_BUILDDIR)/,gid.dll lua.dll pystring.dll)

##PLAYER
INCLUDEPATHS_player+=libgid/include/win32
OBJFILES_player+= $(basename $(wildcard 2dsg/gfxbackends/gl2/*.cpp))
#OBJFILES_player+= $(basename $(wildcard 2dsg/gfxbackends/dx11/*.cpp)) $(basename $(wildcard 2dsg/gfxbackends/dx11/*.c))
OBJFILES_player+= win32_example/win32 win32_example/applicationmanager libgid/src/win32/platform-win32
INCLUDEPATHS_player+=win32_example
INCLUDEPATHS_player+=2dsg/gfxbackends/gl2
INCLUDEPATHS_player+=2dsg/gfxbackends/dx11
DEFINES_player+=WIN32=1
LIBS_player = $(addprefix $(WIN32_BUILDDIR)/,gvfs.dll gid.dll lua.dll pystring.dll gideros.dll) \
	libgid/external/zlib-1.2.8/build/mingw48_32/libzlibx.a \
	-L"libgid/external/glew-1.10.0/lib/mingw48_32" -lglew32 \
	-lopengl32 -luser32 -lgdi32 -lcomdlg32 -lcomctl32 -lws2_32 -liphlpapi -lwinmm

##RULES
%.win32.libs: $(OBJFILES_%) $(addprefix $(WIN32_BUILDDIR)/,$(addsuffix .o,$(OBJFILES_%))) $(LIBS_%)
	#BUILDING $*
	@mkdir -p $(addprefix $(WIN32_BUILDDIR)/,$(dir $(sort $(OBJFILES_$*))))
	@OBJFILES="$(OBJFILES_$*)" LIBS="$(LIBS_$*)" INCLUDEPATHS="$(INCLUDEPATHS_$*)" DEFINES="$(DEFINES_$*)" LIBNAME=$* $(MAKE) $(MAKEJOBS) -f $(firstword $(MAKEFILE_LIST)) win32.libs.build

win32.libs.build: CXXFLAGS = -g -O2 -fno-keep-inline-dllexport $(addprefix -D,$(DEFINES)) $(addprefix -I,$(INCLUDEPATHS))
win32.libs.build: $(addprefix $(WIN32_BUILDDIR)/,$(addsuffix .o,$(OBJFILES)))
	#LINK $(LIBNAME).dll
	@$(CXX) -g -o $(WIN32_BUILDDIR)/$(LIBNAME).dll -shared $^ $(LIBS)
	cp $(WIN32_BUILDDIR)/$(LIBNAME).dll $(SDK)/lib/win32

%.win32.app: $(OBJFILES_%) $(addprefix $(WIN32_BUILDDIR)/,$(addsuffix .o,$(OBJFILES_%))) $(LIBS_%)
	#BUILDING $*
	@mkdir -p $(addprefix $(WIN32_BUILDDIR)/,$(dir $(sort $(OBJFILES_$*))))
	@OBJFILES="$(OBJFILES_$*)" LIBS="$(LIBS_$*) -mwindows" INCLUDEPATHS="$(INCLUDEPATHS_$*)" DEFINES="$(DEFINES_$*)" APPNAME=$* $(MAKE) $(MAKEJOBS) -f $(firstword $(MAKEFILE_LIST)) win32.app.build
	@OBJFILES="$(OBJFILES_$*)" LIBS="$(LIBS_$*) -mconsole" INCLUDEPATHS="$(INCLUDEPATHS_$*)" DEFINES="$(DEFINES_$*)" APPNAME=$*-console $(MAKE) $(MAKEJOBS) -f $(firstword $(MAKEFILE_LIST)) win32.app.build

win32.app.build: CXXFLAGS = -g -O2 -fno-keep-inline-dllexport $(addprefix -D,$(DEFINES)) $(addprefix -I,$(INCLUDEPATHS))
win32.app.build: $(addprefix $(WIN32_BUILDDIR)/,$(addsuffix .o,$(OBJFILES)))
	#EXE $(APPNAME) $(LIBS)
	@$(CXX) -g -o $(WIN32_BUILDDIR)/$(APPNAME) $^ $(LIBS)


$(WIN32_BUILDDIR)/%.o : %.cpp
	#C+ $(basename $(notdir $@))
	@$(CXX) -g -std=c++17 $(CXXFLAGS) -c $< -o $@

$(WIN32_BUILDDIR)/%.o : %.c
	#CC $(basename $(notdir $@))
	@$(CC) -g $(CXXFLAGS) -c $< -o $@

	
#-include libgvfs.dep

.PHONY : depend
depend:
	g++ $(INCLUDEPATHS) -MM ../libgvfs/*.cpp ../libgvfs/*.c > libgvfs.dep
			
win32.libs: sdk.win32libs.dir gvfs.win32.libs pystring.win32.libs lua.win32.libs gid.win32.libs gideros.win32.libs

win32.app: versioning player.win32.app

win32.libs.install: win32.libs
	mkdir -p $(WIN32_RELEASE)
	cp $(WIN32_BUILDDIR)/gid.dll $(WIN32_RELEASE)
	cp $(WIN32_BUILDDIR)/gvfs.dll $(WIN32_RELEASE)
	cp $(WIN32_BUILDDIR)/lua.dll $(WIN32_RELEASE)
	cp $(WIN32_BUILDDIR)/gideros.dll $(WIN32_RELEASE)
	cp $(WIN32_BUILDDIR)/pystring.dll $(WIN32_RELEASE)

%.win32.plugin:
	R=$(PWD); cd $(ROOT)/plugins/$*/source; if [ -d "win32" ]; then cd win32; ROOT=$$R/$(ROOT) RELEASE=$$R/$(RELEASE) $(MINGWMAKE) $(MAKEJOBS); fi

%.win32.plugin.clean:
	R=$(PWD); cd $(ROOT)/plugins/$*/source; if [ -d "win32" ]; then cd win32; ROOT=$$R/$(ROOT) RELEASE=$$R/$(RELEASE) $(MINGWMAKE) clean; fi

%.win32.plugin.install:
	@if [ -d "$(ROOT)/plugins/$*/source/win32" ]; then echo -n "Installing" $*; \
		mkdir -p $(RELEASE)/"All Plugins"/$*/bin/win32; \
		cp $(ROOT)/plugins/$*/source/win32/Build/*.dll $(RELEASE)/"All Plugins"/$*/bin/win32; \
		strip $(RELEASE)/"All Plugins"/$*/bin/win32/*.dll; \
		if [ -n "$(findstring $(notdir $*),$(PLUGINS_DEFAULT))" ]; then \
			echo " DEFAULT"; mkdir -p $(WIN32_RELEASE)/plugins; \
			cp $(ROOT)/plugins/$*/source/win32/Build/*.dll $(WIN32_RELEASE)/plugins; \
	else echo ""; fi; fi

win32.install: win32.libs.install win32.plugins.install win32.app
	cp $(WIN32_BUILDDIR)/player.exe $(WIN32_RELEASE)/WindowsDesktopTemplate.exe
	cp $(WIN32_BUILDDIR)/player-console.exe $(WIN32_RELEASE)/WindowsDesktopTemplate-Console.exe
	cp $(ROOT)/libgid/external/glew-1.10.0/lib/mingw48_32/glew32.dll $(WIN32_RELEASE)
	cp $(ROOT)/libgid/external/openal-soft-1.13/build/mingw48_32/OpenAL32.dll $(WIN32_RELEASE)
	cp $(ROOT)/libgid/external/curl-7.40.0-devel-mingw32/bin/*.dll $(WIN32_RELEASE)
	for f in libgcc_s_dw2-1 libstdc++-6 libwinpthread-1; do cp $(QT)/bin/$$f.dll $(WIN32_RELEASE); done
	strip $(addprefix $(WIN32_RELEASE)/,WindowsDesktopTemplate.exe WindowsDesktopTemplate-Console.exe gid.dll gvfs.dll lua.dll pystring.dll gideros.dll)

win32.clean: win32.plugins.clean
	rm -rf $(WIN32_BUILDDIR) 
		
win32.plugins: 
	$(SUBMAKE) $(addsuffix .win32.plugin,$(PLUGINS_WIN32))

win32.plugins.clean: 
	$(SUBMAKE) $(addsuffix .win32.plugin.clean,$(PLUGINS_WIN32))

win32.plugins.install: win32.plugins 
	$(SUBMAKE) $(addsuffix .win32.plugin.install,$(PLUGINS_WIN32))


		