#include "sounddecoderstreamer.h"
#include <stdio.h>
#include <gfile.h>
#include <gstdio.h>

#include "audiostreamer.h"

static void soundEndCallback(void* data)
{
	SoundDecoderStreamer::SoundChannel_t* soundChannel = static_cast<SoundDecoderStreamer::SoundChannel_t*>(data);

	soundChannel->isPlaying = false;
	soundChannel->lastPosition = soundChannel->sound->length;

	if (soundChannel->callback != 0)
		soundChannel->callback(soundChannel->id, soundChannel->data);
}

unsigned int SoundDecoderStreamer::createSound(const char* fileName)
{
	G_FILE* fis = g_fopen(fileName, "rb");
	if (fis == 0)
	{
		setSoundError(eSoundFileNotFound);
		return 0;
	}
	g_fclose(fis);

	gmod::AudioSource* source = new gmod::AudioSourceMpg123(fileName);

	if (source->channelCount() == 0)
	{
		delete source;
		setSoundError(eSoundFormatNotRecognized);
		return 0;
	}

	sounds_[nextid_] = new Sound_t(source, source->length());

	return nextid_++;
}


void SoundDecoderStreamer::destroySound(unsigned int sound)
{
	std::map<unsigned int, Sound_t*>::iterator iter = sounds_.find(sound);

	if (iter == sounds_.end())
		return;

	if (iter->second->soundChannels.empty() == false)
		return;

	delete iter->second->source;

	delete iter->second;

	sounds_.erase(iter);
}

void SoundDecoderStreamer::destroyAll()
{
	while (soundChannels_.empty() == false)
		destroySoundChannel(soundChannels_.begin()->first);

	while (sounds_.empty() == false)
		destroySound(sounds_.begin()->first);
}

double SoundDecoderStreamer::getSoundLength(unsigned int sound)
{
	std::map<unsigned int, Sound_t*>::iterator iter = sounds_.find(sound);

	if (iter == sounds_.end())
		return 0;

	return iter->second->length;
}


unsigned int SoundDecoderStreamer::playSound(unsigned int sound, double msec, int loops)
{
	std::map<unsigned int, Sound_t*>::iterator iter = sounds_.find(sound);

	if (iter == sounds_.end())
		return 0;

	gmod::AudioPlayer* player = new gmod::AudioPlayer(iter->second->source, msec, loops);

	SoundChannel_t* soundChannel = new SoundChannel_t(iter->second, player, nextid_, true, msec);

	player->setSoundCompleteCallback(soundEndCallback, soundChannel);

	iter->second->soundChannels.insert(soundChannel);

	soundChannels_[nextid_] = soundChannel;

	return nextid_++;
}


void SoundDecoderStreamer::destroySoundChannel(unsigned int soundChannel)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);

	if (iter == soundChannels_.end())
		return;

	iter->second->sound->soundChannels.erase(iter->second);
	iter->second->player->stop();
	delete iter->second->player;
	delete iter->second;
	soundChannels_.erase(iter);
}


double SoundDecoderStreamer::getSoundChannelOffset(unsigned int soundChannel)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);

	if (iter == soundChannels_.end())
		return 0;

	if (iter->second->isPlaying == true)
		iter->second->lastPosition = iter->second->player->position();

	return iter->second->lastPosition;
}

void SoundDecoderStreamer::setSoundChannelVolume(unsigned int soundChannel, float volume)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);

	if (iter == soundChannels_.end())
		return;

	iter->second->player->setVolume(volume);
}

float SoundDecoderStreamer::getSoundChannelVolume(unsigned int soundChannel)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);

	if (iter == soundChannels_.end())
		return 0;

	return iter->second->player->getVolume();
}
void SoundDecoderStreamer::setSoundCompleteCallback(unsigned int soundChannel, void(*callback)(unsigned int, void*), void* data)
{
	std::map<unsigned int, SoundChannel_t*>::iterator iter = soundChannels_.find(soundChannel);

	if (iter == soundChannels_.end())
		return;

	iter->second->callback = callback;
	iter->second->data = data;
}

void SoundDecoderStreamer::tickSound()
{
	static std::vector<gmod::AudioPlayer*> v;
	v.clear();

	std::map<unsigned int, SoundChannel_t*>::iterator iter, e = soundChannels_.end();
	for (iter = soundChannels_.begin(); iter != e; ++iter)
		v.push_back(iter->second->player);

	for (std::size_t i = 0; i < v.size(); ++i)
		v[i]->tick();
}

