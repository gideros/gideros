#include "../../../tts/source/Common/gtts.h"
#include "lauxlib.h"


/*
function TTSEngine:init()
  require("tts")

-- initialise the tts engine
--  options = {
--    language = "UK", --string,
--    speed = 1.0, -- float
--    pitch = 1.0 -- float
--  }
--  TTSEngine = TTS.new( options )
  -- OR
  TTSEngine = TTS.new( "UK", 1.0, 1.0 )

  TTSEngine.addEvent("onInitComplete", self.onInitComplete, self)
  TTSEngine.addEvent("onInitError", self.onInitError, self)
  -- this callback would require API level 21 (Android 5.0)
  --TTSEngine.addEvent("onUtteranceComplete", self.onUtteranceComplete, self)
end
function TTSEngine:setPitch( pitch )
  TTSEngine:setPitch( pitch ) -- float
end
function TTSEngine:setSpeed( speed )
  TTSEngine.setSpeed( speed ) -- float
end
function TTSEngine:setLanuage( language )
  -- valid params: US or UK only
  -- can add ore later
  TTSEngine.setLanguage( language ) -- string
end
function TTSEngine:stop()
  -- stop speaking
  TTSEngine.stop()
end
function TTSEngine:shutDown()
  -- stop speaking and release system resourxes
  TTSEngine.shutDown()
end
function TTSEngine:speak( text, utteranceID )
  --  TTSEngine.speak( "Say something", utteranceID ) -- requires API level 21
  TTSEngine.speak( "Say something" )
end*/

static const char* TTS_INIT_COMPLETE = "ttsInitComplete";
static const char* TTS_ERROR = "ttsInitError";
static const char* TTS_UTTERANCE_COMPLETE = "ttsUtteranceComplete";
static const char *TTS_UTTERANCE_STARTED = "ttsUtteranceStarted";

// some Lua helper functions
#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif

static void luaL_newweaktable(lua_State *L, const char *mode)
{
    lua_newtable(L);			// create table for instance list
    lua_pushstring(L, mode);
    lua_setfield(L, -2, "__mode");	  // set as weak-value table
    lua_pushvalue(L, -1);             // duplicate table
    lua_setmetatable(L, -2);          // set itself as metatable
}

static void luaL_rawgetptr(lua_State *L, int idx, void *ptr)
{
    idx = abs_index(L, idx);
    lua_pushlightuserdata(L, ptr);
    lua_rawget(L, idx);
}

static void luaL_rawsetptr(lua_State *L, int idx, void *ptr)
{
    idx = abs_index(L, idx);
    lua_pushlightuserdata(L, ptr);
    lua_insert(L, -2);
    lua_rawset(L, idx);
}

static char keyStrong = ' ';
static char keyWeak = ' ';


GTts::GTts(lua_State *L) : L(L)
{
}

GTts::~GTts()
{
}
    void GTts::callback(int type, void *event)
    {
        if (type == GTTS_ERROR_EVENT)
        {
             luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
            luaL_rawgetptr(L, -1, this);

            if (lua_isnil(L, -1))
            {
                lua_pop(L, 2);
                return;
            }

            lua_getfield(L, -1, "dispatchEvent");

            lua_pushvalue(L, -2);

            lua_getfield(L, -1, "__errorEvent");

            lua_pushstring(L, (const char *)event);
            lua_setfield(L, -2, "error");

            lua_call(L, 2, 0);

            lua_pop(L, 2);
        }
        else if ((type == GTTS_UTTERANCE_COMPLETE_EVENT)||(type == GTTS_UTTERANCE_STARTED_EVENT))
        {
             luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
            luaL_rawgetptr(L, -1, this);

            if (lua_isnil(L, -1))
            {
                lua_pop(L, 2);
                return;
            }

            lua_getfield(L, -1, "dispatchEvent");

            lua_pushvalue(L, -2);

            lua_getfield(L, -1, (type == GTTS_UTTERANCE_STARTED_EVENT)?"__utteranceStartedEvent":"__utteranceCompleteEvent");

            lua_pushstring(L, (const char *)event);
            lua_setfield(L, -2, "state");
            lua_pushstring(L, (const char *)event+strlen((const char *)event)+1);
            lua_setfield(L, -2, "utteranceId");

            lua_call(L, 2, 0);

            lua_pop(L, 2);
        }
        else if (type == GTTS_INIT_COMPLETE_EVENT)
        {
             luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
            luaL_rawgetptr(L, -1, this);

            if (lua_isnil(L, -1))
            {
                lua_pop(L, 2);
                return;
            }

            lua_getfield(L, -1, "dispatchEvent");

            lua_pushvalue(L, -2);

            lua_getfield(L, -1, "__initCompleteEvent");

            lua_call(L, 2, 0);

            lua_pop(L, 2);
        }
    }

static int create(lua_State *L)
{
/*
    --  options = {
    --    language = "UK", --string,
    --    speed = 1.0, -- float
    --    pitch = 1.0 -- float
    --  }
    --  TTSEngine = TTS.new( options )
      -- OR
      TTSEngine = TTS.new( "UK", 1.0, 1.0 )

      TTSEngine.addEvent("onInitComplete", self.onInitComplete, self)
      TTSEngine.addEvent("onInitError", self.onInitError, self)
      -- this callback would require API level 21 (Android 5.0)
      --TTSEngine.addEvent("onUtteranceComplete", self.onUtteranceComplete, self)
    end
*/
	const char *lang;
	float speed,pitch;
	if (lua_istable(L,1))
	{
		lua_getfield(L,1,"language");
		lua_getfield(L,1,"speed");
		lua_getfield(L,1,"pitch");
	    lang=luaL_optstring(L,-3,"");
		speed=luaL_optnumber(L,-2,1.0);
		pitch=luaL_optnumber(L,-1,1.0);
		lua_pop(L,3);
	}
	else
	{
	    lang=luaL_optstring(L,1,"");
		speed=luaL_optnumber(L,2,1.0);
		pitch=luaL_optnumber(L,3,1.0);
	}


    GTts *tts = gtts_Create(L,lang,speed,pitch);

    g_pushInstance(L, "TTS", tts->object());

    lua_getglobal(L, "Event");
    lua_getfield(L, -1, "new");
    lua_pushstring(L, TTS_INIT_COMPLETE);
    lua_call(L, 1, 1);
    lua_setfield(L, -3, "__initCompleteEvent");

    lua_getfield(L, -1, "new");
    lua_pushstring(L, TTS_ERROR);
    lua_call(L, 1, 1);
    lua_setfield(L, -3, "__errorEvent");

    lua_getfield(L, -1, "new");
    lua_pushstring(L, TTS_UTTERANCE_COMPLETE);
    lua_call(L, 1, 1);
    lua_setfield(L, -3, "__utteranceCompleteEvent");

    lua_getfield(L, -1, "new");
    lua_pushstring(L, TTS_UTTERANCE_STARTED);
    lua_call(L, 1, 1);
    lua_setfield(L, -3, "__utteranceStartedEvent");

    lua_pop(L, 1);

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, tts);
    lua_pop(L, 1);

    return 1;
}

static int destruct(lua_State *L)
{
    void* ptr = *(void**)lua_touserdata(L, 1);
    GReferenced* object = static_cast<GReferenced*>(ptr);
    GReferenced* proxy = object->proxy();
    proxy->unref();

    return 0;
}

static GTts *getInstanceTts(lua_State *L, int index)
{
    GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "TTS", index));
    GReferenced *proxy = object->proxy();

    return static_cast<GTts*>(proxy);
}

static int speak(lua_State *L)
{
    GTts * tts = getInstanceTts(L, 1);
    const char *v=luaL_checkstring(L,2);
    const char *uid=luaL_optstring(L,3,"");
    lua_pushboolean(L,tts->Speak(v,uid));
    return 1;
}

static int stop(lua_State *L)
{
    GTts * tts = getInstanceTts(L, 1);
    tts->Stop();
    return 0;
}

static int setLanguage(lua_State *L)
{
    GTts * tts = getInstanceTts(L, 1);
    const char *v=luaL_checkstring(L,2);
    lua_pushboolean(L,tts->SetLanguage(v));
    return 1;
}

static int setVoice(lua_State *L)
{
    GTts * tts = getInstanceTts(L, 1);
    const char *v=luaL_checkstring(L,2);
    lua_pushboolean(L,tts->SetVoice(v));
    return 1;
}


static int getVoicesInstalled(lua_State*L)
{
	std::vector<VoiceInfo> voices=gtts_GetVoicesInstalled();    
    lua_createtable(L, voices.size(), 0);
    
    int index = 1;
    for (std::vector<VoiceInfo>::iterator it=voices.begin();it!=voices.end();it++) {
        lua_createtable(L, 0, 4);
        lua_pushstring(L, (*it).identifier.c_str());
        lua_setfield(L, -2, "identifier");
        lua_pushstring(L, (*it).name.c_str());
        lua_setfield(L, -2, "name");
        lua_pushnumber(L, (*it).quality);
        lua_setfield(L, -2, "quality");
        lua_pushstring(L, (*it).language.c_str());
        lua_setfield(L, -2, "language");
        lua_rawseti(L, -2,index++);
    }
    return 1;
}

static int setPitch(lua_State *L)
{
    GTts * tts = getInstanceTts(L, 1);
    float v=luaL_checknumber(L,2);
    lua_pushboolean(L,tts->SetPitch(v));
    return 1;
}

static int getPitch(lua_State *L)
{
    GTts * tts = getInstanceTts(L, 1);
    float pitchMultiplier = tts->GetPitch();
    lua_pushnumber(L, pitchMultiplier);
    return 1;
}

static int setSpeed(lua_State *L)
{
    GTts * tts = getInstanceTts(L, 1);
    float v=luaL_checknumber(L,2);
    lua_pushboolean(L,tts->SetSpeed(v));
    return 1;
}

static int getSpeed(lua_State *L)
{
    GTts * tts = getInstanceTts(L, 1);
    float speed = tts->GetSpeed();
    lua_pushnumber(L, speed);
    return 1;
}

static int setVolume(lua_State *L)
{
    GTts * tts = getInstanceTts(L, 1);
    float v=luaL_checknumber(L,2);
    lua_pushboolean(L,tts->SetVolume(v));
    return 1;
}

static int getVolume(lua_State *L)
{
    GTts * tts = getInstanceTts(L, 1);
    float volume = tts->GetVolume();
    lua_pushnumber(L, volume);
    return 1;
}

static int shutdown(lua_State *L)
{
    GTts * tts = getInstanceTts(L, 1);
	tts->Shutdown();
    return 0;
}

static int loader(lua_State* L)
{
    const luaL_Reg functionlist[] = {
			{"speak", speak }, //string, utterance
    		{"stop", stop},
    		{"setLanguage", setLanguage}, //string
        	{"setVoice", setVoice},
    		{"setPitch", setPitch}, //float
    		{"setSpeed", setSpeed}, //float
	        {"setVolume", setVolume},
        	{"getPitch", getPitch},
    	    {"getSpeed", getSpeed},
	        {"getVolume", getVolume},
	        {"getVoicesInstalled", getVoicesInstalled},
    		{"shutdown", shutdown},
        {NULL, NULL},
    };

    g_createClass(L, "TTS", "EventDispatcher", create, destruct, functionlist);


    lua_getglobal(L, "Event");
    lua_pushstring(L, TTS_ERROR);
    lua_setfield(L, -2, "TTS_ERROR");
    lua_pushstring(L, TTS_INIT_COMPLETE);
    lua_setfield(L, -2, "TTS_INIT_COMPLETE");
    lua_pushstring(L, TTS_UTTERANCE_COMPLETE);
    lua_setfield(L, -2, "TTS_UTTERANCE_COMPLETE");
	lua_pushstring(L, TTS_UTTERANCE_STARTED);
	lua_setfield(L, -2, "TTS_UTTERANCE_STARTED");
    lua_pop(L, 1);

    lua_newtable(L);
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyStrong);

    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);

    lua_getglobal(L, "TTS");
    return 1;
}

static void g_initializePlugin(lua_State *L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, loader);
    lua_setfield(L, -2, "tts");

    lua_pop(L, 2);
    gtts_Init(GTts::callback_s);
}

static void g_deinitializePlugin(lua_State *L)
{
	gtts_Cleanup();
}

REGISTER_PLUGIN("TTS", "1.1")
