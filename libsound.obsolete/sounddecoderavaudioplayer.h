/*
 *  sounddecoderavaudioplayer.h
 *  AudioPlayer
 *
 *  Created by Atilim Cetin on 11/9/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "soundimplementation.h"

#include <map>

#import <AVFoundation/AVFoundation.h>

class SoundDecoderAVAudioPlayer : public SoundDecoder
{
public:
	SoundDecoderAVAudioPlayer(unsigned int nextid) : nextid_(nextid)
	{
		soundError_ = eNoSoundError;
	}
		
	virtual ~SoundDecoderAVAudioPlayer() {}
	
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
	
	virtual void tickSound() {}
	
	virtual void destroyAll();
	
	
	struct Sound_t;	
	struct SoundChannel_t;
		
	struct Sound_t
	{
		Sound_t(AVAudioPlayer* audioPlayer, const char* fileName, double length) :
			audioPlayer(audioPlayer),
			fileName(fileName),
			length(length) {}

		AVAudioPlayer* audioPlayer;
		std::string fileName;
		double length;
	};
	
	struct SoundChannel_t
	{
		SoundChannel_t(Sound_t* sound, AVAudioPlayer* audioPlayer, unsigned int id, bool isPlaying, double lastPosition) :
			sound(sound),
			audioPlayer(audioPlayer),
			isPlaying(isPlaying),
			lastPosition(lastPosition),
			callback(NULL),
			data(NULL)
		{
		}

		Sound_t* sound;
		AVAudioPlayer* audioPlayer;
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
	
	SoundError soundError_;

	void setSoundError(SoundError soundError)
	{
		if (soundError_ == eNoSoundError)
			soundError_ = soundError;
	}
};
