VPATH = \
..\libgideros

INCLUDEPATHS = \
-I..\luabinding \
-I..\libnetwork \
-I..\libgid\external\glew-1.10.0\include \
-I..\libgid\external\freetype-2.4.12\src \
-I..\libgid\external\freetype-2.4.12\include \
-I..\libgid\external\mpg123-1.15.3\src \
-I..\libgid\external\pthreads-w32-2-9-1-release\Pre-built.2\include \
-I..\external\liquidfun-1.0.0\liquidfun\Box2D \
-I..\libgid\include\win32 \
-I..\libgid\external\snappy-1.1.0 \
-I..\libgid\external\libpng-1.6.2 \
-I..\libgid\external\jpeg-9 \
-I..\libgid\external\openal-soft-1.13\include\AL \
-I..\libgid\external\mpg123-1.15.3\src\libmpg123 \
-I..\lua\src \
-I..\libgid\external\zlib-1.2.8 \
-I..\external\glu \
-I..\libpystring \
-I..\libgvfs \
-I..\libgideros \
-I..\2dsg \
-I..\2dsg\gfxbackends \
-I..\2dsg\gfxbackends\gl2 \
-I..\libgid\include

objfiles = binderutil.o stringid.o eventdispatcher.o \
event.o refptr.o eventvisitor.o pluginmanager.o luautil.o

CXXFLAGS = -O2 -DGIDEROS_LIBRARY $(INCLUDEPATHS)

links = gid.dll lua.dll pystring.dll

%.o : %.cpp
	g++ $(CXXFLAGS) -c $<

%.o : %.c
	gcc $(CXXFLAGS) -c $<

gideros.dll: $(objfiles) $(links)
	g++ -o gideros.dll -shared $(objfiles) $(links)

-include libgideros.dep

.PHONY : depend
depend:
	g++ $(INCLUDEPATHS) -MM ../libgideros/*.cpp > libgideros.dep
