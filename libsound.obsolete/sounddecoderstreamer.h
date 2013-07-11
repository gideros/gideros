#ifndef SOUNDDECODERSTREMER_H
#define SOUNDDECODERSTREMER_H

#include "soundimplementation.h"
#include <set>

namespace gmod
{
	class AudioSource;
	class AudioPlayer;
};

class SoundDecoderStreamer : public SoundDecoder
{
public:
	SoundDecoderStreamer(unsigned int nextid) : nextid_(nextid)
	{
		soundError_ = eNoSoundError;
	}

	virtual ~SoundDecoderStreamer() {}

	virtual unsigned int createSound(const char* fileName);
	virtual void destroySound(unsigned int sound);
	virtual double getSoundLength(unsigned int sound);

	virtual unsigned int playSound(unsigned int sound, double msec, int loops);
	virtual void destroySoundChannel(unsigned int soundChannel);
	virtual double getSoundChannelOffset(unsigned int soundChannel);
	virtual void setSoundChannelVolume(unsigned int soundChannel, float volume);
	virtual float getSoundChannelVolume(unsigned int soundChannel);
	virtual void setSoundCompleteCallback(unsigned int soundChannel, void(*callback)(unsigned int, void*), void* data);

	virtual void destroyAll();

	virtual void tickSound();

	virtual SoundError getSoundError()
	{
		SoundError result = soundError_;
		soundError_ = eNoSoundError;
		return result;
	}

	struct SoundChannel_t;

	struct Sound_t
	{
		Sound_t() {}
		Sound_t(gmod::AudioSource* source, double length) : source(source), length(length) {}
		gmod::AudioSource* source;
		double length;
		std::set<SoundChannel_t*> soundChannels;
	};

	struct SoundChannel_t
	{
		SoundChannel_t() {}
		SoundChannel_t(Sound_t* sound, gmod::AudioPlayer* player, unsigned int id, bool isPlaying, double lastPosition) : sound(sound), player(player), id(id), isPlaying(isPlaying), lastPosition(lastPosition), callback(0), data(0) {}
		Sound_t* sound;
		gmod::AudioPlayer* player;
		unsigned int id;
		bool isPlaying;
		double lastPosition;
		void(*callback)(unsigned int, void*);
		void* data;
	};


private:
	unsigned int nextid_;
	std::map<unsigned int, Sound_t*> sounds_;
	std::map<unsigned int, SoundChannel_t*> soundChannels_;

private:
	SoundError soundError_;

	void setSoundError(SoundError soundError)
	{
		if (soundError_ == eNoSoundError)
			soundError_ = soundError;
	}
};


#endif
