#include "ggaudiomanager.h"
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <ctype.h>
#include <stdio.h>

#ifdef WINSTORE
#include "winrt/wave.h"
#undef interface  // silly Microsoft!
#endif

GGSoundManager::GGSoundManager()
{
    interfacesInit();
}

GGSoundManager::~GGSoundManager()
{
    while (!sounds_.empty())
    {
        Sound *sound = sounds_.begin()->second;
        SoundDelete(sound->gid);
    }

    interfacesCleanup();
}

g_id GGSoundManager::SoundCreateFromFile(const char *fileName, bool stream, gaudio_Error *error)
{
    const char *dot = strrchr(fileName, '.');

    if (dot == NULL)
    {
        if (error)
            *error = GAUDIO_UNSUPPORTED_FORMAT;
        return 0;
    }

    std::string dot2 = dot + 1;
    std::transform(dot2.begin(), dot2.end(), dot2.begin(), ::tolower);

    std::map<std::string, GGAudioLoader>::iterator iter = loaders_.find(dot2);

    if (iter == loaders_.end())
    {
        if (error)
            *error = GAUDIO_UNSUPPORTED_FORMAT;
        return 0;
    }

    const GGAudioLoader &loader = iter->second;

    if (stream == false)
    {
        int numChannels, sampleRate, bitsPerSample, numSamples;
        g_id handle = loader.open(fileName, &numChannels, &sampleRate, &bitsPerSample, &numSamples, error);

        if (handle == 0)
            return 0;

        size_t size = numChannels * (bitsPerSample / 8) * numSamples;
        void *data = malloc(size);
        loader.read(handle, size, data);

        loader.close(handle);

        g_id sound = sampleInterface_->SoundCreateFromBuffer(data, numChannels, sampleRate, bitsPerSample, numSamples);

        free(data);

		sounds_[sound] = new Sound(sound, sampleInterface_);

        return sound;
    }
    else
    {
        g_id sound = streamInterface_->SoundCreateFromFile(fileName, loader, error);

        if (sound == 0)
            return 0;

        sounds_[sound] = new Sound(sound, streamInterface_);

        return sound;
    }
}

void GGSoundManager::SoundDelete(g_id sound)
{
    std::map<g_id, Sound*>::iterator iter = sounds_.find(sound);
    if (iter == sounds_.end())
        return;

    Sound *sound2 = iter->second;

    std::set<Channel*>::iterator iter2, e = sound2->channels.end();
    for (iter2 = sound2->channels.begin(); iter2 != e; ++iter2)
    {
        Channel *channel = *iter2;

        channel->interface->ChannelStop(channel->gid);

        channels_.erase(channel->gid);

        delete channel;
    }

    sound2->interface->SoundDelete(sound);

    delete sound2;

    sounds_.erase(iter);
}

unsigned int GGSoundManager::SoundGetLength(g_id sound)
{
    std::map<g_id, Sound*>::iterator iter = sounds_.find(sound);
    if (iter == sounds_.end())
        return 0;

    Sound *sound2 = iter->second;

    return sound2->interface->SoundGetLength(sound);
}

g_id GGSoundManager::SoundPlay(g_id sound, bool paused)
{
    std::map<g_id, Sound*>::iterator iter = sounds_.find(sound);
    if (iter == sounds_.end())
        return 0;

    Sound *sound2 = iter->second;

    g_id channel = sound2->interface->SoundPlay(sound, paused);

    Channel *channel2 = new Channel(channel, sound2, sound2->interface);

    sound2->channels.insert(channel2);

    channels_[channel] = channel2;

    return channel;
}

void GGSoundManager::SoundListener(float x,float y,float z,float vx,float vy,float vz,float dx,float dy,float dz,float ux,float uy,float uz)
{
	  sampleInterface_->SoundListener(x,y,z,vx,vy,vz,dx,dy,dz,ux,uy,uz);
	  streamInterface_->SoundListener(x,y,z,vx,vy,vz,dx,dy,dz,ux,uy,uz);
}


void GGSoundManager::ChannelStop(g_id channel)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelStop(channel);

    channel2->sound->channels.erase(channel2);

    delete channel2;

    channels_.erase(iter);
}

void GGSoundManager::ChannelSetPosition(g_id channel, unsigned int position)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelSetPosition(channel, position);
}

unsigned int GGSoundManager::ChannelGetPosition(g_id channel)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return 0;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelGetPosition(channel);
}

void GGSoundManager::ChannelSetPaused(g_id channel, bool paused)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelSetPaused(channel, paused);
}

bool GGSoundManager::ChannelIsPaused(g_id channel)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return false;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelIsPaused(channel);
}

bool GGSoundManager::ChannelIsPlaying(g_id channel)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return false;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelIsPlaying(channel);
}

void GGSoundManager::ChannelSetVolume(g_id channel, float volume)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelSetVolume(channel, volume);
}

float GGSoundManager::ChannelGetVolume(g_id channel)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return 0.f;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelGetVolume(channel);
}

void GGSoundManager::ChannelSetPitch(g_id channel, float pitch)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelSetPitch(channel, pitch);
}

float GGSoundManager::ChannelGetPitch(g_id channel)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return 0.f;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelGetPitch(channel);
}

void GGSoundManager::ChannelSetLooping(g_id channel, bool looping)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelSetLooping(channel, looping);
}

bool GGSoundManager::ChannelIsLooping(g_id channel)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return false;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelIsLooping(channel);
}

void GGSoundManager::ChannelSetWorldPosition(g_id channel, float x,float y,float z,float vx,float vy,float vz)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return ;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelSetWorldPosition(channel, x,y,z,vx,vy,vz);
}

g_id GGSoundManager::ChannelAddCallback(g_id channel, gevent_Callback callback, void *udata)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return 0;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelAddCallback(channel, callback, udata);
}

void GGSoundManager::ChannelRemoveCallback(g_id channel, gevent_Callback callback, void *udata)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelRemoveCallback(channel, callback, udata);
}

void GGSoundManager::ChannelRemoveCallbackWithGid(g_id channel, g_id gid)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelRemoveCallbackWithGid(channel, gid);
}

void GGSoundManager::preTick()
{
    sampleInterface_->preTick();
    streamInterface_->preTick();
}

void GGSoundManager::postTick()
{
    sampleInterface_->postTick();
    streamInterface_->postTick();

    std::map<g_id, Channel*>::iterator iter = channels_.begin(), end = channels_.end();
    while (iter != end)
    {
        Channel *channel2 = iter->second;

        g_bool valid = channel2->interface->ChannelIsValid(channel2->gid);

        if (!valid)
        {
            channel2->sound->channels.erase(channel2);
            delete channel2;
            channels_.erase(iter++);
        }
        else
        {
            ++iter;
        }
    }
}


GGAudioManager::GGAudioManager()
{
    systemInit();
    interrupted_ = false;

    soundManager_ = new GGSoundManager();
    createBackgroundMusicInterface();

    gevent_AddCallback(tick_s, this);
}

GGAudioManager::~GGAudioManager()
{
    gevent_RemoveCallback(tick_s, this);

    delete soundManager_;
    deleteBackgroundMusicInterface();

    systemCleanup();
}

g_id GGAudioManager::SoundCreateFromFile(const char *fileName, bool stream, gaudio_Error *error)
{
    return soundManager_->SoundCreateFromFile(fileName, stream, error);
}

void GGAudioManager::SoundDelete(g_id sound)
{
    soundManager_->SoundDelete(sound);
}

unsigned int GGAudioManager::SoundGetLength(g_id sound)
{
    return soundManager_->SoundGetLength(sound);
}

void GGAudioManager::SoundListener(float x,float y,float z,float vx,float vy,float vz,float dx,float dy,float dz,float ux,float uy,float uz)
{
	soundManager_->SoundListener(x,y,z,vx,vy,vz,dx,dy,dz,ux,uy,uz);
}

g_id GGAudioManager::SoundPlay(g_id sound, bool paused)
{
    return soundManager_->SoundPlay(sound, paused);
}

void GGAudioManager::ChannelStop(g_id channel)
{
    soundManager_->ChannelStop(channel);
}

void GGAudioManager::ChannelSetPosition(g_id channel, unsigned int position)
{
    soundManager_->ChannelSetPosition(channel, position);
}

unsigned int GGAudioManager::ChannelGetPosition(g_id channel)
{
    return soundManager_->ChannelGetPosition(channel);
}

void GGAudioManager::ChannelSetPaused(g_id channel, bool paused)
{
    soundManager_->ChannelSetPaused(channel, paused);
}

bool GGAudioManager::ChannelIsPaused(g_id channel)
{
    return soundManager_->ChannelIsPaused(channel);
}

bool GGAudioManager::ChannelIsPlaying(g_id channel)
{
    return soundManager_->ChannelIsPlaying(channel);
}

void GGAudioManager::ChannelSetVolume(g_id channel, float volume)
{
    soundManager_->ChannelSetVolume(channel, volume);
}

float GGAudioManager::ChannelGetVolume(g_id channel)
{
    return soundManager_->ChannelGetVolume(channel);
}

void GGAudioManager::ChannelSetPitch(g_id channel, float pitch)
{
    soundManager_->ChannelSetPitch(channel, pitch);
}

float GGAudioManager::ChannelGetPitch(g_id channel)
{
    return soundManager_->ChannelGetPitch(channel);
}

void GGAudioManager::ChannelSetLooping(g_id channel, bool looping)
{
    soundManager_->ChannelSetLooping(channel, looping);
}

bool GGAudioManager::ChannelIsLooping(g_id channel)
{
    return soundManager_->ChannelIsLooping(channel);
}

void GGAudioManager::ChannelSetWorldPosition(g_id channel, float x,float y,float z,float vx,float vy,float vz)
{
    return soundManager_->ChannelSetWorldPosition(channel,x,y,z,vx,vy,vz);
}

g_id GGAudioManager::ChannelAddCallback(g_id channel, gevent_Callback callback, void *udata)
{
    return soundManager_->ChannelAddCallback(channel, callback, udata);
}

void GGAudioManager::ChannelRemoveCallback(g_id channel, gevent_Callback callback, void *udata)
{
    soundManager_->ChannelRemoveCallback(channel, callback, udata);
}

void GGAudioManager::ChannelRemoveCallbackWithGid(g_id channel, g_id gid)
{
    soundManager_->ChannelRemoveCallbackWithGid(channel, gid);
}

g_bool GGAudioManager::BackgroundMusicIsAvailable()
{
    return backgroundMusicInterface_ != NULL;
}

g_id GGAudioManager::BackgroundMusicCreateFromFile(const char *fileName, gaudio_Error *error)
{
    if (backgroundMusicInterface_ == NULL)
        return 0;

    return backgroundMusicInterface_->BackgroundMusicCreateFromFile(fileName, error);
}

void GGAudioManager::BackgroundMusicDelete(g_id backgroundMusic)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundMusicDelete(backgroundMusic);
}

unsigned int GGAudioManager::BackgroundMusicGetLength(g_id backgroundMusic)
{
    if (backgroundMusicInterface_ == NULL)
        return 0;

    return backgroundMusicInterface_->BackgroundMusicGetLength(backgroundMusic);
}

g_id GGAudioManager::BackgroundMusicPlay(g_id backgroundMusic, bool paused)
{
    if (backgroundMusicInterface_ == NULL)
        return 0;

    return backgroundMusicInterface_->BackgroundMusicPlay(backgroundMusic, paused);
}

void GGAudioManager::BackgroundChannelStop(g_id backgroundChannel)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundChannelStop(backgroundChannel);
}

void GGAudioManager::BackgroundChannelSetPosition(g_id backgroundChannel, unsigned int position)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundChannelSetPosition(backgroundChannel, position);
}

unsigned int GGAudioManager::BackgroundChannelGetPosition(g_id backgroundChannel)
{
    if (backgroundMusicInterface_ == NULL)
        return 0;

    return backgroundMusicInterface_->BackgroundChannelGetPosition(backgroundChannel);
}

void GGAudioManager::BackgroundChannelSetPaused(g_id backgroundChannel, bool paused)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundChannelSetPaused(backgroundChannel, paused);
}

bool GGAudioManager::BackgroundChannelIsPaused(g_id backgroundChannel)
{
    if (backgroundMusicInterface_ == NULL)
        return false;

    return backgroundMusicInterface_->BackgroundChannelIsPaused(backgroundChannel);
}

bool GGAudioManager::BackgroundChannelIsPlaying(g_id backgroundChannel)
{
    if (backgroundMusicInterface_ == NULL)
        return false;

    return backgroundMusicInterface_->BackgroundChannelIsPlaying(backgroundChannel);
}

void GGAudioManager::BackgroundChannelSetVolume(g_id backgroundChannel, float volume)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundChannelSetVolume(backgroundChannel, volume);
}

float GGAudioManager::BackgroundChannelGetVolume(g_id backgroundChannel)
{
    if (backgroundMusicInterface_ == NULL)
        return 0.f;

    return backgroundMusicInterface_->BackgroundChannelGetVolume(backgroundChannel);
}

void GGAudioManager::BackgroundChannelSetLooping(g_id backgroundChannel, bool looping)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundChannelSetLooping(backgroundChannel, looping);
}

bool GGAudioManager::BackgroundChannelIsLooping(g_id backgroundChannel)
{
    if (backgroundMusicInterface_ == NULL)
        return false;

    return backgroundMusicInterface_->BackgroundChannelIsLooping(backgroundChannel);
}

g_id GGAudioManager::BackgroundChannelAddCallback(g_id backgroundChannel, gevent_Callback callback, void *udata)
{
    if (backgroundMusicInterface_ == NULL)
        return 0;

    return backgroundMusicInterface_->BackgroundChannelAddCallback(backgroundChannel, callback, udata);
}

void GGAudioManager::BackgroundChannelRemoveCallback(g_id backgroundChannel, gevent_Callback callback, void *udata)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundChannelRemoveCallback(backgroundChannel, callback, udata);
}

void GGAudioManager::BackgroundChannelRemoveCallbackWithGid(g_id backgroundChannel, g_id gid)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundChannelRemoveCallbackWithGid(backgroundChannel, gid);
}

void GGAudioManager::tick_s(int type, void *event, void *udata)
{
    GGAudioManager *manager = static_cast<GGAudioManager*>(udata);

    if (type == GEVENT_PRE_TICK_EVENT)
        manager->preTick();
    else if (type == GEVENT_POST_TICK_EVENT)
        manager->postTick();
}

void GGAudioManager::preTick()
{
    soundManager_->preTick();
    if (backgroundMusicInterface_)
        backgroundMusicInterface_->preTick();
}

void GGAudioManager::postTick()
{
    soundManager_->postTick();
    if (backgroundMusicInterface_)
        backgroundMusicInterface_->postTick();
}

static GGAudioManager *s_manager = NULL;

extern "C" {

void gaudio_Init()
{
    s_manager = new GGAudioManager();
}

void gaudio_Cleanup()
{
    delete s_manager;
    s_manager = NULL;
}


g_id gaudio_SoundCreateFromFile(const char *fileName, g_bool stream, gaudio_Error *error)
{
    return s_manager->SoundCreateFromFile(fileName, stream, error);
}

void gaudio_SoundDelete(g_id sound)
{
    s_manager->SoundDelete(sound);
}

unsigned int gaudio_SoundGetLength(g_id sound)
{
    return s_manager->SoundGetLength(sound);
}


g_id gaudio_SoundPlay(g_id sound, g_bool paused)
{
    return s_manager->SoundPlay(sound, paused);
}

void gaudio_SoundListener(float x,float y,float z,float vx,float vy,float vz,float dx,float dy,float dz,float ux,float uy,float uz)
{
	s_manager->SoundListener(x,y,z,vx,vy,vz,dx,dy,dz,ux,uy,uz);
}

void gaudio_ChannelStop(g_id channel)
{
    s_manager->ChannelStop(channel);
}

void gaudio_ChannelSetPosition(g_id channel, unsigned int position)
{
    s_manager->ChannelSetPosition(channel, position);
}

unsigned int gaudio_ChannelGetPosition(g_id channel)
{
    return s_manager->ChannelGetPosition(channel);
}

void gaudio_ChannelSetPaused(g_id channel, g_bool paused)
{
    s_manager->ChannelSetPaused(channel, paused);
}

g_bool gaudio_ChannelIsPaused(g_id channel)
{
    return s_manager->ChannelIsPaused(channel);
}

g_bool gaudio_ChannelIsPlaying(g_id channel)
{
    return s_manager->ChannelIsPlaying(channel);
}

void gaudio_ChannelSetVolume(g_id channel, float volume)
{
    s_manager->ChannelSetVolume(channel, volume);
}

float gaudio_ChannelGetVolume(g_id channel)
{
    return s_manager->ChannelGetVolume(channel);
}

void gaudio_ChannelSetPitch(g_id channel, float pitch)
{
    s_manager->ChannelSetPitch(channel, pitch);
}

float gaudio_ChannelGetPitch(g_id channel)
{
    return s_manager->ChannelGetPitch(channel);
}

void gaudio_ChannelSetLooping(g_id channel, g_bool looping)
{
    s_manager->ChannelSetLooping(channel, looping);
}

g_bool gaudio_ChannelIsLooping(g_id channel)
{
    return s_manager->ChannelIsLooping(channel);
}

void gaudio_ChannelSetWorldPosition(g_id channel, float x,float y,float z,float vx,float vy,float vz)
{
	s_manager->ChannelSetWorldPosition(channel, x,y,z,vx,vy,vz);
}

g_id gaudio_ChannelAddCallback(g_id channel, gevent_Callback callback, void *udata)
{
    return s_manager->ChannelAddCallback(channel, callback, udata);
}

void gaudio_ChannelRemoveCallback(g_id channel, gevent_Callback callback, void *udata)
{
    s_manager->ChannelRemoveCallback(channel, callback, udata);
}

void gaudio_ChannelRemoveCallbackWithGid(g_id channel, g_id gid)
{
    s_manager->ChannelRemoveCallbackWithGid(channel, gid);
}

g_bool gaudio_BackgroundMusicIsAvailable()
{
    return s_manager->BackgroundMusicIsAvailable();
}

g_id gaudio_BackgroundMusicCreateFromFile(const char *fileName, gaudio_Error *error)
{
    return s_manager->BackgroundMusicCreateFromFile(fileName, error);
}

void gaudio_BackgroundMusicDelete(g_id backgroundMusic)
{
    s_manager->BackgroundMusicDelete(backgroundMusic);
}

unsigned int gaudio_BackgroundMusicGetLength(g_id backgroundMusic)
{
    return s_manager->BackgroundMusicGetLength(backgroundMusic);
}

g_id gaudio_BackgroundMusicPlay(g_id backgroundMusic, g_bool paused)
{
    return s_manager->BackgroundMusicPlay(backgroundMusic, paused);
}

void gaudio_BackgroundChannelStop(g_id backgroundChannel)
{
    s_manager->BackgroundChannelStop(backgroundChannel);
}

void gaudio_BackgroundChannelSetPosition(g_id backgroundChannel, unsigned int position)
{
    s_manager->BackgroundChannelSetPosition(backgroundChannel, position);
}

unsigned int gaudio_BackgroundChannelGetPosition(g_id backgroundChannel)
{
    return s_manager->BackgroundChannelGetPosition(backgroundChannel);
}

void gaudio_BackgroundChannelSetPaused(g_id backgroundChannel, g_bool paused)
{
    s_manager->BackgroundChannelSetPaused(backgroundChannel, paused);
}

g_bool gaudio_BackgroundChannelIsPaused(g_id backgroundChannel)
{
    return s_manager->BackgroundChannelIsPaused(backgroundChannel);
}

g_bool gaudio_BackgroundChannelIsPlaying(g_id backgroundChannel)
{
    return s_manager->BackgroundChannelIsPlaying(backgroundChannel);
}

void gaudio_BackgroundChannelSetVolume(g_id backgroundChannel, float volume)
{
    s_manager->BackgroundChannelSetVolume(backgroundChannel, volume);
}

float gaudio_BackgroundChannelGetVolume(g_id backgroundChannel)
{
    return s_manager->BackgroundChannelGetVolume(backgroundChannel);
}

void gaudio_BackgroundChannelSetLooping(g_id backgroundChannel, g_bool looping)
{
    s_manager->BackgroundChannelSetLooping(backgroundChannel, looping);
}

g_bool gaudio_BackgroundChannelIsLooping(g_id backgroundChannel)
{
    return s_manager->BackgroundChannelIsLooping(backgroundChannel);
}

g_id gaudio_BackgroundChannelAddCallback(g_id backgroundChannel, gevent_Callback callback, void *udata)
{
    return s_manager->BackgroundChannelAddCallback(backgroundChannel, callback, udata);
}

void gaudio_BackgroundChannelRemoveCallback(g_id backgroundChannel, gevent_Callback callback, void *udata)
{
    s_manager->BackgroundChannelRemoveCallback(backgroundChannel, callback, udata);
}

void gaudio_BackgroundChannelRemoveCallbackWithGid(g_id backgroundChannel, g_id gid)
{
    s_manager->BackgroundChannelRemoveCallbackWithGid(backgroundChannel, gid);
}

void gaudio_AdvanceStreamBuffers()
{
	s_manager->AdvanceStreamBuffers();
}


}
