#include "soundsystemopenal.h"

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

void SoundSystemOpenAL::initialize()
{
	ALCdevice* device = alcOpenDevice(NULL);

	int params[3];
	params[0] = ALC_FREQUENCY;
	params[1] = 22050;
	params[2] = 0;

	ALCcontext* context = alcCreateContext(device, params);

	alcMakeContextCurrent(context);
}

void SoundSystemOpenAL::deinitialize()
{
	ALCcontext* context;
	ALCdevice* device;

	context = alcGetCurrentContext();
	device = alcGetContextsDevice(context);

	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}
