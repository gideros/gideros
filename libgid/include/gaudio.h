#ifndef GAUDIO_H
#define GAUDIO_H

#include <gglobal.h>
#include <gevent.h>

enum gaudio_Error
{
    GAUDIO_NO_ERROR,
    GAUDIO_CANNOT_OPEN_FILE,
    GAUDIO_UNRECOGNIZED_FORMAT,
    GAUDIO_ERROR_WHILE_READING,
    GAUDIO_UNSUPPORTED_FORMAT,
    GAUDIO_INTERNAL_ERROR,
};

typedef struct gaudio_ChannelCompleteEvent
{
    g_id channel;
} gaudio_ChannelCompleteEvent;

typedef struct gaudio_SyncPointEvent
{
    g_id channel;
    void *udata;
} gaudio_SyncPointEvent;

#ifdef __cplusplus
extern "C" {
#endif

G_API void gaudio_Init();
G_API void gaudio_Cleanup();

// simdilik sadece 8 ve 16 bit wavlari destekliyoruz
// ilerde daha fazla bitsPerSample'i (24 ve 32) desetkleyip, opsiyonel 16'a bite cevirme imkani saglayacagiz.
G_API g_id gaudio_WavOpen(const char *fileName, int *numChannels, int *sampleRate, int *bitsPerSample, int *numSamples, gaudio_Error *error);
G_API void gaudio_WavClose(g_id id);
G_API size_t gaudio_WavRead(g_id id, size_t size, void *data);
G_API int gaudio_WavSeek(g_id id, long int offset, int whence);
G_API long int gaudio_WavTell(g_id id);

// simdilik mpg123'u 16 bit output verecek sekilde ayarladik ve bitsPerSample'i her zaman 16 olarak dondurduk
G_API g_id gaudio_Mp3Open(const char *fileName, int *numChannels, int *sampleRate, int *bitsPerSample, int *numSamples, gaudio_Error *error);
G_API void gaudio_Mp3Close(g_id id);
G_API size_t gaudio_Mp3Read(g_id id, size_t size, void *data);
G_API int gaudio_Mp3Seek(g_id id, long int offset, int whence);
G_API long int gaudio_Mp3Tell(g_id id);

// XMP API
G_API g_id gaudio_XmpOpen(const char *fileName, int *numChannels, int *sampleRate, int *bitsPerSample, int *numSamples, gaudio_Error *error);
G_API void gaudio_XmpClose(g_id id);
G_API size_t gaudio_XmpRead(g_id id, size_t size, void *data);
G_API int gaudio_XmpSeek(g_id id, long int offset, int whence);
G_API long int gaudio_XmpTell(g_id id);

// sound & channel
G_API g_id gaudio_SoundCreateFromFile(const char *fileName, g_bool stream, gaudio_Error *error);
G_API void gaudio_SoundDelete(g_id sound);
G_API unsigned int gaudio_SoundGetLength(g_id sound);
G_API g_id gaudio_SoundPlay(g_id sound, g_bool paused);
G_API void gaudio_SoundListener(float x,float y,float z,float vx,float vy,float vz,float dx,float dy,float dz,float ux,float uy,float uz);

G_API void gaudio_ChannelStop(g_id channel);
G_API void gaudio_ChannelSetPosition(g_id channel, unsigned int position);
G_API unsigned int gaudio_ChannelGetPosition(g_id channel);
G_API void gaudio_ChannelSetPaused(g_id channel, g_bool paused);
G_API g_bool gaudio_ChannelIsPaused(g_id channel);
G_API g_bool gaudio_ChannelIsPlaying(g_id channel);
G_API void gaudio_ChannelSetVolume(g_id channel, float volume);
G_API float gaudio_ChannelGetVolume(g_id channel);
G_API void gaudio_ChannelSetPitch(g_id channel, float pitch);
G_API float gaudio_ChannelGetPitch(g_id channel);
G_API void gaudio_ChannelSetLooping(g_id channel, g_bool looping);
G_API g_bool gaudio_ChannelIsLooping(g_id channel);
G_API g_id gaudio_ChannelAddCallback(g_id channel, gevent_Callback callback, void *udata);
G_API void gaudio_ChannelRemoveCallback(g_id channel, gevent_Callback callback, void *udata);
G_API void gaudio_ChannelRemoveCallbackWithGid(g_id channel, g_id gid);
G_API g_id gaudio_ChannelAddSyncPoint(g_id channel, unsigned int position, void *udata);
G_API void gaudio_ChannelDeleteSyncPoint(g_id channel, g_id syncPoint);
G_API void gaudio_ChannelSetWorldPosition(g_id channel, float x,float y,float z,float vx,float vy,float vz);

// background music & background channel

G_API g_bool gaudio_BackgroundMusicIsAvailable();

G_API g_id gaudio_BackgroundMusicCreateFromFile(const char *fileName, gaudio_Error *error);
G_API void gaudio_BackgroundMusicDelete(g_id backgroundMusic);
G_API unsigned int gaudio_BackgroundMusicGetLength(g_id backgroundMusic);

G_API g_id gaudio_BackgroundMusicPlay(g_id backgroundMusic, g_bool paused);
G_API void gaudio_BackgroundChannelStop(g_id backgroundChannel);
G_API void gaudio_BackgroundChannelSetPosition(g_id backgroundChannel, unsigned int position);
G_API unsigned int gaudio_BackgroundChannelGetPosition(g_id backgroundChannel);
G_API void gaudio_BackgroundChannelSetPaused(g_id backgroundChannel, g_bool paused);
G_API g_bool gaudio_BackgroundChannelIsPaused(g_id backgroundChannel);
G_API g_bool gaudio_BackgroundChannelIsPlaying(g_id backgroundChannel);
G_API void gaudio_BackgroundChannelSetVolume(g_id backgroundChannel, float volume);
G_API float gaudio_BackgroundChannelGetVolume(g_id backgroundChannel);
G_API void gaudio_BackgroundChannelSetLooping(g_id backgroundChannel, g_bool looping);
G_API g_bool gaudio_BackgroundChannelIsLooping(g_id backgroundChannel);
G_API g_id gaudio_BackgroundChannelAddCallback(g_id backgroundChannel, gevent_Callback callback, void *udata);
G_API void gaudio_BackgroundChannelRemoveCallback(g_id backgroundChannel, gevent_Callback callback, void *udata);
G_API void gaudio_BackgroundChannelRemoveCallbackWithGid(g_id backgroundChannel, g_id gid);
G_API g_id gaudio_BackgroundChannelAddSyncPoint(g_id backgroundChannel, unsigned int position, void *udata);
G_API void gaudio_BackgroundChannelDeleteSyncPoint(g_id backgroundChannel, g_id syncPoint);

G_API void gaudio_AdvanceStreamBuffers();

#ifdef __cplusplus
}
#endif

#endif
