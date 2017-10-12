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

#include <stdio.h>

#include <map>
#include <set>
#include <deque>

#include <pthread.h>

#include <stdlib.h>

#include <xaudio2.h>
#include "winrt/wave.h"


#define NUM_BUFFERS 4
#define BUFFER_SIZE (4096 * 4)

typedef unsigned int ALuint;
typedef float ALfloat;

extern IXAudio2 *g_audioengine;
extern IXAudio2MasteringVoice *g_masteringvoice;


namespace {

class GGLock
{
public:
    GGLock(pthread_mutex_t& mutex) : mutex(mutex)
    {
        pthread_mutex_lock(&mutex);
    }
    ~GGLock()
    {
        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_t& mutex;
};

class GGStreamXAudio2Manager : public GGStreamInterface
{
public:
    GGStreamXAudio2Manager()
    {
        mutex_ = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        running_ = true;
        pthread_create(&thread_, NULL, run_s, this);
    }

    ~GGStreamXAudio2Manager()
    {
        {
            GGLock lock(mutex_);
            running_ = false;
        }

        pthread_join(thread_, NULL);

        while (!sounds_.empty())
        {
            Sound *sound = sounds_.begin()->second;
            SoundDelete(sound->gid);
        }
    }

    g_id SoundCreateFromFile(const char *fileName, const GGAudioLoader& loader, gaudio_Error *error)
    {
        GGLock lock(mutex_);

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
        GGLock lock(mutex_);

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
        GGLock lock(mutex_);

        std::map<g_id, Sound*>::iterator iter = sounds_.find(sound);
        if (iter == sounds_.end())
            return 0;

        Sound *sound2 = iter->second;

        return sound2->length;
    }

    g_id SoundPlay(g_id sound, bool paused)
    {
        GGLock lock(mutex_);

        std::map<g_id, Sound*>::iterator iter = sounds_.find(sound);
        if (iter == sounds_.end())
            return 0;

        Sound *sound2 = iter->second;

//	    ALuint source;
	//        alGetError();
	//        alGenSources(1, &source);
	//        if(alGetError() != AL_NO_ERROR)
	//            return 0;

		IXAudio2SourceVoice* source;
		WAVEFORMATEX wf;

		wf.wFormatTag = WAVE_FORMAT_PCM;
		wf.nChannels = sound2->numChannels;
		wf.nSamplesPerSec = sound2->sampleRate;
		wf.nAvgBytesPerSec = sound2->numChannels * sound2->sampleRate * sound2->bitsPerSample / 8;
		wf.nBlockAlign = sound2->bitsPerSample * sound2->numChannels / 8;
		wf.wBitsPerSample = sound2->bitsPerSample;
		wf.cbSize = 0;

		g_audioengine->CreateSourceVoice(&source, &wf);

        g_id file = sound2->loader.open(sound2->fileName.c_str(), NULL, NULL, NULL, NULL, NULL);

        if (file == 0)
            return 0;

        g_id gid = g_NextId();

        Channel *channel = new Channel(gid, file, sound2, source);

        sound2->channels.insert(channel);

        channels_[gid] = channel;

        enqueueBuffer(channel);

        channel->paused = paused;

		if (!paused)
			source->Start();

	//        if (!paused)
	//            alSourcePlay(channel->source);

        return gid;
    }

    void ChannelStop(g_id channel)
    {
        GGLock lock(mutex_);

        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        if (channel2->source != 0)
            deleteSourceAndBuffers(channel2);

        channel2->sound->loader.close(channel2->file);

        channel2->sound->channels.erase(channel2);

        gevent_RemoveEventsWithGid(channel2->gid);

        delete channel2;

        channels_.erase(iter);
    }

    void ChannelSetPosition(g_id channel, unsigned int position)
    {
        GGLock lock(mutex_);

        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        tick(channel2);

        if (channel2->source == 0)
            return;

        deleteSourceAndBuffers(channel2);

	//        alGenSources(1, &channel2->source);
		
		WAVEFORMATEX wf;

		Sound *sound2 = channel2->sound;

		wf.wFormatTag = WAVE_FORMAT_PCM;
		wf.nChannels = sound2->numChannels;
		wf.nSamplesPerSec = sound2->sampleRate;
		wf.nAvgBytesPerSec = sound2->numChannels * sound2->sampleRate * sound2->bitsPerSample / 8;
		wf.nBlockAlign = sound2->bitsPerSample * sound2->numChannels / 8;
		wf.wBitsPerSample = sound2->bitsPerSample;
		wf.cbSize = 0;

		g_audioengine->CreateSourceVoice(&channel2->source, &wf);

        channel2->nodata = false;

        channel2->sound->loader.seek(channel2->file, ((long long)position * channel2->sound->sampleRate) / 1000, SEEK_SET);

        enqueueBuffer(channel2);

	//        if (!channel2->paused)
	//            alSourcePlay(channel2->source);

		if (!channel2->paused)
			channel2->source->Start();
    }

    unsigned int ChannelGetPosition(g_id channel)
    {
        GGLock lock(mutex_);

        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return 0;

        Channel *channel2 = iter->second;

        tick(channel2);

        if (channel2->source == 0)
            return channel2->lastPosition;

	    ALfloat offset;
		//  alGetSourcef(channel2->source, AL_SEC_OFFSET, &offset);
		offset = 0;

        return channel2->buffers[0].second + (unsigned int)(offset * 1000.0);
    }

    void ChannelSetPaused(g_id channel, bool paused)
    {
        GGLock lock(mutex_);

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
				channel2->source->Stop();
			//                alSourcePause(channel2->source);
			else
				channel2->source->Start();
	        //                alSourcePlay(channel2->source);
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
        GGLock lock(mutex_);

        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return false;

        Channel *channel2 = iter->second;

        tick(channel2);

        if (channel2->source == 0)
            return false;

	//        ALint state;
	//        alGetSourcei(channel2->source, AL_SOURCE_STATE, &state);

	//        return state == AL_PLAYING;

		XAUDIO2_VOICE_STATE state;
		channel2->source->GetState(&state);

		if (state.BuffersQueued > 0 && !channel2->paused)
			return true;
		else
			return false;
    }

    void ChannelSetVolume(g_id channel, float volume)
    {
        GGLock lock(mutex_);

        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        channel2->volume = volume;

		if (channel2->source != 0) 
			channel2->source->SetVolume(volume);
	  //            alSourcef(channel2->source, AL_GAIN, volume);
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
        GGLock lock(mutex_);

        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        channel2->pitch = pitch;

		if (channel2->source != 0) channel2->source->SetFrequencyRatio(pitch);
	  //            alSourcef(channel2->source, AL_PITCH, pitch);
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
        GGLock lock(mutex_);

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
        GGLock lock(mutex_);

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
        GGLock lock(mutex_);

        std::map<g_id, Channel*>::iterator iter = channels_.begin(), end = channels_.end();
        while (iter != end)
        {
            Channel *channel2 = iter->second;

            if (channel2->source == 0)
            {
                channel2->sound->loader.close(channel2->file);

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
    static void *run_s(void *arg)
    {
        static_cast<GGStreamXAudio2Manager*>(arg)->run();
        return NULL;
    }

    void run()
    {
        while (running_)
        {
            pthread_mutex_lock(&mutex_);

            std::map<g_id, Channel*>::iterator iter, e = channels_.end();

            for (iter = channels_.begin(); iter != e; ++iter)
            {
                Channel *channel = iter->second;
                tick(channel);
            }

            pthread_mutex_unlock(&mutex_);

#ifdef _WIN32
            Sleep(16);
#else
            usleep(16 * 1000);
#endif
        }
    }

private:
    volatile bool running_;
    pthread_t thread_;
    pthread_mutex_t mutex_;

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
            // format = 0;
            // if (bitsPerSample == 8)
            // {
            //     if (numChannels == 1)
            //         format = AL_FORMAT_MONO8;
            //     else if (numChannels == 2)
            //         format = AL_FORMAT_STEREO8;
            // }
            // else if (bitsPerSample == 16)
            // {
            //     if (numChannels == 1)
            //         format = AL_FORMAT_MONO16;
            //     else if (numChannels == 2)
            //         format = AL_FORMAT_STEREO16;
            // }

            length = (numSamples * 1000LL) / sampleRate;
        }

        g_id gid;
        std::string fileName;
        GGAudioLoader loader;
        int numChannels;
        int sampleRate;
        int bitsPerSample;
        int numSamples;
      //        ALenum format;
        unsigned int length;
        std::set<Channel*> channels;
    };

    struct Channel
    {
        Channel(g_id gid, g_id file, Sound *sound, IXAudio2SourceVoice *source) :
            gid(gid),
            file(file),
            sound(sound),
            source(source),
            paused(true),
            volume(1.f),
            pitch(1.f),
            looping(false),
            nodata(false),
            lastPosition(0)
        {
        }

        g_id gid;
        g_id file;
        Sound *sound;
		IXAudio2SourceVoice *source;
        bool paused;
        float volume;
        float pitch;
        bool looping;
        bool nodata;
        unsigned int lastPosition;

        std::deque<std::pair<Wave *, unsigned int> > buffers;

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

			XAUDIO2_VOICE_STATE state;
			channel->source->GetState(&state);
	  //            ALint state;
	  //            alGetSourcei(channel->source, AL_SOURCE_STATE, &state);

       	    if (state.BuffersQueued==0) // state == AL_STOPPED)
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

        // ALint queued;
        // alGetSourcei(channel->source, AL_BUFFERS_QUEUED, &queued);

		XAUDIO2_VOICE_STATE state;
		channel->source->GetState(&state);
		int queued = state.BuffersQueued;

        // ALuint buffer;
		Wave *buffer;

        // if queued is less than NUM_BUFFERS we either need to create a new buffer or recycle a dead one
		// if the buffer that is playing is not equal to buffers[0] then a buffer has been unqueued

//		char output[100];
//		sprintf(output,"enqueueBuffer: %d queued in channel\n",queued);
//		OutputDebugStringA(output);

        if (queued < NUM_BUFFERS)
        {

			Wave *bufferContext = (Wave *)state.pCurrentBufferContext;   // NULL if no buffers in the queue
        //     ALint processed;
        //     alGetSourcei(channel->source, AL_BUFFERS_PROCESSED, &processed);

        //     if (processed == 0)
        //     {
        //         return;
        //     }

        //     alSourceUnqueueBuffers(channel->source, 1, &buffer);

			if (channel->buffers.size()==0 || bufferContext == channel->buffers[0].first){  // still playing first buffer
//				OutputDebugStringA("still playing first buffer\n");
				buffer = new Wave;
			} else {  // buffer has unqueued
//				OutputDebugStringA("buffer has unqueued\n");
				buffer = channel->buffers[0].first;
				channel->buffers.pop_front();
			}

        }
		else {
			return; // no need to create a new buffer or recycle an old one
		}

        unsigned int pos = (channel->sound->loader.tell(channel->file) * 1000LL) / channel->sound->sampleRate;
        size_t size = channel->sound->loader.read(channel->file, BUFFER_SIZE, data);

		if (size == 0 && channel->looping)
		{
//			OutputDebugStringA("size==0 and looping\n");
			channel->sound->loader.seek(channel->file, 0, SEEK_SET);
			pos = 0;
			size = channel->sound->loader.read(channel->file, BUFFER_SIZE, data);
		}

        if (size != 0)
        {
        	int chn=channel->sound->numChannels;
        	int csr=channel->sound->sampleRate;
        	if (channel->sound->loader.format)
        	{
        		channel->sound->loader.format(channel->file,&csr,&chn);
        	}
//			OutputDebugStringA("size not zero\n");
        //     alBufferData(buffer, channel->sound->format, data, size, channel->sound->sampleRate);
			// either allocate new memory and put data into it, or, if already allocated, replace.
			buffer->Create(data,
				chn,
				csr,
				channel->sound->bitsPerSample,
				channel->sound->numSamples, size);

        //     alSourceQueueBuffers(channel->source, 1, &buffer);
			channel->source->SubmitSourceBuffer(buffer->xaBuffer());

            channel->buffers.push_back(std::make_pair(buffer, pos));
 
		    if (!channel->paused)
            {
				channel->source->Start();
        //         ALint state;
        //         alGetSourcei(channel->source, AL_SOURCE_STATE, &state);
        //         if (state == AL_STOPPED)
        //             alSourcePlay(channel->source);
             }
        }
        else
        {
//			OutputDebugStringA("size zero\n");
        //     alDeleteBuffers(1, &buffer);
			buffer->Destroy();
			delete buffer;
			buffer = NULL;
         
			channel->nodata = true;
        }
    }

    void deleteSourceAndBuffers(Channel *channel)
    {

		channel->source->Stop();
		channel->source->FlushSourceBuffers();
		channel->source->DestroyVoice();
		channel->source = NULL;

		// alSourceStop(channel->source);
        // alDeleteSources(1, &channel->source);
        // channel->source = 0;

		for (size_t i = 0; i < channel->buffers.size(); ++i){
			channel->buffers[i].first->Destroy();        //     alDeleteBuffers(1, &channel->buffers[i].first);
			delete channel->buffers[i].first;
			channel->buffers[i].first = NULL;
		}

        channel->buffers.clear();
    }

    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<Channel*>(udata)->callbackList.dispatchEvent(type, event);
    }
};

}

extern "C"
{

GGStreamInterface *GGStreamXAudio2ManagerCreate()
{
    return new GGStreamXAudio2Manager();
}

void GGStreamXAudio2ManagerDelete(GGStreamInterface *manager)
{
    delete manager;
}

}
