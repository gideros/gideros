VPATH = \
..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Common;\
..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Collision;\
..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Collision\Shapes;\
..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Dynamics;\
..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Dynamics\Contacts;\
..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Dynamics\Joints;\
..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Rope;\
..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Particle;\
..\2dsg;\
..\2dsg\gfxbackends;\
..\2dsg\gfxbackends\gl2;\
..\luabinding;\
..\libnetwork;\
..\libpvrt;\
..\external\glu\libtess;\
..\libgid\src;\
..\libgid\src\win32;\
../external/minizip-1.1/source

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
-I..\libgid\include \
-I..\libgid\external\curl-7.40.0-devel-mingw32\include

objfiles = \
gapplication-win32.o \
gaudio-win32.o \
ggeolocation-win32.o \
ghttp-win32.o \
ginput-win32.o \
gui-win32.o \
gaudio-loader-mp3.o \
gaudio-loader-wav.o \
gaudio-sample-openal.o \
gaudio-stream-openal.o \
gaudio.o \
gevent.o \
gglobal.o \
gimage-jpg.o \
gimage-png.o \
gimage.o \
glog.o \
gtexture.o \
gtimer.o \
gvfs-native.o \
snappy.o \
snappy-c.o \
snappy-sinksource.o \
snappy-stubs-internal.o

CXXFLAGS = -O2 -DGIDEROS_LIBRARY -fno-keep-inline-dllexport $(INCLUDEPATHS)

links = ..\libgid\external\freetype-2.4.12\build\mingw48_32\libfreetype.a \
..\libgid\external\jpeg-9\build\mingw48_32\libjpeg.a \
..\libgid\external\libpng-1.6.2\build\mingw481_win32\libpng.a \
..\libgid\external\mpg123-1.15.3\lib\mingw48_32\libmpg123.a \
..\libgid\external\openal-soft-1.13\build\mingw48_32\libOpenAL32.dll.a \
..\libgid\external\pthreads-w32-2-9-1-release\Pre-built.2\lib\x86\libpthreadGC2.a \
..\libgid\external\zlib-1.2.8\build\mingw48_32\libzlibx.a \
..\libgid\external\curl-7.40.0-devel-mingw32\lib\libcurldll.a \
-L"../libgid/external/glew-1.10.0/lib/mingw48_32" -lglew32 -lopengl32 \
gvfs.dll

%.o : %.cpp
	g++ $(CXXFLAGS) -c $<

%.o : %.c
	gcc $(CXXFLAGS) -c $<

gid.dll: $(objfiles) gvfs.dll
	g++ -o gid.dll -shared $(objfiles) $(links)

win32_res.o: ..\libgid\src\win32\win32_res.rc
	windres $< win32_res.o

# uses C++ 2011 code
ginput-win32.o: ..\libgid\src\win32\ginput-win32.cpp \
 ..\libgid\include/ginput.h ..\libgid\include/gglobal.h \
 ..\libgvfs/gexport.h ..\libgid\include/gevent.h \
 ..\libgid\include\win32/ginput-win32.h \
 ..\libgid\external\pthreads-w32-2-9-1-release\Pre-built.2\include/pthread.h \
 ..\libgid\external\pthreads-w32-2-9-1-release\Pre-built.2\include/sched.h
	g++ -std=c++11 $(CXXFLAGS) -c $<

snappy.o: ..\libgid\external\snappy-1.1.0\snappy.cpp
	g++ $(CXXFLAGS) -c $<

snappy-c.o: ..\libgid\external\snappy-1.1.0\snappy-c.cpp
	g++ $(CXXFLAGS) -c $<

snappy-sinksource.o: ..\libgid\external\snappy-1.1.0\snappy-sinksource.cpp
	g++ $(CXXFLAGS) -c $<

snappy-stubs-internal.o: ..\libgid\external\snappy-1.1.0\snappy-stubs-internal.cpp
	g++ $(CXXFLAGS) -c $<

-include libgid.dep
-include libgidwin32.dep

.PHONY : depend
depend :
	g++ $(INCLUDEPATHS) -MM ../libgid/src/*.cpp > libgid.dep
	g++ $(INCLUDEPATHS) -MM ../libgid/src/win32/*.cpp > libgidwin32.dep
