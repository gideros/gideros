#ifndef SOUNDIMPLEMENTATION_H
#define SOUNDIMPLEMENTATION_H

#include <map>
#include <vector>
#include "soundinterface.h"
#include <string>

class SoundSystem
{
public:
	virtual ~SoundSystem() {}

	virtual void initialize() = 0;
	virtual void deinitialize() = 0;
};

class SoundDecoder
{
public:
	virtual ~SoundDecoder() {}

	virtual unsigned int createSound(const char* fileName) = 0;
	virtual void destroySound(unsigned int sound) = 0;
	virtual double getSoundLength(unsigned int sound) = 0;

	virtual unsigned int playSound(unsigned int sound, double msec, int loops) = 0;
	virtual void destroySoundChannel(unsigned int soundChannel) = 0;
	virtual double getSoundChannelOffset(unsigned int soundChannel) = 0;
	virtual void setSoundChannelVolume(unsigned int soundChannel, float volume) = 0;
	virtual float getSoundChannelVolume(unsigned int soundChannel) = 0;
	virtual void setSoundCompleteCallback(unsigned int soundChannel, void(*callback)(unsigned int, void*), void* data) = 0;

	virtual SoundError getSoundError() = 0;

	virtual void tickSound() = 0;

	virtual void destroyAll() = 0;
};

class SoundImplementation : public SoundInterface
{
public:
	SoundImplementation();

	template <typename T>
	T* addSoundSystem()
	{
		T* soundSystem = new T;
		soundSystems_.push_back(soundSystem);
		return soundSystem;
	}

	template <typename T>
	T* addSoundDecoder(unsigned int nextid)
	{
		T* soundDecoder = new T(nextid);
		soundDecoders_.push_back(soundDecoder);
		return soundDecoder;
	}


	template <typename T, typename T2>
	SoundDecoder* addSoundDecoder(unsigned int nextid, const T2& t2)
	{
		SoundDecoder* soundDecoder = new T(nextid, t2);
		soundDecoders_.push_back(soundDecoder);
		return soundDecoder;
	}

	void addExtension(const char* ext, SoundDecoder* soundDecoder)
	{
		suffixMap_[ext] = soundDecoder;
	}

	virtual ~SoundImplementation();
	
	virtual void initializeSound();
	virtual void deinitializeSound();

	virtual unsigned int createSound(const char* fileName);
	virtual void destroySound(unsigned int sound);
	virtual double getSoundLength(unsigned int sound);

	virtual unsigned int playSound(unsigned int sound, double msec, int loops);
	virtual void destroySoundChannel(unsigned int soundChannel);
	virtual double getSoundChannelOffset(unsigned int soundChannel);
	virtual void setSoundChannelVolume(unsigned int soundChannel, float volume);
	virtual float getSoundChannelVolume(unsigned int soundChannel);
	virtual void setSoundCompleteCallback(unsigned int soundChannel, void(*callback)(unsigned int, void*), void* data);

	virtual void tickSound()
	{
		for (std::size_t i = 0; i < soundDecoders_.size(); ++i)
			soundDecoders_[i]->tickSound();
	}

	virtual void destroyAll()
	{
		for (std::size_t i = 0; i < soundDecoders_.size(); ++i)
			soundDecoders_[i]->destroyAll();
	}

	virtual SoundError getSoundError()
	{
		SoundError result = soundError_;
		soundError_ = eNoSoundError;
		return result;
	}

private:
	std::vector<SoundSystem*> soundSystems_;
	std::map<std::string, SoundDecoder*> suffixMap_;
	std::vector<SoundDecoder*> soundDecoders_;
	std::map<unsigned int, SoundDecoder*> decoderMap_;

	SoundError soundError_;

	void setSoundError(SoundError soundError)
	{
		if (soundError_ == eNoSoundError)
			soundError_ = soundError;
	}
};


#endif
