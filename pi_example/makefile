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
../2dsg/paths:\
../luabinding:\
../libnetwork:\
../libpvrt:\
../external/glu/libtess:\
../libgid/src:\
../libgid/src/pi:\
../external/minizip-1.1/source:\
../libgid/external/freetype-2.4.12/src/base \
../libgid/external/freetype-2.4.12/src/raster: \
../libgid/external/freetype-2.4.12/src/sfnt: \
../libgid/external/freetype-2.4.12/src/smooth: \
../libgid/external/freetype-2.4.12/src/autofit: \
../libgid/external/freetype-2.4.12/src/truetype: \
../libgid/external/freetype-2.4.12/src/cff: \
../libgid/external/freetype-2.4.12/src/psnames: \
../libgid/external/freetype-2.4.12/src/pshinter


INCLUDEPATHS = \
-I../luabinding \
-I../libnetwork \
-I../libgid/external/glew-1.10.0/include \
-I../libgid/external/freetype-2.4.12/src \
-I../libgid/external/freetype-2.4.12/include \
-I../libgid/external/freetype-2.4.12/include/freetype \
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
-I../2dsg/paths \
-I../libgid/include \
-I/opt/vc/include \
-I/opt/vc/include/interface/vmcs_host/linux \
-I/opt/vc/include/interface/vcos/pthreads

CXXFLAGS = -Og -g -D_REENTRANT -DSTRICT_LINUX -DRASPBERRY_PI -DRPI_NO_X -DFT2_BUILD_LIBRARY -std=gnu++0x -fPIE $(INCLUDEPATHS)
  CFLAGS = -Og -g -D_REENTRANT -DSTRICT_LINUX -DRASPBERRY_PI -DRPI_NO_X -DFT2_BUILD_LIBRARY -fPIE $(INCLUDEPATHS)

#-DPTW32_STATIC_LIB /
#-DOPT_GENERIC /
#-DREAL_IS_FLOAT /
#-DFT2_BUILD_LIBRARY /
#-DPYSTRING_LIBRARY

LINKS = \
../libgid/external/zlib-1.2.8/build/gcc463_pi/libzlibx.a \
gvfs.so \
lua.so \
gideros.so \
pystring.so \
gid.so \
-lpthread \
-lGLESv2 -lEGL -lm -lbcm_host -L/opt/vc/lib

OBJFILES = pi.o \
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
aes.o	md5.o                         memalloc.o \
mesh.o                        meshbinder.o \
mouseevent.o                  movieclip2.o \
movieclipbinder.o             normal.o \
ogl.o                         platform-pi.o \
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
gl2ShaderTexture.o gl2PathShaders.o \
shaderbinder.o \
ftlibrarysingleton.o \
viewport.o viewportbinder.o \
pixel.o pixelbinder.o \
particles.o particlesbinder.o \
esUtil.o \
ftbbox.o ftbitmap.o ftglyph.o ftlcdfil.o ftstroke.o ftxf86.o ftbase.o ftsystem.o ftinit.o ftgasp.o \
raster.o sfnt.o smooth.o autofit.o truetype.o cff.o psnames.o pshinter.o \
path2dbinder.o path.o ft-path.o svg-path.o

%.o : %.cpp
	g++ $(CXXFLAGS) -c $<

%.o : %.c
	gcc $(CFLAGS) -c $<

pi: $(OBJFILES) gvfs.so lua.so gideros.so pystring.so gid.so
	g++ -o pi $(OBJFILES) $(LINKS)

-include pi.dep
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
-include md5.dep
-include aes.dep
-include utf8.dep
-include ioapi.dep
-include unzip.dep
-include ft1.dep
-include ft2.dep
-include ft3.dep
-include ft4.dep
-include ft5.dep
-include ft6.dep
-include ft7.dep
-include ft8.dep
-include ft9.dep
-include paths.dep
-include paths2.dep

.PHONY : depend
depend :
	g++ $(INCLUDEPATHS) -MM pi.cpp esUtil.c > pi.dep
	g++ $(INCLUDEPATHS) -MM ../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Common/*.cpp > box2d1.dep
	g++ $(INCLUDEPATHS) -MM ../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/*.cpp > box2d2.dep
	g++ $(INCLUDEPATHS) -MM ../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/Shapes/*.cpp > box2d3.dep
	g++ $(INCLUDEPATHS) -MM ../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/*.cpp > box2d4.dep
	g++ $(INCLUDEPATHS) -MM ../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Contacts/*.cpp > box2d5.dep
	g++ $(INCLUDEPATHS) -MM ../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints/*.cpp > box2d6.dep
	g++ $(INCLUDEPATHS) -MM ../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Rope/*.cpp > box2d7.dep
	g++ $(INCLUDEPATHS) -MM ../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Particle/*.cpp > box2d8.dep
	g++ $(INCLUDEPATHS) -MM ../2dsg/*.cpp > 2dsg.dep
	g++ $(INCLUDEPATHS) -MM ../2dsg/gfxbackends/*.cpp > 2dsg1.dep
	g++ $(INCLUDEPATHS) -MM ../2dsg/gfxbackends/gl2/*.cpp > 2dsg2.dep
	g++ $(INCLUDEPATHS) -MM ../luabinding/*.cpp  > luabinding.dep
	g++ $(INCLUDEPATHS) -MM ../luabinding/tlsf.c > tlsf.dep
	g++ $(INCLUDEPATHS) -MM ../libnetwork/*.cpp > libnetwork.dep
	g++ $(INCLUDEPATHS) -MM ../libpvrt/*.cpp > libpvrt.dep
	g++ $(INCLUDEPATHS) -MM ../external/glu/libtess/*.c > libtess.dep
	g++ $(INCLUDEPATHS) -MM ../libgid/src/md5.c > md5.dep
	g++ $(INCLUDEPATHS) -MM ../libgid/src/aes.c > aes.dep
	g++ $(INCLUDEPATHS) -MM ../libgid/src/utf8.c > utf8.dep
	g++ $(INCLUDEPATHS) -MM ../external/minizip-1.1/source/ioapi.c > ioapi.dep
	g++ $(INCLUDEPATHS) -MM ../external/minizip-1.1/source/unzip.c > unzip.dep
	g++ $(INCLUDEPATHS) -MM -DFT2_BUILD_LIBRARY ../libgid/external/freetype-2.4.12/src/base/*.c > ft1.dep
	g++ $(INCLUDEPATHS) -MM -DFT2_BUILD_LIBRARY ../libgid/external/freetype-2.4.12/src/raster/raster.c > ft2.dep
	g++ $(INCLUDEPATHS) -MM -DFT2_BUILD_LIBRARY ../libgid/external/freetype-2.4.12/src/sfnt/sfnt.c > ft3.dep
	g++ $(INCLUDEPATHS) -MM -DFT2_BUILD_LIBRARY ../libgid/external/freetype-2.4.12/src/smooth/smooth.c > ft4.dep
	g++ $(INCLUDEPATHS) -MM -DFT2_BUILD_LIBRARY ../libgid/external/freetype-2.4.12/src/autofit/autofit.c > ft5.dep
	g++ $(INCLUDEPATHS) -MM -DFT2_BUILD_LIBRARY ../libgid/external/freetype-2.4.12/src/truetype/truetype.c > ft6.dep
	g++ $(INCLUDEPATHS) -MM -DFT2_BUILD_LIBRARY ../libgid/external/freetype-2.4.12/src/cff/cff.c > ft7.dep
	g++ $(INCLUDEPATHS) -MM -DFT2_BUILD_LIBRARY ../libgid/external/freetype-2.4.12/src/psnames/psnames.c > ft8.dep
	g++ $(INCLUDEPATHS) -MM -DFT2_BUILD_LIBRARY ../libgid/external/freetype-2.4.12/src/pshinter/pshinter.c > ft9.dep
	g++ $(INCLUDEPATHS) -MM ../2dsg/paths/*.cpp > paths.dep
	g++ $(INCLUDEPATHS) -MM ../2dsg/paths/*.c > paths2.dep

.PHONY : clean
clean :
	rm $(OBJFILES) pi
