#ifndef SOUNDDECODERMEDIAPLAYER_H
#define SOUNDDECODERMEDIAPLAYER_H

#include <jni.h>
#include "soundimplementation.h"
#include <stopwatch.h>

class SoundDecoderMediaPlayer : public SoundDecoder
{
public:
	SoundDecoderMediaPlayer(unsigned int nextid, JavaVM* vm);
	virtual ~SoundDecoderMediaPlayer();

	virtual unsigned int createSound(const char* fileName);
	virtual void destroySound(unsigned int sound);
	virtual double getSoundLength(unsigned int sound);

	virtual unsigned int playSound(unsigned int sound, double msec, int loops);
	virtual void destroySoundChannel(unsigned int soundChannel);
	virtual double getSoundChannelOffset(unsigned int soundChannel);
	virtual void setSoundChannelVolume(unsigned int soundChannel, float volume);
	virtual float getSoundChannelVolume(unsigned int soundChannel);
	virtual void setSoundCompleteCallback(unsigned int soundChannel, void(*callback)(unsigned int, void*), void* data);
	virtual void soundChannelPaused(unsigned int soundChannel);
	virtual void soundChannelResumed(unsigned int soundChannel);

	virtual SoundError getSoundError()
	{
		SoundError result = soundError_;
		soundError_ = eNoSoundError;
		return result;
	}

	virtual void tickSound();

	virtual void destroyAll();

	void soundCompleteCallback(unsigned int soundChannel);

private:
	unsigned int nextid_;

	JNIEnv* getEnv();
	JavaVM* vm_;
	jclass javaNativeBridge_;
	jmethodID createSoundID_;
	jmethodID destroySoundID_;
	jmethodID getSoundLengthID_;
	jmethodID playSoundID_;
	jmethodID destroySoundChannelID_;
	jmethodID getSoundChannelOffsetID_;
	jmethodID setSoundChannelLoopingID_;
	jmethodID setSoundChannelVolumeID_;
	jmethodID getSoundChannelVolumeID_;
	jmethodID destroyAllID_;
	jmethodID getSoundErrorID_;

	void setSoundChannelLooping(unsigned int soundChannel, bool looping);

	struct SoundCompleteCallback_t
	{
		SoundCompleteCallback_t() : id(0), callback(NULL), data(NULL) {}
		SoundCompleteCallback_t(unsigned int id,
								void(*callback)(unsigned int, void*),
								void* data) :
			id(id),
			callback(callback),
			data(data)
		{

		}

		unsigned int id;
		void(*callback)(unsigned int, void*);
		void* data;
	};

	struct SoundChannelClock_t
	{
		StopWatch stopWatch;
		double endTime;
	};

	std::map<unsigned int, SoundCompleteCallback_t> soundCompleteCallbacks_;
	std::vector<SoundCompleteCallback_t> soundCompleteCallbacks2_;
	std::map<unsigned int, SoundChannelClock_t> soundChannelClocks_;

	SoundError soundError_;

	void setSoundError(SoundError soundError)
	{
		if (soundError_ == eNoSoundError)
			soundError_ = soundError;
	}
};

#endif
