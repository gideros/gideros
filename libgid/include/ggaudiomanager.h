#ifndef GGAUDIOMANAGER_H
#define GGAUDIOMANAGER_H

#include <gaudio.h>
#include <map>
#include <set>
#include <string>

class GGBackgroundMusicInterface
{
public:
    virtual ~GGBackgroundMusicInterface() {}

    virtual g_id BackgroundMusicCreateFromFile(const char *fileName, gaudio_Error *error) = 0;
    virtual void BackgroundMusicDelete(g_id backgroundMusic) = 0;
    virtual unsigned int BackgroundMusicGetLength(g_id backgroundMusic) = 0;

    virtual g_id BackgroundMusicPlay(g_id backgroudMusic, bool paused) = 0;
    virtual void BackgroundChannelStop(g_id backgroundChannel) = 0;
    virtual void BackgroundChannelSetPosition(g_id backgroundChannel, unsigned int position) = 0;
    virtual unsigned int BackgroundChannelGetPosition(g_id backgroundChannel) = 0;
    virtual void BackgroundChannelSetPaused(g_id backgroundChannel, bool paused) = 0;
    virtual bool BackgroundChannelIsPaused(g_id backgroundChannel) = 0;
    virtual bool BackgroundChannelIsPlaying(g_id backgroundChannel) = 0;
    virtual void BackgroundChannelSetVolume(g_id backgroundChannel, float volume) = 0;
    virtual float BackgroundChannelGetVolume(g_id backgroundChannel) = 0;
    virtual void BackgroundChannelSetLooping(g_id backgroundChannel, bool looping) = 0;
    virtual bool BackgroundChannelIsLooping(g_id backgroundChannel) = 0;
    virtual g_id BackgroundChannelAddCallback(g_id channel, gevent_Callback callback, void *udata) = 0;
    virtual void BackgroundChannelRemoveCallback(g_id channel, gevent_Callback callback, void *udata) = 0;
    virtual void BackgroundChannelRemoveCallbackWithGid(g_id channel, g_id gid) = 0;

    virtual void preTick() = 0;
    virtual void postTick() = 0;
};

class GGSoundInterface
{
public:
    virtual ~GGSoundInterface() {}

    virtual void SoundDelete(g_id sound) = 0;
    virtual unsigned int SoundGetLength(g_id sound) = 0;
    virtual void SoundListener(float x,float y,float z,float vx,float vy,float vz,float dx,float dy,float dz,float ux,float uy,float uz) { };
    virtual g_id SoundPlay(g_id sound, bool paused) = 0;

    virtual void ChannelStop(g_id channel) = 0;
    virtual void ChannelSetPosition(g_id channel, unsigned int position) = 0;
    virtual unsigned int ChannelGetPosition(g_id channel) = 0;
    virtual void ChannelSetPaused(g_id channel, bool paused) = 0;
    virtual bool ChannelIsPaused(g_id channel) = 0;
    virtual bool ChannelIsPlaying(g_id channel) = 0;
    virtual void ChannelSetVolume(g_id channel, float volume) = 0;
    virtual float ChannelGetVolume(g_id channel) = 0;
    virtual void ChannelSetPitch(g_id channel, float pitch) = 0;
    virtual float ChannelGetPitch(g_id channel) = 0;
    virtual void ChannelSetLooping(g_id channel, bool looping) = 0;
    virtual bool ChannelIsLooping(g_id channel) = 0;
    virtual g_id ChannelAddCallback(g_id channel, gevent_Callback callback, void *udata) = 0;
    virtual void ChannelRemoveCallback(g_id channel, gevent_Callback callback, void *udata) = 0;
    virtual void ChannelRemoveCallbackWithGid(g_id channel, g_id gid) = 0;
    virtual g_bool ChannelIsValid(g_id channel) = 0;
    virtual void ChannelSetWorldPosition(g_id channel, float x, float y, float z, float vx,float vy,float vz) { };

    virtual void preTick() = 0;
    virtual void postTick() = 0;
};

struct GGAudioLoader;

class GGSampleInterface : public GGSoundInterface
{
public:
    virtual ~GGSampleInterface() {}

    virtual g_id SoundCreateFromBuffer(const void *data, int numChannels, int sampleRate, int bitsPerSample, int numSamples) = 0;
};

class GGStreamInterface : public GGSoundInterface
{
public:
    virtual ~GGStreamInterface() {}

    virtual g_id SoundCreateFromFile(const char *fileName, const GGAudioLoader& loader, gaudio_Error *error) = 0;
	virtual void AdvanceStreamBuffers() {}
};

struct GGAudioLoader
{
    typedef g_id (*OpenFunc)(const char *fileName, int *numChannels, int *sampleRate, int *bitsPerSample, int *numSamples, gaudio_Error *error);
    typedef void (*CloseFunc)(g_id id);
    typedef size_t (*ReadFunc)(g_id id, size_t size, void *data);
    typedef int (*SeekFunc)(g_id id, long int offset, int whence);
    typedef long int (*TellFunc)(g_id id);

    GGAudioLoader()
    {
    }

    GGAudioLoader(OpenFunc open, CloseFunc close, ReadFunc read, SeekFunc seek, TellFunc tell) :
        open(open),
        close(close),
        read(read),
        seek(seek),
        tell(tell)
    {

    }

    OpenFunc open;
    CloseFunc close;
    ReadFunc read;
    SeekFunc seek;
    TellFunc tell;
};


class GGSoundManager
{
public:
    GGSoundManager();
    ~GGSoundManager();

    g_id SoundCreateFromFile(const char *fileName, bool stream, gaudio_Error *error);
    void SoundDelete(g_id sound);
    unsigned int SoundGetLength(g_id sound);
    void SoundListener(float x,float y,float z,float vx,float vy,float vz,float dx,float dy,float dz,float ux,float uy,float uz);
    g_id SoundPlay(g_id sound, bool paused);

    void ChannelStop(g_id channel);
    void ChannelSetPosition(g_id channel, unsigned int position);
    unsigned int ChannelGetPosition(g_id channel);
    void ChannelSetPaused(g_id channel, bool paused);
    bool ChannelIsPaused(g_id channel);
    bool ChannelIsPlaying(g_id channel);
    void ChannelSetVolume(g_id channel, float volume);
    float ChannelGetVolume(g_id channel);
    void ChannelSetPitch(g_id channel, float pitch);
    float ChannelGetPitch(g_id channel);
    void ChannelSetLooping(g_id channel, bool looping);
    bool ChannelIsLooping(g_id channel);
    void ChannelSetWorldPosition(g_id channel, float x,float y,float z,float vx,float vy,float vz);
    g_id ChannelAddCallback(g_id channel, gevent_Callback callback, void *udata);
    void ChannelRemoveCallback(g_id channel, gevent_Callback callback, void *udata);
    void ChannelRemoveCallbackWithGid(g_id channel, g_id gid);

    void preTick();
    void postTick();

	void AdvanceStreamBuffers();

private:
    void interfacesInit();
    void interfacesCleanup();

private:
    GGSampleInterface *sampleInterface_;
    GGStreamInterface *streamInterface_;
    std::map<std::string, GGAudioLoader> loaders_;

    struct Channel;

    struct Sound
    {
        Sound(g_id gid, GGSoundInterface *interface) :
            gid(gid),
            interface(interface)
        {

        }

        g_id gid;
        GGSoundInterface *interface;
        std::set<Channel*> channels;
    };

    struct Channel
    {
        Channel(g_id gid, Sound *sound, GGSoundInterface *interface) :
            gid(gid),
            sound(sound),
            interface(interface)
        {

        }

        g_id gid;
        Sound *sound;
        GGSoundInterface *interface;
    };

    std::map<g_id, Sound*> sounds_;
    std::map<g_id, Channel*> channels_;
};

struct GGAudioSystemData;

class GGAudioManager
{
public:
    GGAudioManager();
    ~GGAudioManager();

    g_id SoundCreateFromFile(const char *fileName, bool stream, gaudio_Error *error);
    void SoundDelete(g_id sound);
    unsigned int SoundGetLength(g_id sound);
    void SoundListener(float x,float y,float z,float vx,float vy,float vz,float dx,float dy,float dz,float ux,float uy,float uz);
    g_id SoundPlay(g_id sound, bool paused);

    void ChannelStop(g_id channel);
    void ChannelSetPosition(g_id channel, unsigned int position);
    unsigned int ChannelGetPosition(g_id channel);
    void ChannelSetPaused(g_id channel, bool paused);
    bool ChannelIsPaused(g_id channel);
    bool ChannelIsPlaying(g_id channel);
    void ChannelSetVolume(g_id channel, float volume);
    float ChannelGetVolume(g_id channel);
    void ChannelSetPitch(g_id channel, float pitch);
    float ChannelGetPitch(g_id channel);
    void ChannelSetLooping(g_id channel, bool looping);
    bool ChannelIsLooping(g_id channel);
    void ChannelSetWorldPosition(g_id channel, float x,float y,float z,float vx,float vy,float vz);
    g_id ChannelAddCallback(g_id channel, gevent_Callback callback, void *udata);
    void ChannelRemoveCallback(g_id channel, gevent_Callback callback, void *udata);
    void ChannelRemoveCallbackWithGid(g_id channel, g_id gid);

    g_bool BackgroundMusicIsAvailable();

    g_id BackgroundMusicCreateFromFile(const char *fileName, gaudio_Error *error);
    void BackgroundMusicDelete(g_id backgroundMusic);
    unsigned int BackgroundMusicGetLength(g_id backgroundMusic);

    g_id BackgroundMusicPlay(g_id backgroundMusic, bool paused);
    void BackgroundChannelStop(g_id backgroundChannel);
    void BackgroundChannelSetPosition(g_id backgroundChannel, unsigned int position);
    unsigned int BackgroundChannelGetPosition(g_id backgroundChannel);
    void BackgroundChannelSetPaused(g_id backgroundChannel, bool paused);
    bool BackgroundChannelIsPaused(g_id backgroundChannel);
    bool BackgroundChannelIsPlaying(g_id backgroundChannel);
    void BackgroundChannelSetVolume(g_id backgroundChannel, float volume);
    float BackgroundChannelGetVolume(g_id backgroundChannel);
    void BackgroundChannelSetLooping(g_id backgroundChannel, bool looping);
    bool BackgroundChannelIsLooping(g_id backgroundChannel);
    g_id BackgroundChannelAddCallback(g_id backgroundChannel, gevent_Callback callback, void *udata);
    void BackgroundChannelRemoveCallback(g_id backgroundChannel, gevent_Callback callback, void *udata);
    void BackgroundChannelRemoveCallbackWithGid(g_id backgroundChannel, g_id gid);

    void beginInterruption();
    void endInterruption();

	void AdvanceStreamBuffers();

private:
    static void tick_s(int type, void *event, void *udata);
    void preTick();
    void postTick();

private:
    void systemInit();
    void systemCleanup();
    GGAudioSystemData *systemData_;
    void createBackgroundMusicInterface();
    void deleteBackgroundMusicInterface();
    bool interrupted_;

private:
    GGSoundManager *soundManager_;
    GGBackgroundMusicInterface *backgroundMusicInterface_;
};

#endif
