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
..\2dsg\paths;\
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
-I..\2dsg\paths \
-I..\libgid\include

CXXFLAGS = -O2 -fno-keep-inline-dllexport $(INCLUDEPATHS)

#-DPTW32_STATIC_LIB \
#-DOPT_GENERIC \
#-DREAL_IS_FLOAT \
#-DFT2_BUILD_LIBRARY \
#-DPYSTRING_LIBRARY

# ..\libgid\external\libpng-1.6.2\build\mingw48_32\libpng.a \
#..\libgid\external\jpeg-9\build\mingw48_32\libjpeg.a \
#..\libgid\external\mpg123-1.15.3\lib\mingw48_32\libmpg123.a \
#..\libgid\external\openal-soft-1.13\build\mingw48_32\libOpenAL32.dll.a \
#..\libgid\external\pthreads-w32-2-9-1-release\Pre-built.2\lib\x86\libpthreadGC2.a \

links = \
..\libgid\external\freetype-2.4.12\build\mingw48_32\libfreetype.a \
..\libgid\external\zlib-1.2.8\build\mingw48_32\libzlibx.a \
-L"../libgid/external/glew-1.10.0/lib/mingw48_32" -lglew32 \
gvfs.dll \
lua.dll \
gideros.dll \
pystring.dll \
gid.dll \
-lopengl32 -luser32 -lgdi32 -lcomdlg32 -lcomctl32 -lws2_32 -liphlpapi

linksnoconsole = \
..\libgid\external\freetype-2.4.12\build\mingw48_32\libfreetype.a \
..\libgid\external\zlib-1.2.8\build\mingw48_32\libzlibx.a \
-L"../libgid/external/glew-1.10.0/lib/mingw48_32" -lglew32 \
gvfs.dll \
lua.dll \
gideros.dll \
pystring.dll \
gid.dll \
-lopengl32 -mwindows -lws2_32 -liphlpapi


objfiles = win32.o \
accelerometerbinder.o         alertdialogbinder.o \
application.o                 applicationbinder.o \
asyncloader.o                 audiobinder.o \
b2BlockAllocator.o \
b2Body.o                      b2BroadPhase.o \
b2ChainAndCircleContact.o     b2ChainAndPolygonContact.o \
b2ChainShape.o                b2CircleContact.o \
b2CircleShape.o               b2CollideCircle.o \
b2CollideEdge.o               b2CollidePolygon.o \
b2Collision.o                 b2Contact.o \
b2ContactManager.o            b2ContactSolver.o \
b2Distance.o                  b2DistanceJoint.o \
b2Draw.o                      b2DynamicTree.o \
b2EdgeAndCircleContact.o      b2EdgeAndPolygonContact.o \
b2EdgeShape.o                 b2Fixture.o \
b2FreeList.o                  b2FrictionJoint.o \
b2GearJoint.o                 b2Island.o \
b2Joint.o                     b2Math.o \
b2MotorJoint.o                b2MouseJoint.o \
b2Particle.o                  b2ParticleGroup.o \
b2ParticleSystem.o            b2PolygonAndCircleContact.o \
b2PolygonContact.o            b2PolygonShape.o \
b2PrismaticJoint.o            b2PulleyJoint.o \
b2RevoluteJoint.o             b2Rope.o \
b2RopeJoint.o                 b2Settings.o \
b2StackAllocator.o            b2Stat.o \
b2TimeOfImpact.o              b2Timer.o \
b2TrackedBlock.o              b2VoronoiDiagram.o \
b2WeldJoint.o                 b2WheelJoint.o \
b2World.o                     b2WorldCallbacks.o \
binder.o                      bitmap.o \
bitmapbinder.o                bitmapdata.o \
bitmapdatabinder.o            blendfunc.o \
box2dbinder2.o                bytebuffer.o \
color.o \
colortransform.o              completeevent.o \
dib.o                         dibbinder.o \
dict.o                        drawinfo.o \
enterframeevent.o \
errorevent.o                  eventbinder.o \
eventdispatcherbinder.o       font.o \
fontbase.o                    fontbasebinder.o \
fontbinder.o \
gbitmapbatch.o                geolocationbinder.o \
geom.o \
glcompat.o                    gmesh.o \
graphicsbase.o                grendertarget.o \
gstatus.o                     gtimer.o \
gyroscopebinder.o \
keyboardevent.o               keys.o \
libnetwork.o                  luaapplication.o \
Matrices.o                    matrix.o \
matrixbinder.o                MaxRectsBinPack.o \
aes.o md5.o                         memalloc.o \
mesh.o                        meshbinder.o \
mouseevent.o                  movieclip2.o \
movieclipbinder.o             normal.o \
ogl.o                         platform-win32.o \
platformutil.o \
priorityq.o                   progressevent.o \
PVRTDecompress.o              PVRTResourceFile.o \
PVRTString.o                  PVRTTexture.o \
registermodules.o \
render.o                      rendertargetbinder.o \
shape.o                       shapebinder.o \
splashscreen.o \
sprite.o                      spritebinder.o \
stackchecker.o                stage.o \
stagebinder.o                 stageorientationevent.o \
stopwatch.o                   sweep.o \
tess.o                        tessmono.o \
textfield.o                   textfieldbase.o \
textfieldbinder.o             textinputdialogbinder.o \
texture.o                     texturebase.o \
texturebasebinder.o           texturebinder.o \
texturemanager.o              texturepack.o \
texturepackbinder.o           texturepacker.o \
tilemap.o                     tilemapbinder.o \
timer.o                       timerbinder.o \
timercontainer.o              timerevent.o \
tlsf.o                        touch.o \
touchevent.o                  transform.o \
ttbmfont.o \
ttfont.o                      ttfontbinder.o \
tttextfield.o \
urlloaderbinder.o             urlrequest.o \
utf8.o                        zlibbinder.o cryptobinder.o \
Shaders.o \
gl2ShaderBuffer.o \
gl2ShaderEngine.o \
gl2ShaderProgram.o \
gl2ShaderTexture.o \
shaderbinder.o gl2PathShaders.o \
ftlibrarysingleton.o \
viewport.o viewportbinder.o \
pixel.o pixelbinder.o \
particles.o particlesbinder.o \
path2dbinder.o path.o ft-path.o svg-path.o \
win32_res.o

%.o : %.cpp
	g++ $(CXXFLAGS) -c $<

%.o : %.c
	gcc $(CXXFLAGS) -c $<

.PHONY : all
all: win32.exe win32noconsole.exe

win32.exe: $(objfiles) gvfs.dll lua.dll gideros.dll pystring.dll gid.dll
	g++ -o win32.exe $(objfiles) $(links)

win32noconsole.exe: $(objfiles) gvfs.dll lua.dll gideros.dll pystring.dll gid.dll
	g++ -o win32noconsole.exe $(objfiles) $(linksnoconsole)

win32_res.o: ..\libgid\src\win32\win32_res.rc
	windres $< win32_res.o

# b2Timer needs to undefine _WIN32
b2Timer.o: \
 ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Common\b2Timer.cpp \
 ..\external\liquidfun-1.0.0\liquidfun\Box2D/Box2D/Common/b2Timer.h \
 ..\external\liquidfun-1.0.0\liquidfun\Box2D/Box2D/Common/b2Settings.h
	g++ -U_WIN32 $(CXXFLAGS) -c $<

# needs C++11
shaderbinder.o: ..\luabinding\shaderbinder.cpp \
 ..\luabinding\shaderbinder.h ..\luabinding\binder.h ..\lua\src/lua.hpp \
 ..\lua\src/lua.h ..\lua\src/luaconf.h ..\lua\src/lualib.h \
 ..\lua\src/lauxlib.h ..\2dsg\gfxbackends/Shaders.h ..\2dsg/Matrices.h \
 ..\2dsg/Vectors.h ..\luabinding\stackchecker.h \
 ..\luabinding\luaapplication.h ..\libgideros/gideros_p.h \
 ..\libgideros/gexport.h ..\libgid\include/gglobal.h ..\libgvfs/gexport.h \
 ..\2dsg/orientation.h ..\2dsg/gstatus.h ..\libgideros/luautil.h \
 ..\lua\src/lua.h ..\lua\src/lauxlib.h ..\lua\src/lualib.h
	g++ -std=c++11 $(CXXFLAGS) -c $<

-include win32.dep
-include box2d1.dep
-include box2d2.dep
-include box2d3.dep
-include box2d4.dep
-include box2d5.dep
-include box2d6.dep
-include box2d7.dep
-include box2d8.dep
-include 2dsg.dep
-include 2dsg1.dep
-include 2dsg2.dep
-include luabinding.dep
-include tlsf.dep
-include libnetwork.dep
-include libpvrt.dep
-include libtess.dep
-include aes.dep
-include md5.dep
-include utf8.dep
-include ioapi.dep
-include unzip.dep

.PHONY : depend
depend :
	g++ $(INCLUDEPATHS) -MM win32.cpp > win32.dep
	g++ $(INCLUDEPATHS) -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Common\*.cpp > box2d1.dep
	g++ $(INCLUDEPATHS) -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Collision\*.cpp > box2d2.dep
	g++ $(INCLUDEPATHS) -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Collision\Shapes\*.cpp > box2d3.dep
	g++ $(INCLUDEPATHS) -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Dynamics\*.cpp > box2d4.dep
	g++ $(INCLUDEPATHS) -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Dynamics\Contacts\*.cpp > box2d5.dep
	g++ $(INCLUDEPATHS) -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Dynamics\Joints\*.cpp > box2d6.dep
	g++ $(INCLUDEPATHS) -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Rope\*.cpp > box2d7.dep
	g++ $(INCLUDEPATHS) -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Particle\*.cpp > box2d8.dep
	g++ $(INCLUDEPATHS) -MM ..\2dsg\*.cpp > 2dsg.dep
	g++ $(INCLUDEPATHS) -MM ..\2dsg\gfxbackends\*.cpp > 2dsg1.dep
	g++ $(INCLUDEPATHS) -MM ..\2dsg\gfxbackends\gl2\*.cpp > 2dsg2.dep
	g++ $(INCLUDEPATHS) -MM ..\luabinding\*.cpp  > luabinding.dep
	g++ $(INCLUDEPATHS) -MM ..\luabinding\tlsf.c > tlsf.dep
	g++ $(INCLUDEPATHS) -MM ..\libnetwork\*.cpp > libnetwork.dep
	g++ $(INCLUDEPATHS) -MM ..\libpvrt\*.cpp > libpvrt.dep
	g++ $(INCLUDEPATHS) -MM ..\external\glu\libtess\*.c > libtess.dep
	g++ $(INCLUDEPATHS) -MM ..\libgid\src\aes.c > aes.dep
	g++ $(INCLUDEPATHS) -MM ..\libgid\src\md5.c > md5.dep
	g++ $(INCLUDEPATHS) -MM ..\libgid\src\utf8.c > utf8.dep
	g++ $(INCLUDEPATHS) -MM ../external/minizip-1.1/source/ioapi.c > ioapi.dep
	g++ $(INCLUDEPATHS) -MM ../external/minizip-1.1/source/unzip.c > unzip.dep
	g++ $(INCLUDEPATHS) -MM ../2dsg/paths/*.cpp > paths.dep
	g++ $(INCLUDEPATHS) -MM ../2dsg/paths/*.c > paths2.dep
