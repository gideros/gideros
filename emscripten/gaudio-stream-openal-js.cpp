/*
notes:
1. OpenAL is a thread-safe library.
2. It's hard to delete queued buffers without deleting the source first.
3. The streaming code is based on {mpg123 sources}/src/output/openal.c
4. Be careful about double locks
*/

#include <gaudio.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "ggaudiomanager.h"

#define AL_ALEXT_PROTOTYPES
#if defined(OPENAL_SUBDIR_OPENAL)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#elif defined(OPENAL_SUBDIR_AL)
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#else
#include <al.h>
#include <alc.h>
#endif

#include <stdio.h>

#include <map>
#include <set>
#include <deque>


#include <stdlib.h>

#define NUM_BUFFERS 4
#define BUFFER_SIZE (4096 * 4)

namespace {

class GGStreamOpenALManager : public GGStreamInterface
{
public:
    GGStreamOpenALManager()
    {
        running_ = true;
    }

    ~GGStreamOpenALManager()
    {
        {
            running_ = false;
        }

        while (!sounds_.empty())
        {
            Sound *sound = sounds_.begin()->second;
            SoundDelete(sound->gid);
        }
    }

    g_id SoundCreateFromFile(const char *fileName, const GGAudioLoader& loader, gaudio_Error *error)
    {

        int numChannels, sampleRate, bitsPerSample, numSamples;
        g_id file = loader.open(fileName, &numChannels, &sampleRate, &bitsPerSample, &numSamples, error);

        if (file == 0)
            return 0;

        loader.close(file);

        g_id gid = g_NextId();
        sounds_[gid] = new Sound(gid, fileName, loader, numChannels, sampleRate, bitsPerSample, numSamples);

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
                deleteSourceAndBuffers(channel);

            channel->sound->loader.close(channel->file);

            channels_.erase(channel->gid);

            gevent_RemoveEventsWithGid(channel->gid);

            delete channel;
        }

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

    g_id SoundPlay(g_id sound, bool paused, bool streaming)
    {

        std::map<g_id, Sound*>::iterator iter = sounds_.find(sound);
        if (iter == sounds_.end())
            return 0;

        Sound *sound2 = iter->second;

        ALuint source;
        alGetError();
        alGenSources(1, &source);
        if(alGetError() != AL_NO_ERROR)
            return 0;

        gaudio_Error aerr;
        g_id file = sound2->loader.open(sound2->fileName.c_str(), NULL, NULL, NULL, NULL, &aerr);
        if (file == 0)
        {
            printf("Cannot open audio file:%d\n",aerr);
            return 0;
        }
            
        g_id gid = g_NextId();

        Channel *channel = new Channel(gid, file, sound2, source, streaming);

        sound2->channels.insert(channel);

        channels_[gid] = channel;

        enqueueBuffer(channel);

        channel->paused = paused;
        if (!paused)
            alSourcePlay(channel->source);

        return gid;
    }

    bool SoundHasEffect(const char *effect)
    {
#ifdef AL_EFFECT_TYPE
    	return (effect&&!strcmp(effect,"equalizer"));
#else
    	return false;
#endif
    }

    void ChannelStop(g_id channel)
    {

        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        if (channel2->source != 0)
            deleteSourceAndBuffers(channel2);
#ifdef AL_EFFECT_TYPE
        if (channel2->slot)
            alDeleteAuxiliaryEffectSlots(1,&channel2->slot);
        if (channel2->effect)
            alDeleteEffects(1,&channel2->effect);
#endif
        channel2->sound->loader.close(channel2->file);

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

        if (channel2->source == 0)
            return;

        deleteSourceAndBuffers(channel2);

        alGenSources(1, &channel2->source);

        channel2->nodata = false;

        channel2->sound->loader.seek(channel2->file, ((long long)position * channel2->sound->sampleRate) / 1000, SEEK_SET);

        enqueueBuffer(channel2);

        if (!channel2->paused)
            alSourcePlay(channel2->source);
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

        return channel2->buffers[0].second + (unsigned int)(offset * 1000.0);
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

    bool ChannelIsPlaying(g_id channel, int *bufferSize, float *bufferSeconds)
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

        *bufferSize=channel2->bufferedSize;
        int sampSize=channel2->sound->bitsPerSample*channel2->sound->numChannels/8;
        *bufferSeconds=((float)channel2->bufferedSize)/(channel2->sound->sampleRate*sampSize);

        return state == AL_PLAYING;
    }

    void ChannelSetVolume(g_id channel, float volume, float balance)
    {

        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        channel2->volume = volume;

        if (channel2->source != 0) {
            alSourcef(channel2->source, AL_GAIN, volume);
#ifdef AL_BALANCE
            alSourcef(channel2->source, AL_BALANCE, balance);
#endif
        	}
    }

    float ChannelGetVolume(g_id channel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return 0.f;

        Channel *channel2 = iter->second;

        return channel2->volume;
    }

    g_id ChannelGetStreamId(g_id channel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return 0.f;

        Channel *channel2 = iter->second;

        return channel2->file;
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

        if (channel2->source != 0 && looping)
            channel2->nodata = false;
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

    void ChannelSetEffect(g_id channel, const char *effect, float *params)
    {
#ifdef AL_EFFECT_TYPE
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;
        int etype=0;
        if (!strcmp(effect,"equalizer"))
            etype=AL_EFFECT_EQUALIZER;
    	if (etype) {
    		if (!channel2->slot)
    			alGenAuxiliaryEffectSlots(1, &channel2->slot);
    		if (!channel2->effect) {
                alGenEffects(1, &channel2->effect);
        		alEffecti(channel2->effect, AL_EFFECT_TYPE, etype);
    		}
            if (params) {
                switch (etype) {
                    case AL_EFFECT_EQUALIZER:
                    {
                        //Set params
                        for (int p=AL_EQUALIZER_LOW_GAIN;p<=AL_EQUALIZER_HIGH_CUTOFF;p++)
                            alEffectf(channel2->effect,p,params[p]);
                    }
                    break;
                }
                alSourcef(channel2->source, AL_DIRECT_GAIN,params[0]);
            }
            alAuxiliaryEffectSloti(channel2->slot, AL_EFFECTSLOT_EFFECT, (ALint)channel2->effect);
            alSource3i(channel2->source, AL_AUXILIARY_SEND_FILTER, (ALint)channel2->slot, 0, AL_FILTER_NULL);
        }
        else {
            if (channel2->slot)
                alDeleteAuxiliaryEffectSlots(1,&channel2->slot);
            if (channel2->effect)
                alDeleteEffects(1,&channel2->effect);
            alSourcef(channel2->source, AL_DIRECT_GAIN,1);
            channel2->slot=0;
	   		channel2->effect=0;
    	}
#endif
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

    }

    void postTick()
    {

        std::map<g_id, Channel*>::iterator iter = channels_.begin(), end = channels_.end();
        while (iter != end)
        {
            Channel *channel2 = iter->second;

            if (channel2->toClose)
            {
                channel2->sound->loader.close(channel2->file);

                channel2->sound->channels.erase(channel2);
                delete channel2;
                channels_.erase(iter++);
            }
            else
            {
                if (channel2->source==0)
                    channel2->toClose=true; //Delay close for one cycle, in case event was enqueued asynchronously
                ++iter;
            }
        }
    }

public:
    void AdvanceStreamBuffers()
    {
            std::map<g_id, Channel*>::iterator iter, e = channels_.end();

            for (iter = channels_.begin(); iter != e; ++iter)
            {
                Channel *channel = iter->second;
                tick(channel);
            }
    }

private:
    volatile bool running_;

    struct Channel;

    struct Sound
    {
        Sound(g_id gid, const char *fileName, const GGAudioLoader& loader, int numChannels, int sampleRate, int bitsPerSample, int numSamples) :
            gid(gid),
            fileName(fileName),
            loader(loader),
            numChannels(numChannels),
            sampleRate(sampleRate),
            bitsPerSample(bitsPerSample),
            numSamples(numSamples)
        {
            format = 0;
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

            length = (numSamples * 1000LL) / sampleRate;
        }

        g_id gid;
        std::string fileName;
        GGAudioLoader loader;
        int numChannels;
        int sampleRate;
        int bitsPerSample;
        int numSamples;
        ALenum format;
        unsigned int length;
        std::set<Channel*> channels;
    };

    struct Channel
    {
        Channel(g_id gid, g_id file, Sound *sound, ALuint source, bool streaming) :
            gid(gid),
            file(file),
            sound(sound),
            source(source),
			effect(0),
			slot(0),
            paused(true),
            volume(1.f),
            pitch(1.f),
            looping(false),
            nodata(false),
            toClose(false),
			streaming(streaming),
			bufferedSize(0),
            lastPosition(0)
        {
        }

        g_id gid;
        g_id file;
        Sound *sound;
        ALuint source;
        ALuint effect;
        ALuint slot;
        bool paused;
        float volume;
        float pitch;
        bool looping;
        bool nodata;
        bool toClose;
        bool streaming;
        unsigned int lastPosition;
        unsigned int bufferedSize;

        std::deque<std::pair<ALuint, unsigned int> > buffers;

        gevent_CallbackList callbackList;
    };

    std::map<g_id, Sound*> sounds_;
    std::map<g_id, Channel*> channels_;

private:
    void tick(Channel *channel)
    {
        if (channel->source == 0)
            return;

        if (channel->nodata)
        {
            ALint state;
            alGetSourcei(channel->source, AL_SOURCE_STATE, &state);

            if (state == AL_STOPPED)
            {
                deleteSourceAndBuffers(channel);

                channel->lastPosition = channel->sound->length;

                gaudio_ChannelCompleteEvent *event = (gaudio_ChannelCompleteEvent*)malloc(sizeof(gaudio_ChannelCompleteEvent));
                event->channel = channel->gid;

                gevent_EnqueueEvent(channel->gid, callback_s, GAUDIO_CHANNEL_COMPLETE_EVENT, event, 1, channel);
            }
        }
        else
        {
            enqueueBuffer(channel);
        }
    }

    void enqueueBuffer(Channel *channel)
    {
        char data[BUFFER_SIZE];

        ALint queued;
        alGetSourcei(channel->source, AL_BUFFERS_QUEUED, &queued);

        ALuint buffer;

        if (queued < NUM_BUFFERS)
        {
            alGenBuffers(1, &buffer);
        }
        else
        {
            ALint processed;
            ALint sizeInBytes;
            alGetSourcei(channel->source, AL_BUFFERS_PROCESSED, &processed);

            if (processed == 0)
            {
                return;
            }

            alSourceUnqueueBuffers(channel->source, 1, &buffer);
            alGetBufferi(buffer, AL_SIZE, &sizeInBytes);
            channel->buffers.pop_front();
            channel->bufferedSize-=sizeInBytes;
        }

        int sampSize=channel->sound->bitsPerSample*channel->sound->numChannels/8;
        int sampMult=1;
        if (channel->sound->sampleRate<22050)
            sampMult++;
        if (channel->sound->sampleRate<11025)
            sampMult+=2;

        unsigned int pos = (channel->sound->loader.tell(channel->file) * 1000LL) / channel->sound->sampleRate;
        size_t size = channel->sound->loader.read(channel->file, BUFFER_SIZE/sampMult, data);

        if (size == 0 && channel->looping)
        {
            channel->sound->loader.seek(channel->file, 0, SEEK_SET);
            pos = 0;
            size = channel->sound->loader.read(channel->file, BUFFER_SIZE/sampMult, data);
        }

        if (size > 0)
        {
            //printf("SampRate:%d SampMult:%d SampSize:%d\n",channel->sound->sampleRate,sampMult,sampSize);
        	ALenum cformat=channel->sound->format;
        	int csr=channel->sound->sampleRate;
        	int sampMult=1;
        	if (channel->sound->sampleRate<22050)
            	sampMult++;
        	if (channel->sound->sampleRate<11025)
            sampMult+=2;
        	if (channel->sound->loader.format)
        	{
        		int chn;
        		channel->sound->loader.format(channel->file,&csr,&chn);
	            if (channel->sound->bitsPerSample == 8)
	            {
	                if (chn == 1)
	                	cformat = AL_FORMAT_MONO8;
        		    else if (chn == 2)
        		        cformat = AL_FORMAT_STEREO8;
        		}
        		else if (channel->sound->bitsPerSample == 16)
        		{
        		     if (chn == 1)
        		          cformat = AL_FORMAT_MONO16;
        		     else if (chn == 2)
        		          cformat = AL_FORMAT_STEREO16;
        		}
        	}
            if (sampMult>1)
            {
             //Expand buffer to match sampleRate             
             int nsmp=size/sampSize;
                if (sampSize==1)
                {
                 int8_t *b=(int8_t *)(data+size*sampMult);
                 for (int k=nsmp-1;k>=0;k--)
                 {
                     int8_t smp=((int8_t *)data)[k];
                     for (int j=0;j<sampMult;j++)
                         *(--b)=smp;
                 }
                }
                else if (sampSize==2)
                {
                 int16_t *b=(int16_t *)(data+size*sampMult);
                 for (int k=size-1;k>=0;k--)
                 {
                     int16_t smp=((int16_t *)data)[k];
                     for (int j=0;j<sampMult;j++)
                         *(--b)=smp;
                 }
                }
                else if (sampSize==4)
                {
                 int32_t *b=(int32_t *)(data+size*sampMult);
                 for (int k=size-1;k>=0;k--)
                 {
                     int32_t smp=((int32_t *)data)[k];
                     for (int j=0;j<sampMult;j++)
                         *(--b)=smp;
                 }                
                }
            }
            alBufferData(buffer, cformat, data, size*sampMult, csr*sampMult);
            alSourceQueueBuffers(channel->source, 1, &buffer);
            channel->bufferedSize+=size;
            channel->buffers.push_back(std::make_pair(buffer, pos));
            if (!channel->paused)
            {
                ALint state;
                alGetSourcei(channel->source, AL_SOURCE_STATE, &state);
                if (state == AL_STOPPED)
                    alSourcePlay(channel->source);
            }
        }
        else
        {
           	alDeleteBuffers(1, &buffer);
            if ((size == 0)&&(!channel->streaming))
	            channel->nodata = true;
        }
    }

    void deleteSourceAndBuffers(Channel *channel)
    {
        alSourceStop(channel->source);
        alDeleteSources(1, &channel->source);
        channel->source = 0;
        for (size_t i = 0; i < channel->buffers.size(); ++i)
            alDeleteBuffers(1, &channel->buffers[i].first);
        channel->buffers.clear();
        channel->bufferedSize=0;
    }

    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<Channel*>(udata)->callbackList.dispatchEvent(type, event);
    }
};

}

extern "C"
{

GGStreamInterface *GGStreamOpenALManagerCreate()
{
    return new GGStreamOpenALManager();
}

void GGStreamOpenALManagerDelete(GGStreamInterface *manager)
{
    delete manager;
}

}
