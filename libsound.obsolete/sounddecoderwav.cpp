#include "sounddecoderwav.h"
#include "CWaves.h"
static ALboolean LoadWaveToBuffer(const char *szWaveFile, ALuint uiBufferID)
{
	WAVEID			WaveID;
	unsigned long	iDataSize, iFrequency;
	unsigned long	eBufferFormat;
	ALchar			*pData;
	ALboolean		bReturn;
	CWaves* g_pWaveLoader = new CWaves;

	bReturn = AL_FALSE;
	if (SUCCEEDED(g_pWaveLoader->LoadWaveFile(szWaveFile, &WaveID)))
	{
		if ((SUCCEEDED(g_pWaveLoader->GetWaveSize(WaveID, (unsigned long*)&iDataSize))) &&
			(SUCCEEDED(g_pWaveLoader->GetWaveData(WaveID, (void**)&pData))) &&
			(SUCCEEDED(g_pWaveLoader->GetWaveFrequency(WaveID, (unsigned long*)&iFrequency))) &&
			(SUCCEEDED(g_pWaveLoader->GetWaveALBufferFormat(WaveID, &alGetEnumValue, (unsigned long*)&eBufferFormat))))
		{
			// Set XRAM Mode (if application)
//			if (eaxSetBufferMode && eXRAMBufferMode)
//				eaxSetBufferMode(1, &uiBufferID, eXRAMBufferMode);

			alGetError();
			alBufferData(uiBufferID, eBufferFormat, pData, iDataSize, iFrequency);
			if (alGetError() == AL_NO_ERROR)
				bReturn = AL_TRUE;

			g_pWaveLoader->DeleteWaveFile(WaveID);
		}
	}

	delete g_pWaveLoader;

	return bReturn;
}


void SoundDecoderWav::destroyAll()
{
	while (soundChannels_.empty() == false)
		destroySoundChannel(soundChannels_.begin()->first);

	while (sounds_.empty() == false)
		destroySound(sounds_.begin()->first);
}

unsigned int SoundDecoderWav::createSound(const char* fileName)
{
	G_FILE* fis = g_fopen(fileName, "rb");
	if (fis == 0)
	{
		setSoundError(eSoundFileNotFound);
		return 0;
	}
	g_fclose(fis);

	ALuint buffer;
	alGenBuffers(1, &buffer);

	if (!LoadWaveToBuffer(fileName, buffer))
	{
		setSoundError(eSoundFormatNotRecognized);
		alDeleteBuffers(1, &buffer);
		return 0;
	}

	double length;
	{
		ALint frequency, bits, channels, size;

		alGetBufferi(buffer, AL_FREQUENCY, &frequency);
		alGetBufferi(buffer, AL_BITS, &bits);
		alGetBufferi(buffer, AL_CHANNELS, &channels);
		alGetBufferi(buffer, AL_SIZE, &size);

		length = (double)size / (double)frequency;
		length /= channels;
		length /= bits / 8;
		length *= 1000;
	}

	sounds_[nextid_] = new Sound_t(buffer, length);

	return nextid_++;
}

void SoundDecoderWav::destroySound(unsigned int sound)
{
	std::map<unsigned int, Sound_t*>::iterator iter = sounds_.find(sound);

	if (iter == sounds_.end())
		return;

	if (iter->second->soundChannels.empty() == false)
		return;

	alDeleteBuffers(1, &iter->second->buffer);

	delete iter->second;

	sounds_.erase(iter);
}

double SoundDecoderWav::getSoundLength(unsigned int sound)
{
	std::map<unsigned int, Sound_t*>::iterator iter = sounds_.find(sound);

	if (iter == sounds_.end())
		return 0;

	return iter->second->length;
}

unsigned int SoundDecoderWav::playSound(unsigned int sound, double msec, int loops)
{
	std::map<unsigned int, Sound_t*>::iterator iter = sounds_.find(sound);

	if (iter == sounds_.end())
		return 0;

	ALuint source;
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, iter->second->buffer);
	alSourcef(source, AL_SEC_OFFSET, msec / 1000.0);
	alSourcePlay(source);

	SoundChannel_t* soundChannel = new SoundChannel_t(iter->second, source, nextid_, loops, true, msec);

	iter->second->soundChannels.insert(soundChannel);

	soundChannels_[nextid_] = soundChannel;

	return nextid_++;
}

void SoundDecoderWav::destroySoundChannel(unsigned int soundChannel)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);

	if (iter == soundChannels_.end())
		return;

	iter->second->sound->soundChannels.erase(iter->second);
	if (iter->second->source != 0)
	{
		alSourceStop(iter->second->source);
		alDeleteSources(1, &iter->second->source);
	}
	delete iter->second;
	soundChannels_.erase(iter);
}

/*
void SoundDecoderWav::stopSoundChannel(unsigned int soundChannel)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);

	if (iter == soundChannels_.end())
		return;

	getSoundChannelOffset(soundChannel);		// update lastPosition

	alSourceStop(iter->second->source);
	iter->second->isPlaying = false;
} */

double SoundDecoderWav::getSoundChannelOffset(unsigned int soundChannel)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);

	if (iter == soundChannels_.end())
		return 0;

	if (iter->second->isPlaying == true)
	{
		ALint state;
		alGetSourcei(iter->second->source, AL_SOURCE_STATE, &state);

		if (state == AL_PLAYING)
		{
			ALfloat offset;
			alGetSourcef(iter->second->source, AL_SEC_OFFSET, &offset);
			iter->second->lastPosition = offset * 1000.0;
		}
	}

	return iter->second->lastPosition;
}

void SoundDecoderWav::setSoundChannelVolume(unsigned int soundChannel, float volume)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);

	if (iter == soundChannels_.end())
		return;

	iter->second->volume = volume;
	alSourcef(iter->second->source, AL_GAIN, volume);
}

float SoundDecoderWav::getSoundChannelVolume(unsigned int soundChannel)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);

	if (iter == soundChannels_.end())
		return 0;

	return iter->second->volume;
}

void SoundDecoderWav::tickSound()
{
	std::vector<SoundChannel_t*> callbackeds;

	std::map<unsigned int, SoundChannel_t*>::iterator iter;

	for (iter = soundChannels_.begin(); iter != soundChannels_.end(); ++iter)
	{
		if (iter->second->isPlaying == true)
		{
			ALint state;
			alGetSourcei(iter->second->source, AL_SOURCE_STATE, &state);

			if (state == AL_STOPPED)
			{
				if (iter->second->loops <= 1)
				{
					iter->second->isPlaying = false;
					iter->second->lastPosition = iter->second->sound->length;
					alDeleteSources(1, &iter->second->source);
					iter->second->source = 0;
					if (iter->second->callback)
						callbackeds.push_back(iter->second);
				}
				else
				{
					alSourcePlay(iter->second->source);
					iter->second->loops--;
				}
			}
		}
	}

	for (std::size_t i = 0; i < callbackeds.size(); ++i)
	{
		SoundChannel_t* soundChannel = callbackeds[i];
		soundChannel->callback(soundChannel->id, soundChannel->data);
	}
}

void SoundDecoderWav::setSoundCompleteCallback(unsigned int soundChannel, void(*callback)(unsigned int, void*), void* data)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);

	if (iter == soundChannels_.end())
		return;

	iter->second->callback = callback;
	iter->second->data = data;
}
