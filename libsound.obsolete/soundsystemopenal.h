#ifndef SOUNDSYSTEMOPENAL_H
#define SOUNDSYSTEMOPENAL_H

#include "soundimplementation.h"

class SoundSystemOpenAL : public SoundSystem
{
public:
	virtual void initialize();
	virtual void deinitialize();
};

#endif
