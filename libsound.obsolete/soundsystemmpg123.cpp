#include "soundsystemmpg123.h"

#include <stdlib.h>
#include <stdio.h>
#if defined(_WIN32) || defined(__APPLE__)
typedef long ssize_t;
#endif
#include <mpg123.h>

void SoundSystemMpg123::initialize()
{
	mpg123_init();
}

void SoundSystemMpg123::deinitialize()
{
	mpg123_exit();
}
