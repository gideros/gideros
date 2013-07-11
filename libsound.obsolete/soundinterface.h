#ifndef SOUNDINTERFACE_H
#define SOUNDINTERFACE_H

enum SoundError
{
	eNoSoundError,
	eSoundFileNotFound,
	eSoundFormatNotRecognized,
	eSoundFileFormatNotSupported,
};

class SoundInterface
{
public:
	virtual ~SoundInterface() {}

	virtual void initializeSound() = 0;
	virtual void deinitializeSound() = 0;

	virtual unsigned int createSound(const char* fileName) = 0;
	virtual void destroySound(unsigned int sound) = 0;
	virtual double getSoundLength(unsigned int sound) = 0;

	virtual unsigned int playSound(unsigned int sound, double msec, int loops) = 0;
	virtual void destroySoundChannel(unsigned int soundChannel) = 0;
	virtual double getSoundChannelOffset(unsigned int soundChannel) = 0;
	virtual void setSoundChannelVolume(unsigned int soundChannel, float volume) = 0;
	virtual float getSoundChannelVolume(unsigned int soundChannel) = 0;
	virtual void setSoundCompleteCallback(unsigned int soundChannel, void(*callback)(unsigned int, void*), void* data) = 0;
	virtual void tickSound() = 0;
	virtual SoundError getSoundError() = 0;
};

#endif
