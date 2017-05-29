#pragma once

#include <gglobal.h>
#include <gevent.h>
#include <string>
#include <vector>

enum
{
    GTTS_INIT_COMPLETE_EVENT,
    GTTS_ERROR_EVENT,
    GTTS_UTTERANCE_COMPLETE_EVENT,
};

void gtts_Init(gevent_Callback callback);
void gtts_Cleanup();
int gtts_Create(const char *lang,float speed,float pitch,void *udata);
void gtts_Destroy(int tid);
bool gtts_SetLanguage(int tid,const char *lang);
bool gtts_SetPitch(int tid,float pitch);
bool gtts_SetSpeed(int tid,float speed);
void gtts_Stop(int tid);
void gtts_Shutdown(int tid);
bool gtts_Speak(int tid,const char *text,const char *utteranceId);
