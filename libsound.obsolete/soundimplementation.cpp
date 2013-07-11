#include "soundimplementation.h"
#include <pystring.h>
#include <string.h>

SoundImplementation::SoundImplementation()
{
	soundError_ = eNoSoundError;
}

SoundImplementation::~SoundImplementation()
{
	for (std::size_t i = 0; i < soundDecoders_.size(); ++i)
		delete soundDecoders_[i];

	for (std::size_t i = 0; i < soundSystems_.size(); ++i)
		delete soundSystems_[i];
}

void SoundImplementation::initializeSound()
{
	for (std::size_t i = 0; i < soundSystems_.size(); ++i)
		soundSystems_[i]->initialize();
}

void SoundImplementation::deinitializeSound()
{
	for (std::size_t i = 0; i < soundDecoders_.size(); ++i)
		soundDecoders_[i]->destroyAll();

	for (std::size_t i = 0; i < soundSystems_.size(); ++i)
		soundSystems_[i]->deinitialize();
}

unsigned int SoundImplementation::createSound(const char* fileName)
{
	// find suffix
	const char* suffix = strrchr(fileName, '.');

	if (suffix == 0)
	{
		setSoundError(eSoundFileFormatNotSupported);
		return 0;
	}

	std::map<std::string, SoundDecoder*>::iterator iter = suffixMap_.find(pystring::lower(suffix + 1));

	if (iter == suffixMap_.end())
	{
		setSoundError(eSoundFileFormatNotSupported);
		return 0;
	}

	unsigned int sound = iter->second->createSound(fileName);

	if (sound == 0)
	{
		setSoundError(iter->second->getSoundError());
		return 0;
	}

	decoderMap_[sound] = iter->second;

	return sound;
}

void SoundImplementation::destroySound(unsigned int sound)
{
	std::map<unsigned int, SoundDecoder*>::iterator iter = decoderMap_.find(sound);
	if (iter == decoderMap_.end())
		return;

	iter->second->destroySound(sound);

	decoderMap_.erase(iter);
}


double SoundImplementation::getSoundLength(unsigned int sound)
{
	std::map<unsigned int, SoundDecoder*>::iterator iter = decoderMap_.find(sound);
	if (iter == decoderMap_.end())
		return 0;

	return iter->second->getSoundLength(sound);
}

unsigned int SoundImplementation::playSound(unsigned int sound, double msec, int loops)
{
	std::map<unsigned int, SoundDecoder*>::iterator iter = decoderMap_.find(sound);
	if (iter == decoderMap_.end())
		return 0;

	unsigned int soundChannel = iter->second->playSound(sound, msec, loops);

	if (soundChannel == 0)
	{
		setSoundError(iter->second->getSoundError());
		return 0;
	}

	decoderMap_[soundChannel] = iter->second;

	return soundChannel;
}

void SoundImplementation::destroySoundChannel(unsigned int soundChannel)
{
	std::map<unsigned int, SoundDecoder*>::iterator iter = decoderMap_.find(soundChannel);
	if (iter == decoderMap_.end())
		return;

	iter->second->destroySoundChannel(soundChannel);

	decoderMap_.erase(iter);
}

double SoundImplementation::getSoundChannelOffset(unsigned int soundChannel)
{
	std::map<unsigned int, SoundDecoder*>::iterator iter = decoderMap_.find(soundChannel);
	if (iter == decoderMap_.end())
		return 0;

	return iter->second->getSoundChannelOffset(soundChannel);
}

void SoundImplementation::setSoundChannelVolume(unsigned int soundChannel, float volume)
{
	std::map<unsigned int, SoundDecoder*>::iterator iter = decoderMap_.find(soundChannel);
	if (iter == decoderMap_.end())
		return;

	return iter->second->setSoundChannelVolume(soundChannel, volume);
}

float SoundImplementation::getSoundChannelVolume(unsigned int soundChannel)
{
	std::map<unsigned int, SoundDecoder*>::iterator iter = decoderMap_.find(soundChannel);
	if (iter == decoderMap_.end())
		return 0;

	return iter->second->getSoundChannelVolume(soundChannel);
}

void SoundImplementation::setSoundCompleteCallback(unsigned int soundChannel, void(*callback)(unsigned int, void*), void* data)
{
	std::map<unsigned int, SoundDecoder*>::iterator iter = decoderMap_.find(soundChannel);
	if (iter == decoderMap_.end())
		return;

	iter->second->setSoundCompleteCallback(soundChannel, callback, data);
}
