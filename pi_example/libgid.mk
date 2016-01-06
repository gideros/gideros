VPATH = \
../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Common:\
../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision:\
../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/Shapes:\
../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics:\
../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Contacts:\
../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints:\
../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Rope:\
../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Particle:\
../2dsg:\
../2dsg/gfxbackends:\
../2dsg/gfxbackends/gl2:\
../luabinding:\
../libnetwork:\
../libpvrt:\
../external/glu/libtess:\
../libgid/src:\
../libgid/src/pi:\
../external/minizip-1.1/source

# -I../libgid/external/glew-1.10.0/include

INCLUDEPATHS = \
-I../luabinding \
-I../libnetwork \
-I../libgid/external/freetype-2.4.12/src \
-I../libgid/external/freetype-2.4.12/include \
-I../libgid/external/mpg123-1.15.3/src \
-I../external/liquidfun-1.0.0/liquidfun/Box2D \
-I../libgid/include/pi \
-I../libgid/external/snappy-1.1.0 \
-I../libgid/external/libpng-1.6.2 \
-I../libgid/external/jpeg-9 \
-I../libgid/external/openal-soft-1.13/include/AL \
-I../libgid/external/mpg123-1.15.3/src/libmpg123 \
-I../lua/src \
-I../libgid/external/zlib-1.2.8 \
-I../external/glu \
-I../libpystring \
-I../libgvfs \
-I../libgideros \
-I../2dsg \
-I../2dsg/gfxbackends \
-I../2dsg/gfxbackends/gl2 \
-I../libgid/include

objfiles = \
gapplication-pi.o \
gaudio-pi.o \
ggeolocation-pi.o \
ghttp-pi.o \
ginput-pi.o \
gui-pi.o \
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

CXXFLAGS = -Og -g -D_REENTRANT -DGIDEROS_LIBRARY -DSTRICT_LINUX -std=gnu++0x -fPIC $(INCLUDEPATHS)
  CFLAGS = -Og -g -D_REENTRANT -DGIDEROS_LIBRARY -DSTRICT_LINUX -fPIC $(INCLUDEPATHS)

links = \
../libgid/external/jpeg-9/build/gcc463_pi/libjpeg.a \
../libgid/external/libpng-1.6.2/build/gcc463_pi/libpng.a \
../libgid/external/mpg123-1.15.3/lib/gcc463_pi/libmpg123.a \
../libgid/external/openal-soft-1.13/build/gcc463_pi/libopenal.so \
../libgid/external/zlib-1.2.8/build/gcc463_pi/libzlibx.a \
-lpthread gvfs.so

# ../libgid/external/glew-1.10.0/lib/gcc463_pi/libGLEW.a 

%.o : %.cpp
	g++ $(CXXFLAGS) -c $<

%.o : %.c
	gcc $(CFLAGS) -c $<

gid.so: $(objfiles) gvfs.so
	g++ -o gid.so -shared $(objfiles) $(links)

#ginput-pi.o: ../libgid/src/pi/ginput-pi.cpp \
# ../libgid/include/ginput.h ../libgid/include/gglobal.h \
# ../libgvfs/gexport.h ../libgid/include/gevent.h \
# ../libgid/include/pi/ginput-pi.h
#	g++ -std=c++11 $(CXXFLAGS) -c $<

snappy.o: ../libgid/external/snappy-1.1.0/snappy.cpp
	g++ $(CXXFLAGS) -c $<

snappy-c.o: ../libgid/external/snappy-1.1.0/snappy-c.cpp
	g++ $(CXXFLAGS) -c $<

snappy-sinksource.o: ../libgid/external/snappy-1.1.0/snappy-sinksource.cpp
	g++ $(CXXFLAGS) -c $<

snappy-stubs-internal.o: ../libgid/external/snappy-1.1.0/snappy-stubs-internal.cpp
	g++ $(CXXFLAGS) -c $<

-include libgid.dep
-include libgidpi.dep

.PHONY : depend
depend :
	g++ $(INCLUDEPATHS) -MM ../libgid/src/*.cpp > libgid.dep
	g++ $(INCLUDEPATHS) -MM ../libgid/src/pi/*.cpp > libgidpi.dep

.PHONY : clean
clean :
	rm $(objfiles)
