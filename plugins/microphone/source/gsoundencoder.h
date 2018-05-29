#pragma once

#include <gglobal.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

g_id gsoundencoder_WavCreate(const char *fileName, int numChannels, int sampleRate, int bitsPerSample, float quality);
void gsoundencoder_WavClose(g_id id);
size_t gsoundencoder_WavWrite(g_id id, size_t sampleCount, void *data);

#ifdef __cplusplus
}
#endif
