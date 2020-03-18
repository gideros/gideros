#include <gaudio.h>

#include "ggaudiomanager.h"

#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <set>

#include <xaudio2.h>
#include "winrt/wave.h"

extern IXAudio2 *g_audioengine;
extern IXAudio2MasteringVoice *g_masteringvoice;

class GGSampleXAudio2Manager : public GGSampleInterface
{
public:
    GGSampleXAudio2Manager()
    {
    }

    ~GGSampleXAudio2Manager()
    {
        while (!sounds_.empty())
        {
            Sound *sound = sounds_.begin()->second;
            SoundDelete(sound->gid);
        }
    }

    g_id SoundCreateFromBuffer(const void *data, int numChannels, int sampleRate, int bitsPerSample, int numSamples)
    {
	//        alBufferData(buffer, format, data, numSamples * numChannels * (bitsPerSample / 8), sampleRate);

        g_id gid = g_NextId();

		Wave *pbuffer=new Wave;
		pbuffer->Create(data, numChannels, sampleRate, bitsPerSample, numSamples);

        sounds_[gid] = new Sound(gid, pbuffer, (numSamples * 1000LL) / sampleRate);

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
				channel->source->Stop();
				channel->source->FlushSourceBuffers();
			}

            channels_.erase(channel->gid);

            gevent_RemoveEventsWithGid(channel->gid);

            delete channel;
        }

		Wave *pbuffer=sound2->pbuffer;
		pbuffer->Destroy();   // deallocates memory for the buffer
		delete pbuffer;
		//        alDeleteBuffers(1, &sound2->buffer);

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

    g_id SoundPlay(g_id sound, bool paused, bool streaming)
    {
        std::map<g_id, Sound*>::iterator iter = sounds_.find(sound);
        if (iter == sounds_.end())
            return 0;

        if (channels_.size() >= 31)
            return 0;

        Sound *sound2 = iter->second;

	//        ALuint source;
	//        alGetError();
	//        alGenSources(1, &source);
	//        if(alGetError() != AL_NO_ERROR)
	//            return 0;

	//        alSourcei(source, AL_BUFFER, sound2->buffer);

		IXAudio2SourceVoice* source;
		Wave *pbuffer = (Wave *) sound2->pbuffer;
		const WAVEFORMATEX *wf=pbuffer->wf();

		g_audioengine->CreateSourceVoice(&source,wf);
		source->SubmitSourceBuffer(pbuffer->xaBuffer());

		g_id gid = g_NextId();

        Channel *channel = new Channel(gid, sound2, source);

        sound2->channels.insert(channel);

        channels_[gid] = channel;

        channel->paused = paused;
		if (!paused)
			source->Start();
		else
			source->Stop();

        return gid;
    }

    void ChannelStop(g_id channel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

		if (channel2->source != NULL)
		{
			channel2->source->Stop();
			channel2->source->FlushSourceBuffers();
			channel2->source->DestroyVoice();
			//            alSourceStop(channel2->source);
			//            alDeleteSources(1, &channel2->source);
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

		if (channel2->source != 0){

			channel2->source->Stop();
			channel2->source->FlushSourceBuffers();
			channel2->source->DestroyVoice();
			channel2->source = NULL;

			Wave *pbuffer = (Wave *)channel2->sound->pbuffer;
			const WAVEFORMATEX *wf = pbuffer->wf();
			const XAUDIO2_BUFFER *pxa = pbuffer->xaBuffer();

			XAUDIO2_BUFFER xa2 = *pxa;

			int samPosition = position / 1000.0 * wf->nSamplesPerSec;

			xa2.PlayBegin = samPosition;  
			channel2->samPosition = samPosition;

			if (samPosition == 0)
				xa2.PlayLength = 0;
			else
				xa2.PlayLength = (xa2.AudioBytes / wf->nBlockAlign) - samPosition;

			xa2.LoopBegin = 0;
			xa2.LoopLength = 0;

			if (channel2->looping)
				xa2.LoopCount = XAUDIO2_LOOP_INFINITE;
			else
				xa2.LoopCount = 0;

			g_audioengine->CreateSourceVoice(&channel2->source, wf);
			channel2->source->SubmitSourceBuffer(&xa2);
			channel2->source->Start();
		}

	  //            alSourcef(channel2->source, AL_SEC_OFFSET, position / 1000.0);
    }

    unsigned int ChannelGetPosition(g_id channel)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return 0;

        Channel *channel2 = iter->second;

        tick(channel2);

        if (channel2->source == NULL)
            return channel2->lastPosition;

    //    ALfloat offset;
	//        alGetSourcef(channel2->source, AL_SEC_OFFSET, &offset);

		Wave *pbuffer = (Wave *)channel2->sound->pbuffer;
		const WAVEFORMATEX *wf = pbuffer->wf();

		XAUDIO2_VOICE_STATE state;
		channel2->source->GetState(&state);
		int offset = (channel2->samPosition + state.SamplesPlayed) / (wf->nSamplesPerSec);

		return offset * 1000.0;  // milliseconds
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
	      channel2->source->Stop();
	    else
	      channel2->source->Start();
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

        if (channel2->source == NULL)
            return false;

	//        ALint state;
	//        alGetSourcei(channel2->source, AL_SOURCE_STATE, &state);

		XAUDIO2_VOICE_STATE state;
		channel2->source->GetState(&state);
		if (state.BuffersQueued > 0 && !channel2->paused)
			return true;
		else
	        return false;
    }

    void ChannelSetVolume(g_id channel, float volume)
    {
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        channel2->volume = volume;

		if (channel2->source != NULL)
			channel2->source->SetVolume(volume);

	//        if (channel2->source != 0)
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

    void ChannelSetPitch(g_id channel, float pitch)
    {
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
        std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
        if (iter == channels_.end())
            return;

        Channel *channel2 = iter->second;

        tick(channel2);

        channel2->looping = looping;

		if (channel2->source != 0){
			XAUDIO2_VOICE_STATE state;
			channel2->source->GetState(&state);

			int samplesPlayed = state.SamplesPlayed;

			channel2->source->Stop();
			channel2->source->FlushSourceBuffers();
			channel2->source->DestroyVoice();
			channel2->source = NULL;

			Wave *pbuffer = (Wave *)channel2->sound->pbuffer;
			const WAVEFORMATEX *wf = pbuffer->wf();
			const XAUDIO2_BUFFER *pxa= pbuffer->xaBuffer();

			XAUDIO2_BUFFER xa2 = *pxa;

			xa2.PlayBegin = channel2->samPosition;
			xa2.PlayLength = (xa2.AudioBytes/wf->nBlockAlign)- channel2->samPosition;
			xa2.LoopBegin = 0;
			xa2.LoopLength = 0;
			
			if (looping)
				xa2.LoopCount = XAUDIO2_LOOP_INFINITE;
			else
				xa2.LoopCount = 0;

			g_audioengine->CreateSourceVoice(&channel2->source, wf);
			channel2->source->SubmitSourceBuffer(&xa2);
			channel2->source->Start();

		}
	  //            alSourcei(channel2->source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
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

            if (channel2->source == NULL)
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
        Sound(g_id gid, Wave *pbuffer, unsigned int length) :
            gid(gid),
            pbuffer(pbuffer),
            length(length)
        {

        }

        g_id gid;
        Wave *pbuffer;
        unsigned int length;
        std::set<Channel*> channels;
    };

    struct Channel
    {
        Channel(g_id gid, Sound *sound, IXAudio2SourceVoice *source) :
            gid(gid),
            sound(sound),
            source(source),
            paused(true),
            volume(1.f),
            pitch(1.f),
            looping(false),
            lastPosition(0),
			samPosition(0)
        {

        }

        g_id gid;
        Sound *sound;
        IXAudio2SourceVoice *source;
        bool paused;
        float volume;
        float pitch;
        bool looping;
        unsigned int lastPosition;
        gevent_CallbackList callbackList;
		unsigned int samPosition;
    };

    std::map<g_id, Sound*> sounds_;
    std::map<g_id, Channel*> channels_;

private:
    void tick(Channel *channel)
    {
        if (channel->source == NULL)
            return;

//        ALint state;
//        alGetSourcei(channel->source, AL_SOURCE_STATE, &state);

		XAUDIO2_VOICE_STATE state;
		channel->source->GetState(&state);

        if (state.BuffersQueued==0)
        {
			channel->source->DestroyVoice();
			channel->source = NULL;
//			alDeleteSources(1, &channel->source);
//            channel->source = 0;

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

GGSampleInterface *GGSampleXAudio2ManagerCreate()
{
    return new GGSampleXAudio2Manager();
}

void GGSampleXAudio2ManagerDelete(GGSampleInterface *manager)
{
    delete manager;
}

}
