#include <gaudio.h>
#include "../ggaudiomanager.h"

#if defined(OPENAL_SUBDIR_OPENAL)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#elif defined(OPENAL_SUBDIR_AL)
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <al.h>
#include <alc.h>
#endif

extern "C" {
GGSampleInterface *GGSampleOpenALManagerCreate();
void GGSampleOpenALManagerDelete(GGSampleInterface *manager);

GGStreamInterface *GGStreamOpenALManagerCreate();
void GGStreamOpenALManagerDelete(GGStreamInterface *manager);

GGBackgroundMusicInterface *GGBackgroundMediaPlayerManagerCreate();
void GGBackgroundMediaPlayerManagerDelete(GGBackgroundMusicInterface *manager);
}

struct GGAudioSystemData
{
    ALCdevice *device;
    ALCcontext *context;
};

void GGAudioManager::systemInit()
{
    systemData_ = (GGAudioSystemData*)malloc(sizeof(GGAudioSystemData));

    systemData_->device = alcOpenDevice(NULL);

    int params[3];
    params[0] = ALC_FREQUENCY;
    params[1] = 22050;
    params[2] = 0;

    systemData_->context = alcCreateContext(systemData_->device, params);

    alcMakeContextCurrent(systemData_->context);
}

void GGAudioManager::systemCleanup()
{
    alcMakeContextCurrent(NULL);
    alcDestroyContext(systemData_->context);
    alcCloseDevice(systemData_->device);

    free(systemData_);
}

void GGAudioManager::createBackgroundMusicInterface()
{
    backgroundMusicInterface_ = GGBackgroundMediaPlayerManagerCreate();
}

void GGAudioManager::deleteBackgroundMusicInterface()
{
    GGBackgroundMediaPlayerManagerDelete(backgroundMusicInterface_);
}

void GGAudioManager::beginInterruption()
{
}

void GGAudioManager::endInterruption()
{
}

void GGSoundManager::interfacesInit()
{
    loaders_["wav"] = GGAudioLoader(gaudio_WavOpen, gaudio_WavClose, gaudio_WavRead, gaudio_WavSeek, gaudio_WavTell);

    sampleInterface_ = GGSampleOpenALManagerCreate();
    streamInterface_ = GGStreamOpenALManagerCreate();
}

void GGSoundManager::interfacesCleanup()
{
    GGSampleOpenALManagerDelete(sampleInterface_);
    GGStreamOpenALManagerDelete(streamInterface_);
}
