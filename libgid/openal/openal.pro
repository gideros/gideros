CONFIG -= qt

TEMPLATE = lib
CONFIG += sharedlib
CONFIG += object_parallel_to_source
CONFIG += c++11

#OpenAL
LOCAL_OPENAL_VERSION=1.17.2
LOCAL_OPENAL_PATH=../external
INCLUDEPATH += \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/include \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/OpenAL32/Include \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/backends

# openal (24 files)
DEFINES += AL_BUILD_LIBRARY AL_ALEXT_PROTOTYPES OPENAL_SUBDIR_AL HAVE_DSOUND HAVE_WINMM
#openal Common FILES
SOURCES  += \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/OpenAL32/alAuxEffectSlot.c \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/OpenAL32/alBuffer.c        \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/OpenAL32/alEffect.c        \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/OpenAL32/alError.c         \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/OpenAL32/alExtension.c     \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/OpenAL32/alFilter.c        \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/OpenAL32/alListener.c      \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/OpenAL32/alSource.c        \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/OpenAL32/alState.c         \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/OpenAL32/alThunk.c         \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/ALc.c                  \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/alcConfig.c            \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/alcRing.c              \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/ALu.c                  \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/bs2b.c                 \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/mixer.c                \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/panning.c

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
SOURCES  += \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/bsinc.c                 \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/helpers.c                 \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/hrtf.c                 \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/mixer_c.c                 \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/effects/autowah.c       \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/effects/chorus.c       \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/effects/compressor.c       \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/effects/dedicated.c       \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/effects/distortion.c       \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/effects/echo.c       \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/effects/equalizer.c       \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/effects/flanger.c       \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/effects/modulator.c       \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/effects/null.c       \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/effects/reverb.c        \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/backends/base.c        \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/backends/dsound.c        \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/backends/winmm.c        \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/backends/null.c        \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/Alc/backends/loopback.c        \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/OpenAL32/sample_cvt.c         \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/common/atomic.c        \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/common/rwlock.c        \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/common/threads.c        \
        $${LOCAL_OPENAL_PATH}/openal-soft-$${LOCAL_OPENAL_VERSION}/common/uintmap.c

 LIBS+= -lwinmm -lole32 -ldsound -ldxguid -lks -lksuser
