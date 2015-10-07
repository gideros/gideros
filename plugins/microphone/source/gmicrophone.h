#pragma once

#include <gglobal.h>
#include <gevent.h>

enum
{
    GMICROPHONE_DATA_AVAILABLE_EVENT,
};

enum gmicrophone_Error
{
    GMICROPHONE_NO_ERROR,
    GMICROPHONE_CANNOT_OPEN_DEVICE,
    GMICROPHONE_UNSUPPORTED_FORMAT,
};

typedef struct gmicrophone_DataAvailableEvent
{
	g_id microphone;
    void *data;
    int sampleCount;
    float peakAmplitude;
    float averageAmplitude;
} gmicrophone_DataAvailableEvent;

#ifdef __cplusplus
extern "C" {
#endif

void gmicrophone_Init();
void gmicrophone_Cleanup();

g_id gmicrophone_Create(const char *deviceName, int numChannels, int sampleRate, int bitsPerSample, gmicrophone_Error *error);
void gmicrophone_Delete(g_id microphone);
void gmicrophone_Start(g_id microphone);
void gmicrophone_Stop(g_id microphone);
g_id gmicrophone_AddCallback(g_id microphone, gevent_Callback callback, void *udata);
void gmicrophone_RemoveCallback(g_id microphone, gevent_Callback callback, void *udata);
void gmicrophone_RemoveCallbackWithId(g_id microphone, g_id callback);

#ifdef __cplusplus
}
#endif
