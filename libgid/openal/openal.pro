CONFIG -= qt

TEMPLATE = lib
CONFIG += sharedlib
CONFIG += object_parallel_to_source
CONFIG += c++11

#OpenAL
LOCAL_OPENAL_VERSION=
LOCAL_OPENAL_PATH=../external
INCLUDEPATH += \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/include \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/common \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}

# openal (24 files)
DEFINES += AL_BUILD_LIBRARY AL_ALEXT_PROTOTYPES OPENAL_SUBDIR_AL HAVE_DSOUND HAVE_WINMM "alc_API=__declspec(dllexport)" "AL_API=__declspec(dllexport)" RESTRICT=__restrict
#openal Common FILES
SOURCES  += \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/auxEffectSlot.cpp \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/buffer.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effect.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/error.cpp         \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/event.cpp      \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/extension.cpp     \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/filter.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/listener.cpp      \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/source.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/state.cpp         \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/autowah.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/chorus.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/compressor.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/convolution.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/dedicated.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/distortion.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/echo.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/effects.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/equalizer.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/fshifter.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/modulator.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/null.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/pshifter.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/reverb.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/al/effects/vmorpher.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/alc.cpp                  \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/alconfig.cpp            \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/alu.cpp                  \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/context.cpp            \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/device.cpp            \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/panning.cpp            \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/effects/autowah.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/effects/chorus.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/effects/compressor.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/effects/convolution.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/effects/dedicated.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/effects/distortion.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/effects/echo.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/effects/equalizer.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/effects/fshifter.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/effects/modulator.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/effects/null.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/effects/pshifter.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/effects/reverb.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/effects/vmorpher.cpp       \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/backends/base.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/backends/dsound.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/backends/winmm.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/backends/null.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/alc/backends/loopback.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/common/alcomplex.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/common/alfstream.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/common/almalloc.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/common/alstring.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/common/dynload.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/common/polyphase_resampler.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/common/ringbuffer.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/common/strutils.cpp        \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/common/threads.cpp    \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/ambdec.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/ambidefs.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/bformatdec.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/bs2b.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/bsinc_tables.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/buffer_storage.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/context.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/converter.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/cpu_caps.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/devformat.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/device.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/effectslot.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/except.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/fmt_traits.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/fpu_ctrl.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/helpers.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/hrtf.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/logging.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/mastering.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/mixer.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/uhjfilter.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/uiddefs.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/voice.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/filters/biquad.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/filters/nfc.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/filters/splitter.cpp                 \
        $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/mixer/mixer_c.cpp

#       $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/rtkit.cpp                 \
#       $${LOCAL_OPENAL_PATH}/openal-soft$${LOCAL_OPENAL_VERSION}/core/dbus_wrap.cpp

 LIBS+= -lwinmm -lole32 -ldsound -ldxguid -lks -lksuser
