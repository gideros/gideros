LOCAL_PATH := $(call my-dir)

####

include $(CLEAR_VARS)

LOCAL_MODULE            := gvfs
LOCAL_SRC_FILES         := ../../../libgvfs/libs/$(TARGET_ARCH_ABI)/libgvfs.so

include $(PREBUILT_SHARED_LIBRARY)

####

include $(CLEAR_VARS)

LOCAL_MODULE            := lua
LOCAL_SRC_FILES         := ../../../lua/libs/$(TARGET_ARCH_ABI)/liblua.so

include $(PREBUILT_SHARED_LIBRARY)

####

include $(CLEAR_VARS)
LOCAL_MODULE			:= openal

LOCAL_OPENAL_VERSION=1.17.2

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../../openal-soft-$(LOCAL_OPENAL_VERSION)/include \
	$(LOCAL_PATH)/../../openal-soft-$(LOCAL_OPENAL_VERSION)/OpenAL32/Include \
	$(LOCAL_PATH)/../../openal-soft-$(LOCAL_OPENAL_VERSION)/include \
	$(LOCAL_PATH)/../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc \
	$(LOCAL_PATH)/../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/backends

# openal (24 files)
LOCAL_CFLAGS += -DAL_BUILD_LIBRARY -DAL_ALEXT_PROTOTYPES -DOPENAL_SUBDIR_AL -std=c99
#openal Common FILES
LOCAL_SRC_FILES  := \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/OpenAL32/alAuxEffectSlot.c \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/OpenAL32/alBuffer.c        \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/OpenAL32/alEffect.c        \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/OpenAL32/alError.c         \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/OpenAL32/alExtension.c     \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/OpenAL32/alFilter.c        \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/OpenAL32/alListener.c      \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/OpenAL32/alSource.c        \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/OpenAL32/alState.c         \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/OpenAL32/alThunk.c         \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/ALc.c                  \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/alcConfig.c            \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/alcRing.c              \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/ALu.c                  \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/bs2b.c                 \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/mixer.c                \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/panning.c              

#1.13 files
#LOCAL_SRC_FILES  += \ 
#	../../openal-soft-$(LOCAL_OPENAL_VERSION)/OpenAL32/alDatabuffer.c    \
#	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/alcEcho.c              \
#	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/alcModulator.c         \
#	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/alcReverb.c            \
#	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/alcThread.c            \
#	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/android.c              \
#	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/null.c
#LOCAL_SRC_FILES += audiodevice.cpp

#1.17 files
LOCAL_SRC_FILES  += \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/bsinc.c                 \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/helpers.c                 \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/hrtf.c                 \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/mixer_c.c                 \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/effects/autowah.c       \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/effects/chorus.c       \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/effects/compressor.c       \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/effects/dedicated.c       \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/effects/distortion.c       \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/effects/echo.c       \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/effects/equalizer.c       \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/effects/flanger.c       \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/effects/modulator.c       \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/effects/null.c       \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/effects/reverb.c        \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/backends/base.c        \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/backends/opensl.c        \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/backends/null.c        \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/backends/loopback.c        \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/OpenAL32/sample_cvt.c         \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/common/atomic.c        \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/common/rwlock.c        \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/common/threads.c        \
	../../openal-soft-$(LOCAL_OPENAL_VERSION)/common/uintmap.c        

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := gideros

LOCAL_CFLAGS := -O2

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES

LOCAL_OPENAL_VERSION=1.17.2

LOCAL_CFLAGS += -DAL_BUILD_LIBRARY -DAL_ALEXT_PROTOTYPES -DOPENAL_SUBDIR_AL 

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../../../2dsg \
	$(LOCAL_PATH)/../../../2dsg/paths \
	$(LOCAL_PATH)/../../../libnetwork \
	$(LOCAL_PATH)/../../../external/glu \
	$(LOCAL_PATH)/../../../libpvrt \
	$(LOCAL_PATH)/../../../lua/src \
	$(LOCAL_PATH)/../../../luabinding \
	$(LOCAL_PATH)/../../../libgid/include \
	$(LOCAL_PATH)/../../../libgid/include/private \
	$(LOCAL_PATH)/../../../libgid/include/android \
	$(LOCAL_PATH)/../../../libgid/external/snappy-1.1.0 \
	$(LOCAL_PATH)/../../../libgvfs \
	$(LOCAL_PATH)/../../../libgideros \
	$(LOCAL_PATH)/../../../libpystring \
	$(LOCAL_PATH)/../../openal-soft-$(LOCAL_OPENAL_VERSION)/include \
	$(LOCAL_PATH)/../../openal-soft-$(LOCAL_OPENAL_VERSION)/OpenAL32/Include \
	$(LOCAL_PATH)/../../openal-soft-$(LOCAL_OPENAL_VERSION)/include \
	$(LOCAL_PATH)/../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc \
	$(LOCAL_PATH)/../../openal-soft-$(LOCAL_OPENAL_VERSION)/Alc/backends \
	$(LOCAL_PATH)/../../../libgid/external/libxmp-4.3/include \
	$(LOCAL_PATH)/../../../libgid/external/jpeg-9

LOCAL_SRC_FILES += gideros.cpp


LOCAL_SRC_FILES += \
    ../../../libgid/src/gimage-png.cpp \
    ../../../libgid/src/gimage-jpg.cpp \
    ../../../libgid/src/gimage.cpp \
    ../../../libgid/src/gtexture.cpp \
    ../../../libgid/src/gevent.cpp \
    ../../../libgid/src/glog.cpp \
	../../../libgid/src/gglobal.cpp \
	../../../libgid/src/gaudio.cpp \
	../../../libgid/src/gaudio-loader-wav.cpp \
	../../../libgid/src/gaudio-loader-xmp.cpp \
	../../../libgid/src/gaudio-sample-openal.cpp \
	../../../libgid/src/gaudio-stream-openal.cpp \
    ../../../libgid/src/android/ginput-android.cpp \
    ../../../libgid/src/android/gvfs-android.cpp \
	../../../libgid/src/android/ggeolocation-android.cpp \
    ../../../libgid/src/android/gui-android.cpp \
    ../../../libgid/src/android/gapplication-android.cpp \
    ../../../libgid/src/android/gaudio-android.cpp \
	../../../libgid/src/android/gaudio-background-mediaplayer.cpp \
	../../../libgid/src/android/ghttp-android.cpp
	
LOCAL_SRC_FILES += \
    ../../../libgid/external/snappy-1.1.0/snappy.cpp \
    ../../../libgid/external/snappy-1.1.0/snappy-c.cpp \
    ../../../libgid/external/snappy-1.1.0/snappy-sinksource.cpp \
    ../../../libgid/external/snappy-1.1.0/snappy-stubs-internal.cpp


# modified from http://source-android.frandroid.com/external/freetype/Android.mk
# also ftmodule.h should be configured like http://source-android.frandroid.com/external/freetype/include/freetype/config/ftmodule.h
LOCAL_CFLAGS += -DFT2_BUILD_LIBRARY
LOCAL_CFLAGS += -DDARWIN_NO_CARBON
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../../../libgid/external/freetype-2.4.12/include \
	$(LOCAL_PATH)/../../../libgid/external/freetype-2.4.12/src
LOCAL_SRC_FILES += \
	../../../libgid/external/freetype-2.4.12/src/base/ftbbox.c \
	../../../libgid/external/freetype-2.4.12/src/base/ftbitmap.c \
	../../../libgid/external/freetype-2.4.12/src/base/ftglyph.c \
	../../../libgid/external/freetype-2.4.12/src/base/ftlcdfil.c \
	../../../libgid/external/freetype-2.4.12/src/base/ftstroke.c \
	../../../libgid/external/freetype-2.4.12/src/base/ftxf86.c \
	../../../libgid/external/freetype-2.4.12/src/base/ftbase.c \
	../../../libgid/external/freetype-2.4.12/src/base/ftsystem.c \
	../../../libgid/external/freetype-2.4.12/src/base/ftinit.c \
	../../../libgid/external/freetype-2.4.12/src/base/ftgasp.c \
	../../../libgid/external/freetype-2.4.12/src/raster/raster.c \
	../../../libgid/external/freetype-2.4.12/src/sfnt/sfnt.c \
	../../../libgid/external/freetype-2.4.12/src/smooth/smooth.c \
	../../../libgid/external/freetype-2.4.12/src/autofit/autofit.c \
	../../../libgid/external/freetype-2.4.12/src/truetype/truetype.c \
	../../../libgid/external/freetype-2.4.12/src/cff/cff.c \
	../../../libgid/external/freetype-2.4.12/src/psnames/psnames.c \
	../../../libgid/external/freetype-2.4.12/src/pshinter/pshinter.c

# jpeg-9
LOCAL_SRC_FILES += \
	../../../libgid/external/jpeg-9/jaricom.c \
	../../../libgid/external/jpeg-9/jcapimin.c \
	../../../libgid/external/jpeg-9/jcapistd.c \
	../../../libgid/external/jpeg-9/jcarith.c \
	../../../libgid/external/jpeg-9/jccoefct.c \
	../../../libgid/external/jpeg-9/jccolor.c \
	../../../libgid/external/jpeg-9/jcdctmgr.c \
	../../../libgid/external/jpeg-9/jchuff.c \
	../../../libgid/external/jpeg-9/jcinit.c \
	../../../libgid/external/jpeg-9/jcmainct.c \
	../../../libgid/external/jpeg-9/jcmarker.c \
	../../../libgid/external/jpeg-9/jcmaster.c \
	../../../libgid/external/jpeg-9/jcomapi.c \
	../../../libgid/external/jpeg-9/jcparam.c \
	../../../libgid/external/jpeg-9/jcprepct.c \
	../../../libgid/external/jpeg-9/jcsample.c \
	../../../libgid/external/jpeg-9/jctrans.c \
	../../../libgid/external/jpeg-9/jdapimin.c \
	../../../libgid/external/jpeg-9/jdapistd.c \
	../../../libgid/external/jpeg-9/jdarith.c \
	../../../libgid/external/jpeg-9/jdatadst.c \
	../../../libgid/external/jpeg-9/jdatasrc.c \
	../../../libgid/external/jpeg-9/jdcoefct.c \
	../../../libgid/external/jpeg-9/jdcolor.c \
	../../../libgid/external/jpeg-9/jddctmgr.c \
	../../../libgid/external/jpeg-9/jdhuff.c \
	../../../libgid/external/jpeg-9/jdinput.c \
	../../../libgid/external/jpeg-9/jdmainct.c \
	../../../libgid/external/jpeg-9/jdmarker.c \
	../../../libgid/external/jpeg-9/jdmaster.c \
	../../../libgid/external/jpeg-9/jdmerge.c \
	../../../libgid/external/jpeg-9/jdpostct.c \
	../../../libgid/external/jpeg-9/jdsample.c \
	../../../libgid/external/jpeg-9/jdtrans.c \
	../../../libgid/external/jpeg-9/jerror.c \
	../../../libgid/external/jpeg-9/jfdctflt.c \
	../../../libgid/external/jpeg-9/jfdctfst.c \
	../../../libgid/external/jpeg-9/jfdctint.c \
	../../../libgid/external/jpeg-9/jidctflt.c \
	../../../libgid/external/jpeg-9/jidctfst.c \
	../../../libgid/external/jpeg-9/jidctint.c \
	../../../libgid/external/jpeg-9/jquant1.c \
	../../../libgid/external/jpeg-9/jquant2.c \
	../../../libgid/external/jpeg-9/jutils.c \
	../../../libgid/external/jpeg-9/jmemmgr.c \
	../../../libgid/external/jpeg-9/jmemnobs.c
		
LOCAL_SRC_FILES += \
	../../../2dsg/application.cpp \
	../../../2dsg/bitmap.cpp \
	../../../2dsg/bitmapdata.cpp \
	../../../2dsg/blendfunc.cpp \
	../../../2dsg/color.cpp \
	../../../2dsg/colortransform.cpp \
	../../../2dsg/dib.cpp \
	../../../2dsg/enterframeevent.cpp \
	../../../libgideros/event.cpp \
	../../../libgideros/eventdispatcher.cpp \
	../../../libgideros/eventvisitor.cpp \
	../../../2dsg/font.cpp \
	../../../2dsg/graphicsbase.cpp \
	../../../2dsg/gstatus.cpp \
	../../../2dsg/errorevent.cpp \
	../../../2dsg/matrix.cpp \
	../../../2dsg/mouseevent.cpp \
	../../../2dsg/keyboardevent.cpp \
	../../../2dsg/movieclip2.cpp \
	../../../2dsg/ogl.cpp \
	../../../2dsg/progressevent.cpp \
	../../../libgideros/refptr.cpp \
	../../../2dsg/shape.cpp \
	../../../2dsg/sprite.cpp \
	../../../2dsg/stage.cpp \
	../../../2dsg/stageorientationevent.cpp \
	../../../2dsg/stopwatch.cpp \
	../../../libgideros/stringid.cpp \
	../../../2dsg/textfield.cpp \
	../../../2dsg/texture.cpp \
	../../../2dsg/texturebase.cpp \
	../../../2dsg/texturepack.cpp \
	../../../2dsg/texturepacker.cpp \
	../../../2dsg/MaxRectsBinPack.cpp \
	../../../2dsg/texturemanager.cpp \
	../../../2dsg/tilemap.cpp \
	../../../2dsg/timer.cpp \
	../../../2dsg/timercontainer.cpp \
	../../../2dsg/timerevent.cpp \
	../../../2dsg/touch.cpp \
	../../../2dsg/touchevent.cpp \
	../../../2dsg/transform.cpp \
	../../../2dsg/urlrequest.cpp \
	../../../2dsg/fontbase.cpp \
	../../../2dsg/textfieldbase.cpp \
	../../../2dsg/ttfont.cpp \
	../../../2dsg/tttextfield.cpp \
	../../../2dsg/splashscreen.cpp \
	../../../2dsg/gmesh.cpp \
	../../../2dsg/ttbmfont.cpp \
	../../../2dsg/ftlibrarysingleton.cpp \
	../../../2dsg/grendertarget.cpp \
	../../../2dsg/completeevent.cpp \
	../../../2dsg/Matrices.cpp \
    ../../../2dsg/paths/path.cpp \
    ../../../2dsg/paths/ft-path.c \
    ../../../2dsg/paths/svg-path.c \
    ../../../2dsg/viewport.cpp \
	../../../2dsg/pixel.cpp \
	../../../2dsg/particles.cpp \
	../../../libgideros/luautil.cpp
	
# LOCAL_SRC_FILES += ../../../2dsg/clipper.cpp

	
LOCAL_SRC_FILES += \
	../../../libgid/src/aes.c \
	../../../libgid/src/md5.c \
	../../../libgid/src/utf8.c \
	../../../libgid/src/platformutil.cpp \
	../../../libgid/src/android/platform-android.cpp \
	../../../libgid/src/drawinfo.cpp \
	../../../libgid/src/android/javanativebridge.cpp \
	../../../libgideros/pluginmanager.cpp


LOCAL_SRC_FILES += \
	../../../libgideros/binderutil.cpp

LOCAL_SRC_FILES += \
	../../../libnetwork/bytebuffer.cpp \
	../../../libnetwork/libnetwork.cpp 

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../2dsg/gfxbackends $(LOCAL_PATH)/../../../2dsg/gfxbackends/gl2
LOCAL_SRC_FILES += \
	../../../2dsg/gfxbackends/Shaders.cpp \
	../../../2dsg/gfxbackends/gl2/gl2ShaderBuffer.cpp \
	../../../2dsg/gfxbackends/gl2/gl2ShaderTexture.cpp \
	../../../2dsg/gfxbackends/gl2/gl2ShaderProgram.cpp \
	../../../2dsg/gfxbackends/gl2/gl2PathShaders.cpp \
	../../../2dsg/gfxbackends/gl2/gl2ShaderEngine.cpp 
	
# zlib (12 files)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../libgid/external/zlib-1.2.8
LOCAL_SRC_FILES += \
        ../../../libgid/external/zlib-1.2.8/adler32.c \
        ../../../libgid/external/zlib-1.2.8/compress.c \
        ../../../libgid/external/zlib-1.2.8/crc32.c \
        ../../../libgid/external/zlib-1.2.8/deflate.c \
        ../../../libgid/external/zlib-1.2.8/gzclose.c \
        ../../../libgid/external/zlib-1.2.8/gzlib.c \
        ../../../libgid/external/zlib-1.2.8/gzread.c \
        ../../../libgid/external/zlib-1.2.8/gzwrite.c \
        ../../../libgid/external/zlib-1.2.8/inflate.c \
        ../../../libgid/external/zlib-1.2.8/infback.c \
        ../../../libgid/external/zlib-1.2.8/inftrees.c \
        ../../../libgid/external/zlib-1.2.8/inffast.c \
        ../../../libgid/external/zlib-1.2.8/trees.c \
        ../../../libgid/external/zlib-1.2.8/uncompr.c \
        ../../../libgid/external/zlib-1.2.8/zutil.c

# libpng (15 files)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../libgid/external/libpng-1.6.2
LOCAL_SRC_FILES += \
	../../../libgid/external/libpng-1.6.2/png.c \
	../../../libgid/external/libpng-1.6.2/pngerror.c \
	../../../libgid/external/libpng-1.6.2/pngget.c \
	../../../libgid/external/libpng-1.6.2/pngmem.c \
	../../../libgid/external/libpng-1.6.2/pngpread.c \
	../../../libgid/external/libpng-1.6.2/pngread.c \
	../../../libgid/external/libpng-1.6.2/pngrio.c \
	../../../libgid/external/libpng-1.6.2/pngrtran.c \
	../../../libgid/external/libpng-1.6.2/pngrutil.c \
	../../../libgid/external/libpng-1.6.2/pngset.c \
	../../../libgid/external/libpng-1.6.2/pngtrans.c \
	../../../libgid/external/libpng-1.6.2/pngwio.c \
	../../../libgid/external/libpng-1.6.2/pngwrite.c \
	../../../libgid/external/libpng-1.6.2/pngwtran.c \
	../../../libgid/external/libpng-1.6.2/pngwutil.c

# glu libtess (10 files)
LOCAL_SRC_FILES += \
	../../../external/glu/libtess/dict.c \
	../../../external/glu/libtess/geom.c \
	../../../external/glu/libtess/memalloc.c \
	../../../external/glu/libtess/mesh.c \
	../../../external/glu/libtess/normal.c \
	../../../external/glu/libtess/priorityq.c \
	../../../external/glu/libtess/render.c \
	../../../external/glu/libtess/sweep.c \
	../../../external/glu/libtess/tess.c \
	../../../external/glu/libtess/tessmono.c

LOCAL_SRC_FILES += ../../../libpystring/pystring.cpp

LOCAL_SRC_FILES += \
	../../../libpvrt/PVRTDecompress.cpp \
	../../../libpvrt/PVRTResourceFile.cpp \
	../../../libpvrt/PVRTString.cpp \
	../../../libpvrt/PVRTTexture.cpp

# liquidfun (53 files)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../external/liquidfun-1.0.0/liquidfun/Box2D
LOCAL_SRC_FILES += \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/b2BroadPhase.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/b2CollideCircle.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/b2CollideEdge.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/b2CollidePolygon.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/b2Collision.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/b2Distance.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/b2DynamicTree.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/b2TimeOfImpact.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/Shapes/b2ChainShape.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/Shapes/b2CircleShape.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/Shapes/b2EdgeShape.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Collision/Shapes/b2PolygonShape.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Common/b2BlockAllocator.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Common/b2Draw.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Common/b2FreeList.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Common/b2Math.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Common/b2Settings.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Common/b2StackAllocator.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Common/b2Stat.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Common/b2Timer.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Common/b2TrackedBlock.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/b2Body.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/b2ContactManager.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/b2Fixture.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/b2Island.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/b2World.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/b2WorldCallbacks.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Contacts/b2CircleContact.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Contacts/b2Contact.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Contacts/b2ContactSolver.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Contacts/b2PolygonContact.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints/b2DistanceJoint.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints/b2FrictionJoint.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints/b2GearJoint.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints/b2Joint.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints/b2MotorJoint.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints/b2MouseJoint.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints/b2PrismaticJoint.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints/b2PulleyJoint.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints/b2RevoluteJoint.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints/b2RopeJoint.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints/b2WeldJoint.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Dynamics/Joints/b2WheelJoint.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Particle/b2Particle.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Particle/b2ParticleGroup.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Particle/b2ParticleSystem.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Particle/b2VoronoiDiagram.cpp \
	../../../external/liquidfun-1.0.0/liquidfun/Box2D/Box2D/Rope/b2Rope.cpp

	
# luabinding (35 files)
LOCAL_SRC_FILES += \
	../../../luabinding/accelerometerbinder.cpp \
	../../../luabinding/applicationbinder.cpp \
	../../../luabinding/binder.cpp \
	../../../luabinding/bitmapbinder.cpp \
	../../../luabinding/bitmapdatabinder.cpp \
	../../../luabinding/box2dbinder2.cpp \
	../../../luabinding/dibbinder.cpp \
	../../../luabinding/eventbinder.cpp \
	../../../luabinding/eventdispatcherbinder.cpp \
	../../../luabinding/fontbinder.cpp \
	../../../luabinding/keys.cpp \
	../../../luabinding/luaapplication.cpp \
	../../../luabinding/matrixbinder.cpp \
	../../../luabinding/movieclipbinder.cpp \
	../../../luabinding/registermodules.cpp \
	../../../luabinding/shapebinder.cpp \
	../../../luabinding/spritebinder.cpp \
	../../../luabinding/stackchecker.cpp \
	../../../luabinding/stagebinder.cpp \
	../../../luabinding/textfieldbinder.cpp \
	../../../luabinding/texturebasebinder.cpp \
	../../../luabinding/texturebinder.cpp \
	../../../luabinding/texturepackbinder.cpp \
	../../../luabinding/tilemapbinder.cpp \
	../../../luabinding/timerbinder.cpp \
	../../../luabinding/urlloaderbinder.cpp \
	../../../luabinding/tlsf.c \
	../../../luabinding/geolocationbinder.cpp \
	../../../luabinding/gyroscopebinder.cpp \
	../../../luabinding/fontbasebinder.cpp \
	../../../luabinding/ttfontbinder.cpp \
	../../../luabinding/alertdialogbinder.cpp \
	../../../luabinding/textinputdialogbinder.cpp \
	../../../luabinding/meshbinder.cpp \
	../../../luabinding/audiobinder.cpp \
	../../../luabinding/zlibbinder.cpp \
	../../../luabinding/cryptobinder.cpp \
	../../../luabinding/shaderbinder.cpp \
	../../../luabinding/path2dbinder.cpp \
	../../../luabinding/viewportbinder.cpp \
	../../../luabinding/pixelbinder.cpp \
	../../../luabinding/particlesbinder.cpp \
	../../../luabinding/rendertargetbinder.cpp

#XMP
LOCAL_CFLAGS+=-D_REENTRANT -DLIBXMP_CORE_PLAYER
XMP_SRC=virtual period player read_event dataio lfo envelope \
		scan control filter effects mixer mix_all load_helpers load \
		hio smix memio
XMP_HDR=common effects envelope format lfo list mixer period player \
		virtual precomp_lut hio memio mdataio tempfile 
XMP_LOADERS=xm_load s3m_load it_load \
			common itsex sample
XMP_LOADERS_HDR=it loader mod s3m xm
LOCAL_SRC_FILES += $(addprefix ../../../libgid/external/libxmp-4.3/src/,$(addsuffix .c,$(XMP_SRC)))
LOCAL_SRC_FILES += $(addprefix ../../../libgid/external/libxmp-4.3/src/loaders/,$(addsuffix .c,$(XMP_LOADERS)))
LOCAL_SRC_FILES += \
	../../../libgid/external/libxmp-4.3/lite/src/format.c \
	../../../libgid/external/libxmp-4.3/lite/src/loaders/mod_load.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../libgid/external/libxmp-4.3/src \
					$(LOCAL_PATH)/../../../libgid/external/libxmp-4.3/src/loaders

LOCAL_LDLIBS := -lGLESv2 -ldl -llog -lOpenSLES

LOCAL_SHARED_LIBRARIES := gvfs lua
LOCAL_STATIC_LIBRARIES := openal

include $(BUILD_SHARED_LIBRARY)

ifeq ($(TARGET_ARCH),armeabi)
LOCAL_ARM_MODE := arm
endif

