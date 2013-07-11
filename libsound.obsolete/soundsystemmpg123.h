#ifndef SOUNDSYSTEMMPG123_H
#define SOUNDSYSTEMMPG123_H

#include "soundimplementation.h"

class SoundSystemMpg123 : public SoundSystem
{
public:
	virtual void initialize();
	virtual void deinitialize();
};

#endif
