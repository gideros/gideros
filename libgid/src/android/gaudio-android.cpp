#include <gaudio.h>
#include "../ggaudiomanager.h"

#if defined(OPENAL_SUBDIR_OPENAL)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <OpenAL/alext.h>
#elif defined(OPENAL_SUBDIR_AL)
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#else
#include <al.h>
#include <alc.h>
#include <alext.h>
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

struct GGAudioSystemData *OpenALData;

void gaudio_android_suspend(bool suspend)
{
	if (suspend)
		alcDevicePauseSOFT(OpenALData->device);
	else
		alcDeviceResumeSOFT(OpenALData->device);
}

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
    OpenALData=systemData_;
}

void GGAudioManager::systemCleanup()
{
	OpenALData=NULL;
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
    loaders_["mod"] = GGAudioLoader(gaudio_XmpOpen, gaudio_XmpClose, gaudio_XmpRead, gaudio_XmpSeek, gaudio_XmpTell);
    loaders_["xm"] = GGAudioLoader(gaudio_XmpOpen, gaudio_XmpClose, gaudio_XmpRead, gaudio_XmpSeek, gaudio_XmpTell);
    loaders_["it"] = GGAudioLoader(gaudio_XmpOpen, gaudio_XmpClose, gaudio_XmpRead, gaudio_XmpSeek, gaudio_XmpTell);
    loaders_["s3m"] = GGAudioLoader(gaudio_XmpOpen, gaudio_XmpClose, gaudio_XmpRead, gaudio_XmpSeek, gaudio_XmpTell);

    sampleInterface_ = GGSampleOpenALManagerCreate();
    streamInterface_ = GGStreamOpenALManagerCreate();
}

void GGSoundManager::interfacesCleanup()
{
    GGSampleOpenALManagerDelete(sampleInterface_);
    GGStreamOpenALManagerDelete(streamInterface_);
}

void GGAudioManager::AdvanceStreamBuffers()
{

}

void GGSoundManager::AdvanceStreamBuffers()
{

}
