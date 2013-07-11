#ifndef SOUNDDECODERWAV_H
#define SOUNDDECODERWAV_H

#include "soundimplementation.h"

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

#include <set>

class SoundDecoderWav : public SoundDecoder
{
public:
	SoundDecoderWav(unsigned int nextid) : nextid_(nextid)
	{
		soundError_ = eNoSoundError;
	}
	virtual ~SoundDecoderWav() {}
	
	virtual unsigned int createSound(const char* fileName);
	virtual void destroySound(unsigned int sound);
	virtual double getSoundLength(unsigned int sound);
	
	virtual unsigned int playSound(unsigned int sound, double msec, int loops);
	virtual void destroySoundChannel(unsigned int soundChannel);
	virtual double getSoundChannelOffset(unsigned int soundChannel);
	virtual void setSoundChannelVolume(unsigned int soundChannel, float volume);
	virtual float getSoundChannelVolume(unsigned int soundChannel);
	virtual void setSoundCompleteCallback(unsigned int soundChannel, void(*callback)(unsigned int, void*), void* data);
	
	virtual SoundError getSoundError()
	{
		SoundError result = soundError_;
		soundError_ = eNoSoundError;
		return result;
	}
	
	virtual void tickSound();

	virtual void destroyAll();

	
private:
	struct SoundChannel_t;
	
	struct Sound_t
	{
		Sound_t() {}
		Sound_t(ALuint buffer, double length) : buffer(buffer), length(length) {}

		ALuint buffer;
		double length;
		std::set<SoundChannel_t*> soundChannels;
	};
	
	struct SoundChannel_t
	{
		SoundChannel_t(Sound_t* sound, ALuint source, unsigned int id, int loops, bool isPlaying, double lastPosition) :
			sound(sound),
			source(source),
			volume(1),
			loops(loops),
			isPlaying(isPlaying),
			lastPosition(lastPosition),
			callback(0),
			data(0)
		{

		}
		
		Sound_t* sound;
		ALuint source;
		unsigned int id;
		float volume;
		int loops;
		bool isPlaying;
		double lastPosition;
		void(*callback)(unsigned int, void*);
		void* data;
	};
	
	unsigned int nextid_;
	std::map<unsigned int, Sound_t*> sounds_;
	std::map<unsigned int, SoundChannel_t*> soundChannels_;

	SoundError soundError_;

	void setSoundError(SoundError soundError)
	{
		if (soundError_ == eNoSoundError)
			soundError_ = soundError;
	}
};


#endif
