LOCAL_PATH := $(call my-dir)

####

include $(CLEAR_VARS)

LOCAL_MODULE            := gvfs
LOCAL_SRC_FILES         := ../../../libgvfs/libs/$(TARGET_ARCH_ABI)/libgvfs.so

include $(PREBUILT_SHARED_LIBRARY)

####

include $(CLEAR_VARS)

LOCAL_MODULE            := lua
LOCAL_SRC_FILES         := ../../../$(LUA_ENGINE)/libs/$(TARGET_ARCH_ABI)/liblua.so

include $(PREBUILT_SHARED_LIBRARY)

####

include $(CLEAR_VARS)
LOCAL_MODULE			:= openal

LOCAL_OPENAL_PATH:=../../../libgid/external/openal-soft

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/$(LOCAL_OPENAL_PATH)/include \
	$(LOCAL_PATH)/$(LOCAL_OPENAL_PATH)/core \
	$(LOCAL_PATH)/$(LOCAL_OPENAL_PATH)/common \
	$(LOCAL_PATH)/$(LOCAL_OPENAL_PATH)/alc \
	$(LOCAL_PATH)/$(LOCAL_OPENAL_PATH)

# openal (24 files)
LOCAL_CFLAGS += -DAL_BUILD_LIBRARY -DAL_ALEXT_PROTOTYPES -DOPENAL_SUBDIR_AL -DRESTRICT=__restrict -DHAVE_OPENSL 
LOCAL_CPPFLAGS = -std=c++14
#openal Common FILES
LOCAL_SRC_FILES  := \
        $(LOCAL_OPENAL_PATH)/al/auxeffectslot.cpp \
        $(LOCAL_OPENAL_PATH)/al/buffer.cpp        \
        $(LOCAL_OPENAL_PATH)/al/effect.cpp        \
        $(LOCAL_OPENAL_PATH)/al/error.cpp         \
        $(LOCAL_OPENAL_PATH)/al/event.cpp      \
        $(LOCAL_OPENAL_PATH)/al/extension.cpp     \
        $(LOCAL_OPENAL_PATH)/al/filter.cpp        \
        $(LOCAL_OPENAL_PATH)/al/listener.cpp      \
        $(LOCAL_OPENAL_PATH)/al/source.cpp        \
        $(LOCAL_OPENAL_PATH)/al/state.cpp         \
        $(LOCAL_OPENAL_PATH)/al/effects/autowah.cpp       \
        $(LOCAL_OPENAL_PATH)/al/effects/chorus.cpp       \
        $(LOCAL_OPENAL_PATH)/al/effects/compressor.cpp       \
        $(LOCAL_OPENAL_PATH)/al/effects/convolution.cpp       \
        $(LOCAL_OPENAL_PATH)/al/effects/dedicated.cpp       \
        $(LOCAL_OPENAL_PATH)/al/effects/distortion.cpp       \
        $(LOCAL_OPENAL_PATH)/al/effects/echo.cpp       \
        $(LOCAL_OPENAL_PATH)/al/effects/effects.cpp       \
        $(LOCAL_OPENAL_PATH)/al/effects/equalizer.cpp       \
        $(LOCAL_OPENAL_PATH)/al/effects/fshifter.cpp       \
        $(LOCAL_OPENAL_PATH)/al/effects/modulator.cpp       \
        $(LOCAL_OPENAL_PATH)/al/effects/null.cpp       \
        $(LOCAL_OPENAL_PATH)/al/effects/pshifter.cpp       \
        $(LOCAL_OPENAL_PATH)/al/effects/reverb.cpp        \
        $(LOCAL_OPENAL_PATH)/al/effects/vmorpher.cpp       \
        $(LOCAL_OPENAL_PATH)/alc/alc.cpp                  \
        $(LOCAL_OPENAL_PATH)/alc/alconfig.cpp            \
        $(LOCAL_OPENAL_PATH)/alc/alu.cpp                  \
        $(LOCAL_OPENAL_PATH)/alc/context.cpp            \
        $(LOCAL_OPENAL_PATH)/alc/device.cpp            \
        $(LOCAL_OPENAL_PATH)/alc/panning.cpp            \
        $(LOCAL_OPENAL_PATH)/alc/effects/autowah.cpp       \
        $(LOCAL_OPENAL_PATH)/alc/effects/chorus.cpp       \
        $(LOCAL_OPENAL_PATH)/alc/effects/compressor.cpp       \
        $(LOCAL_OPENAL_PATH)/alc/effects/convolution.cpp       \
        $(LOCAL_OPENAL_PATH)/alc/effects/dedicated.cpp       \
        $(LOCAL_OPENAL_PATH)/alc/effects/distortion.cpp       \
        $(LOCAL_OPENAL_PATH)/alc/effects/echo.cpp       \
        $(LOCAL_OPENAL_PATH)/alc/effects/equalizer.cpp       \
        $(LOCAL_OPENAL_PATH)/alc/effects/fshifter.cpp       \
        $(LOCAL_OPENAL_PATH)/alc/effects/modulator.cpp       \
        $(LOCAL_OPENAL_PATH)/alc/effects/null.cpp       \
        $(LOCAL_OPENAL_PATH)/alc/effects/pshifter.cpp       \
        $(LOCAL_OPENAL_PATH)/alc/effects/reverb.cpp        \
        $(LOCAL_OPENAL_PATH)/alc/effects/vmorpher.cpp       \
        $(LOCAL_OPENAL_PATH)/alc/backends/base.cpp        \
        $(LOCAL_OPENAL_PATH)/alc/backends/opensl.cpp        \
        $(LOCAL_OPENAL_PATH)/alc/backends/null.cpp        \
        $(LOCAL_OPENAL_PATH)/alc/backends/loopback.cpp        \
        $(LOCAL_OPENAL_PATH)/common/alcomplex.cpp        \
        $(LOCAL_OPENAL_PATH)/common/alfstream.cpp        \
        $(LOCAL_OPENAL_PATH)/common/almalloc.cpp        \
        $(LOCAL_OPENAL_PATH)/common/alstring.cpp        \
        $(LOCAL_OPENAL_PATH)/common/dynload.cpp        \
        $(LOCAL_OPENAL_PATH)/common/polyphase_resampler.cpp        \
        $(LOCAL_OPENAL_PATH)/common/ringbuffer.cpp        \
        $(LOCAL_OPENAL_PATH)/common/strutils.cpp        \
        $(LOCAL_OPENAL_PATH)/common/threads.cpp    \
        $(LOCAL_OPENAL_PATH)/core/ambdec.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/ambidefs.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/bformatdec.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/bs2b.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/bsinc_tables.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/buffer_storage.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/context.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/converter.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/cpu_caps.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/devformat.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/device.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/effectslot.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/except.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/fmt_traits.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/fpu_ctrl.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/helpers.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/hrtf.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/logging.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/mastering.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/mixer.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/uhjfilter.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/uiddefs.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/voice.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/filters/biquad.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/filters/nfc.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/filters/splitter.cpp                 \
        $(LOCAL_OPENAL_PATH)/core/mixer/mixer_c.cpp

include $(BUILD_STATIC_LIBRARY)

#### MP3

include $(CLEAR_VARS)

LOCAL_MODULE            := mpg123
LOCAL_SRC_FILES += \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/compat.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/dct64.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/dither.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/equalizer.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/feature.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/format.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/frame.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/icy.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/icy2utf8.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/id3.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/index.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/layer1.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/layer2.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/layer3.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/libmpg123.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/ntom.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/optimize.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/parse.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/readers.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/stringbuf.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/synth.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/synth_8bit.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/synth_real.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/synth_s32.c \
	../../../libgid/external/mpg123-1.15.3/src/libmpg123/tabinit.c

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../../../libgid/external/mpg123-1.15.3/src \
	$(LOCAL_PATH)/../../../libgid/external/mpg123-1.15.3/src/libmpg123 \
	$(LOCAL_PATH)/../../../libgvfs

LOCAL_CFLAGS+=-DOPT_GENERIC -DREAL_IS_FLOAT

include $(BUILD_STATIC_LIBRARY)

##### XMP
include $(CLEAR_VARS)

LOCAL_MODULE            := libxmp
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
					$(LOCAL_PATH)/../../../libgid/external/libxmp-4.3/src/loaders \
					$(LOCAL_PATH)/../../../libgid/external/libxmp-4.3/include \
					$(LOCAL_PATH)/../../../libgvfs

include $(BUILD_STATIC_LIBRARY)

ifneq ($(OCULUS),)
include $(CLEAR_VARS)
LOCAL_MODULE := vrapi-prebuilt
LOCAL_SRC_FILES := $(LOCAL_PATH)/../oculus/Libs/$(TARGET_ARCH_ABI)/Release/libvrapi.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := xrloader-prebuilt
LOCAL_SRC_FILES := $(LOCAL_PATH)/../oculus/Libs/$(TARGET_ARCH_ABI)/Release/libopenxr_loader.so
include $(PREBUILT_SHARED_LIBRARY)
endif

include $(CLEAR_VARS)

ifneq ($(OCULUS),)
LOCAL_MODULE := giderosvr
else
LOCAL_MODULE := gideros
endif

LOCAL_CFLAGS := -O2

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES

LOCAL_CFLAGS += -DAL_BUILD_LIBRARY -DAL_ALEXT_PROTOTYPES -DOPENAL_SUBDIR_AL 

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../../../2dsg \
	$(LOCAL_PATH)/../../../2dsg/paths \
	$(LOCAL_PATH)/../../../libnetwork \
	$(LOCAL_PATH)/../../../external/glu \
	$(LOCAL_PATH)/../../../libpvrt \
	$(LOCAL_PATH)/../../../$(LUA_INCLUDE) \
	$(addprefix $(LOCAL_PATH)/../../../,$(LUA_INCLUDE_CORE)) \
	$(LOCAL_PATH)/../../../luabinding \
	$(LOCAL_PATH)/../../../libgid/include \
	$(LOCAL_PATH)/../../../libgid/include/private \
	$(LOCAL_PATH)/../../../libgid/include/android \
	$(LOCAL_PATH)/../../../libgid/external/snappy-1.1.0 \
	$(LOCAL_PATH)/../../../libgvfs \
	$(LOCAL_PATH)/../../../libgideros \
	$(LOCAL_PATH)/../../../libpystring \
	$(LOCAL_PATH)/$(LOCAL_OPENAL_PATH)/include \
	$(LOCAL_PATH)/$(LOCAL_OPENAL_PATH)/OpenAL32/Include \
	$(LOCAL_PATH)/$(LOCAL_OPENAL_PATH)/include \
	$(LOCAL_PATH)/$(LOCAL_OPENAL_PATH)/Alc \
	$(LOCAL_PATH)/$(LOCAL_OPENAL_PATH)/Alc/backends \
	$(LOCAL_PATH)/../../../libgid/external/libxmp-4.3/include \
	$(LOCAL_PATH)/../../../libgid/external/mpg123-1.15.3/src \
	$(LOCAL_PATH)/../../../libgid/external/mpg123-1.15.3/src/libmpg123 \
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
	../../../libgid/src/gaudio-loader-mp3.cpp \
	../../../libgid/src/gaudio-sample-openal.cpp \
	../../../libgid/src/gaudio-stream-openal.cpp \
    ../../../libgid/src/android/ginput-android.cpp \
    ../../../libgid/src/gvfs-native.cpp \
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
#FREETYPE_VER=2.4.12
FREETYPE_VER=2.7.1
LOCAL_CFLAGS += -DFT2_BUILD_LIBRARY
LOCAL_CFLAGS += -DDARWIN_NO_CARBON
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../../../libgid/external/freetype-$(FREETYPE_VER)/include \
	$(LOCAL_PATH)/../../../libgid/external/freetype-$(FREETYPE_VER)/src
LOCAL_SRC_FILES += \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/base/ftbbox.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/base/ftbitmap.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/base/ftglyph.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/base/ftlcdfil.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/base/ftstroke.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/base/ftbase.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/base/ftsystem.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/base/ftinit.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/base/ftgasp.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/raster/raster.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/sfnt/sfnt.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/smooth/smooth.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/autofit/autofit.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/truetype/truetype.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/cff/cff.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/gzip/ftgzip.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/psnames/psnames.c \
    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/pshinter/pshinter.c
#2.4.12    ../../../libgid/external/freetype-$(FREETYPE_VER)/src/base/ftxf86.c

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
    ../../../2dsg/screen.cpp \
	../../../2dsg/pixel.cpp \
	../../../2dsg/particles.cpp \
	../../../2dsg/layoutevent.cpp \
	../../../2dsg/gridbaglayout.cpp \
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

# luabinding (35 files)
LOCAL_SRC_FILES += \
	../../../luabinding/accelerometerbinder.cpp \
	../../../luabinding/applicationbinder.cpp \
	../../../luabinding/binder.cpp \
	../../../luabinding/bitmapbinder.cpp \
	../../../luabinding/bitmapdatabinder.cpp \
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
	../../../luabinding/gmathbinder.cpp \
	../../../luabinding/cryptobinder.cpp \
	../../../luabinding/shaderbinder.cpp \
	../../../luabinding/path2dbinder.cpp \
	../../../luabinding/viewportbinder.cpp \
	../../../luabinding/pixelbinder.cpp \
	../../../luabinding/screenbinder.cpp \
	../../../luabinding/particlesbinder.cpp \
	../../../luabinding/debugging.cpp \
	../../../luabinding/bufferbinder.cpp \
	../../../luabinding/rendertargetbinder.cpp

LOCAL_LDLIBS := -lGLESv3 -ldl -llog -lOpenSLES

LOCAL_SHARED_LIBRARIES := gvfs lua
LOCAL_STATIC_LIBRARIES := openal mpg123 libxmp

ifneq ($(OCULUS),)
LOCAL_LDLIBS += -lEGL -landroid 
LOCAL_SRC_FILES += oculus/oculus.cpp \
	oculus/basexr/graphicsplugin_factory.cpp \
	oculus/basexr/graphicsplugin_opengles.cpp \
	oculus/basexr/logger.cpp \
	oculus/basexr/openxr_program.cpp \
	oculus/basexr/platformplugin_factory.cpp \
	oculus/basexr/platformplugin_android.cpp \
	oculus/basexr/VRExtension.cpp \
	oculus/basexr/ext/PassthroughFB.cpp \
	oculus/basexr/ext/SceneFB.cpp \
	oculus/basexr/ext/HandTrackingFB.cpp
	 
LOCAL_CFLAGS += -DOCULUS 
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/oculus \
	$(LOCAL_PATH)/oculus/Include \
	$(LOCAL_PATH)/oculus/basexr 
LOCAL_SHARED_LIBRARIES+=xrloader-prebuilt
endif

include $(BUILD_SHARED_LIBRARY)

ifeq ($(TARGET_ARCH),armeabi)
LOCAL_ARM_MODE := arm
endif

