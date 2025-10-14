#pragma once

#include <gglobal.h>
#include <gevent.h>
#include <string>
#include <vector>
#include "gideros.h"
#include "lua.h"

class GTts : public GEventDispatcherProxy
{
public:
	enum
	{
	    GTTS_INIT_COMPLETE_EVENT,
	    GTTS_ERROR_EVENT,
	    GTTS_UTTERANCE_COMPLETE_EVENT,
	    GTTS_UTTERANCE_STARTED_EVENT,
	};
    GTts(lua_State *L);
    virtual ~GTts();
    virtual bool SetPitch(float v)=0;
    virtual bool SetSpeed(float v)=0;
    virtual bool SetLanguage(const char *v)=0;
    virtual bool Speak(const char *text, const char *utteranceId)=0;
    virtual void Stop()=0;
    virtual void Shutdown()=0;
    virtual float GetSpeed()=0;
    virtual float GetPitch()=0;
    virtual float GetVolume()=0;
    virtual bool SetVolume(float v)=0;
    virtual bool SetVoice(const char *v)=0;

    static void callback_s(int type, void *event, void *udata)
    {
    	if (udata)
    		static_cast<GTts*>(udata)->callback(type, event);
    }
private:
    int tid;

    void callback(int type, void *event);
};


struct VoiceInfo {
	std::string identifier;
	std::string name;
	std::string language;
	float quality;
};

void gtts_Init(gevent_Callback callback);
void gtts_Cleanup();
GTts *gtts_Create(lua_State *L,const char *lang,float speed,float pitch);
std::vector<struct VoiceInfo> gtts_GetVoicesInstalled();



