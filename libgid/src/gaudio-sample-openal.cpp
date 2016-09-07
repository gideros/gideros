#include <gaudio.h>

#include "ggaudiomanager.h"

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

#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <set>

class GGSampleOpenALManager : public GGSampleInterface
{
public:
    GGSampleOpenALManager()
    {
    }

    ~GGSampleOpenALManager()
    {
        while (!sounds_.empty())
        {
            Sound *sound = sounds_.begin()->second;
            SoundDelete(sound->gid);
        }
    }

    g_id SoundCreateFromBuffer(const void *data, int numChannels, int sampleRate, int bitsPerSample, int numSamples)
    {
        ALuint buffer;
        alGenBuffers(1, &buffer);

        ALenum format = 0;
        if (bitsPerSample == 8)
        {
            if (numChannels == 1)
                format = AL_FORMAT_MONO8;
            else if (numChannels == 2)
                format = AL_FORMAT_STEREO8;
        }
        else if (bitsPerSample == 16)
        {
            if (numChannels == 1)
                format = AL_FORMAT_MONO16;
            else if (numChannels == 2)
                format = AL_FORMAT_STEREO16;
        }

        alBufferData(buffer, format, data, numSamples * numChannels * (bitsPerSample / 8), sampleRate);

        g_id gid = g_NextId();
        sounds_[gid] = new Sound(gid, buffer, (numSamples * 1000LL) / sampleRate);

        return gid;
    }

    void SoundDelete(g_id sound)
    {
        std::map<g_id, Sound*>::iterator iter = sounds_.find(sound);
        if (iter == sounds_.end())
            return;

        Sound *sound2 = iter->second;

        std::set<Channel*>::iterator iter2, e = sound2->channels.end();
        for (iter2 = sound2->channels.begin(); iter2 != e; ++iter2)
        {
            Channel *channel = *iter2;

            if (channel->source != 0)
            {
                alSourceStop(channel->source);
                alDeleteSources(1, &channel->source);
            }

            channels_.erase(channel->gid);

            gevent_RemoveEventsWithGid(channel->gid);

            delete channel;
        }

        alDeleteBuffers(1, &sound2->buffer);

        delete sound2;

        sounds_.erase(iter);
    }

    unsigned int SoundGetLength(g_id sound)
    {
        std::map<g_id, Sound*>::iterator iter = sounds_.find(sound);
        if (iter == sounds_.end())
            return 0;

        Sound *sound2 = iter->second;

        return sound2->length;
    }

    void SoundListener(float x,float y,float z,float vx,float vy,float vz,float dx,float dy,float dz,float ux,float uy,float uz)
    {
    	   alListener3f( AL_POSITION, x,y,z );
    	   alListener3f( AL_VELOCITY, vx,vy,vz );
    	   float orient[6] = { dx,dy,dz,ux,uy,uz };
    	   alListenerfv( AL_ORIENTATION, orient );
    }

    g_id SoundPlay(g_id sound, bool paused)
    {
        std::map<g_id, Sound*>::iterator iter = sounds_.find(sound);
        if (iter == sounds_.end())
            return 0;

        if (channels_.size() >= 31)
            return 0;

        Sound *sound2 = iter->second;

        ALuint source;
        alGetError();
        alGenSources(1, &source);
        if(alGetError() != AL_NO_ERROR)
            return 0;

        alSourcei(source, AL_BUFFER, sound2->buffer);

        g_id gid = g_NextId();

        Channel *channel = new Channel(gid, sound2, source);

        sound2->channels.insert(channel);

        channels_[gid] = channel;

        channel->paused = paused;
        if (!paused)
            alSourcePlay(channel->source);

        return gid;
    }

    void ChannelStop(g_id channel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        if (channel2->source != 0)
        {
            alSourceStop(channel2->source);
            alDeleteSources(1, &channel2->source);
        }

        channel2->sound->channels.erase(channel2);

        gevent_RemoveEventsWithGid(channel2->gid);

        delete channel2;

        channels_.erase(iter);
    }

    void ChannelSetPosition(g_id channel, unsigned int position)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        tick(channel2);

        if (channel2->source != 0)
            alSourcef(channel2->source, AL_SEC_OFFSET, position / 1000.0);
    }

    unsigned int ChannelGetPosition(g_id channel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return 0;

        Channel *channel2 = iter->second;

        tick(channel2);

        if (channel2->source == 0)
            return channel2->lastPosition;

        ALfloat offset;
        alGetSourcef(channel2->source, AL_SEC_OFFSET, &offset);

        return offset * 1000.0;
    }

    void ChannelSetPaused(g_id channel, bool paused)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        if (channel2->paused == paused)
            return;

        tick(channel2);

        channel2->paused = paused;

        if (channel2->source != 0)
        {
            if (paused)
                alSourcePause(channel2->source);
            else
                alSourcePlay(channel2->source);
        }
    }

    bool ChannelIsPaused(g_id channel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return false;

        Channel *channel2 = iter->second;

        return channel2->paused;
    }

    bool ChannelIsPlaying(g_id channel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return false;

        Channel *channel2 = iter->second;

        tick(channel2);

        if (channel2->source == 0)
            return false;

        ALint state;
        alGetSourcei(channel2->source, AL_SOURCE_STATE, &state);

        return state == AL_PLAYING;
    }

    void ChannelSetVolume(g_id channel, float volume)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        channel2->volume = volume;

        if (channel2->source != 0)
            alSourcef(channel2->source, AL_GAIN, volume);
    }

    float ChannelGetVolume(g_id channel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return 0.f;

        Channel *channel2 = iter->second;

        return channel2->volume;
    }

    void ChannelSetPitch(g_id channel, float pitch)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        channel2->pitch = pitch;

        if (channel2->source != 0)
            alSourcef(channel2->source, AL_PITCH, pitch);
    }

    float ChannelGetPitch(g_id channel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return 0.f;

        Channel *channel2 = iter->second;

        return channel2->pitch;
    }

    void ChannelSetLooping(g_id channel, bool looping)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        tick(channel2);

        channel2->looping = looping;

        if (channel2->source != 0)
            alSourcei(channel2->source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
    }

    bool ChannelIsLooping(g_id channel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return false;

        Channel *channel2 = iter->second;

        return channel2->looping;
    }

    void ChannelSetWorldPosition(g_id channel, float x, float y, float z, float vx,float vy,float vz)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        if (channel2->source != 0)
        {
        	alSource3f( channel2->source, AL_POSITION, x,y,z );
        	alSource3f( channel2->source, AL_VELOCITY, vx,vy,vz );
        }
    }

    g_id ChannelAddCallback(g_id channel, gevent_Callback callback, void *udata)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return 0;

        Channel *channel2 = iter->second;

        return channel2->callbackList.addCallback(callback, udata);
    }

    void ChannelRemoveCallback(g_id channel, gevent_Callback callback, void *udata)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        channel2->callbackList.removeCallback(callback, udata);
    }

    void ChannelRemoveCallbackWithGid(g_id channel, g_id gid)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        channel2->callbackList.removeCallbackWithGid(gid);
    }

    g_bool ChannelIsValid(g_id channel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return g_false;

        Channel *channel2 = iter->second;

        return channel2->source != 0;
    }

    void preTick()
    {
        std::map<g_id, Channel*>::iterator iter, e = channels_.end();

        for (iter = channels_.begin(); iter != e; ++iter)
        {
            Channel *channel = iter->second;
            tick(channel);
        }
    }

    void postTick()
    {
        std::map<g_id, Channel*>::iterator iter = channels_.begin(), end = channels_.end();
        while (iter != end)
        {
            Channel *channel2 = iter->second;

            if (channel2->source == 0)
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

private:
    struct Channel;

    struct Sound
    {
        Sound(g_id gid, ALuint buffer, unsigned int length) :
            gid(gid),
            buffer(buffer),
            length(length)
        {

        }

        g_id gid;
        ALuint buffer;
        unsigned int length;
        std::set<Channel*> channels;
    };

    struct Channel
    {
        Channel(g_id gid, Sound *sound, ALuint source) :
            gid(gid),
            sound(sound),
            source(source),
            paused(true),
            volume(1.f),
            pitch(1.f),
            looping(false),
            lastPosition(0)
        {

        }

        g_id gid;
        Sound *sound;
        ALuint source;
        bool paused;
        float volume;
        float pitch;
        bool looping;
        unsigned int lastPosition;
        gevent_CallbackList callbackList;
    };

    std::map<g_id, Sound*> sounds_;
    std::map<g_id, Channel*> channels_;

private:
    void tick(Channel *channel)
    {
        if (channel->source == 0)
            return;

        ALint state;
        alGetSourcei(channel->source, AL_SOURCE_STATE, &state);

        if (state == AL_STOPPED)
        {
            alDeleteSources(1, &channel->source);
            channel->source = 0;

            channel->lastPosition = channel->sound->length;

            gaudio_ChannelCompleteEvent *event = (gaudio_ChannelCompleteEvent*)malloc(sizeof(gaudio_ChannelCompleteEvent));
            event->channel = channel->gid;

            gevent_EnqueueEvent(channel->gid, callback_s, GAUDIO_CHANNEL_COMPLETE_EVENT, event, 1, channel);
        }
    }

    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<Channel*>(udata)->callbackList.dispatchEvent(type, event);
    }
};

extern "C"
{

GGSampleInterface *GGSampleOpenALManagerCreate()
{
    return new GGSampleOpenALManager();
}

void GGSampleOpenALManagerDelete(GGSampleInterface *manager)
{
    delete manager;
}

}


