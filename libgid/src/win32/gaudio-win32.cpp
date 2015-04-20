#include <gaudio.h>
#include <mpg123.h>
#include "../ggaudiomanager.h"

extern "C" {
GGSampleInterface *GGSampleXAudio2ManagerCreate();
void GGSampleXAudio2ManagerDelete(GGSampleInterface *manager);

GGStreamInterface *GGStreamXAudio2ManagerCreate();
void GGStreamXAudio2ManagerDelete(GGStreamInterface *manager);
}

void GGAudioManager::systemInit()
{
    mpg123_init();
}

void GGAudioManager::systemCleanup()
{
    mpg123_exit();
}

void GGAudioManager::createBackgroundMusicInterface()
{
    backgroundMusicInterface_ = NULL;
}

void GGAudioManager::deleteBackgroundMusicInterface()
{
}

void GGAudioManager::beginInterruption()
{
}

void GGAudioManager::endInterruption()
{
}

void GGAudioManager::AdvanceStreamBuffers()
{
	soundManager_->AdvanceStreamBuffers();
}

void GGSoundManager::interfacesInit()
{
    loaders_["wav"] = GGAudioLoader(gaudio_WavOpen, gaudio_WavClose, gaudio_WavRead, gaudio_WavSeek, gaudio_WavTell);
    loaders_["mp3"] = GGAudioLoader(gaudio_Mp3Open, gaudio_Mp3Close, gaudio_Mp3Read, gaudio_Mp3Seek, gaudio_Mp3Tell);

    sampleInterface_ = GGSampleXAudio2ManagerCreate();
    streamInterface_ = GGStreamXAudio2ManagerCreate();
}

void GGSoundManager::interfacesCleanup()
{
    GGSampleXAudio2ManagerDelete(sampleInterface_);
    GGStreamXAudio2ManagerDelete(streamInterface_);
}

void GGSoundManager::AdvanceStreamBuffers()
{
	streamInterface_->AdvanceStreamBuffers();
}
