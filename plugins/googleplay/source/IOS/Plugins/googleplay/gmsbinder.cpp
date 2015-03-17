#include "gms.h"
#include "gideros.h"
#include <glog.h>
#include <map>
#include <string>

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

static std::map<std::string, std::string> tableToMap(lua_State *L, int index)
{
    luaL_checktype(L, index, LUA_TTABLE);
    
    std::map<std::string, std::string> result;
    
    int t = abs_index(L, index);
    
	lua_pushnil(L);
	while (lua_next(L, t) != 0)
	{
		lua_pushvalue(L, -2);
        std::string key = luaL_checkstring(L, -1);
		lua_pop(L, 1);
		
        std::string value = luaL_checkstring(L, -1);
		
		result[key] = value;
		
		lua_pop(L, 1);
	}
    
    return result;
}

static const char* LOGIN_ERROR = "loginError";
static const char* LOGIN_COMPLETE = "loginComplete";
static const char* LOAD_SCORES_COMPLETE = "loadScoresComplete";
static const char* REPORT_SCORE_COMPLETE = "reportScoreComplete";
static const char* LOAD_ACHIEVEMENTS_COMPLETE = "loadAchievementsComplete";
static const char* REPORT_ACHIEVEMENT_COMPLETE = "reportAchievementComplete";
static const char* STATE_LOADED = "stateLoaded";
static const char* STATE_ERROR = "stateError";
static const char* STATE_CONFLICT = "stateConflict";
static const char* STATE_DELETED = "stateDeleted";

static const char* TODAY = "today";
static const char* WEEK = "week";
static const char* ALL_TIME = "allTime";

static const char* FRIENDS = "friends";
static const char* ALL_PLAYERS = "allPlayers";

static const int UNLOCKED = 0;
static const int REVEALED = 1;
static const int HIDDEN = 2;

static char keyWeak = ' ';

class GooglePlay : public GEventDispatcherProxy
{
public:
    GooglePlay(lua_State *L) : L(L)
    {
        gms_init();
		gms_addCallback(callback_s, this);		
    }
    
    ~GooglePlay()
    {
		gms_removeCallback(callback_s, this);
		gms_cleanup();
    }
	
	bool isAvailable()
	{
		return gms_isAvailable();
	}
	
	void login()
	{
		gms_login();
	}
	
	void logout()
	{
		gms_logout();
	}
	
	void showSettings()
	{
		gms_showSettings();
	}
	
	void showLeaderboard(const char *id)
	{
		gms_showLeaderboard(id);
	}
	
	void reportScore(const char *id, long score, int immediate)
	{
		gms_reportScore(id, score, immediate);
	}
	
	void showAchievements()
	{
		gms_showAchievements();
	}
	
	void reportAchievement(const char *id, int steps, int immediate)
	{
		gms_reportAchievement(id, steps, immediate);
	}
	
	void loadAchievements()
	{
		gms_loadAchievements();
	}
	
	void loadScores(const char *id, int span, int collection, int maxResults)
	{
		gms_loadScores(id, span, collection, maxResults);
	}
		
	void loadPlayerScores(const char *id, int span, int collection, int maxResults)
	{
		gms_loadPlayerScores(id, span, collection, maxResults);
	}
	
	const char* getCurrentPlayer()
	{
		return gms_getCurrentPlayer();
	}
	
	const char* getCurrentPlayerId()
	{
		return gms_getCurrentPlayerId();
	}
	
	void loadState(int key)
	{
		gms_loadState(key);
	}
	
	void updateState(int key, const void* data, size_t size, int immediate)
	{
		gms_updateState(key, data, size, immediate);
	}
	
	void resolveState(int key, const char* ver, const void* data, size_t size)
	{
		gms_resolveState(key, ver, data, size);
	}
	
	void deleteState(int key)
	{
		gms_deleteState(key);
	}

private:
	static void callback_s(int type, void *event, void *udata)
	{
		static_cast<GooglePlay*>(udata)->callback(type, event);
	}
	
	void callback(int type, void *event)
	{
        dispatchEvent(type, event);
	}
	
	void dispatchEvent(int type, void *event)
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
        
        lua_getglobal(L, "Event");
        lua_getfield(L, -1, "new");
        lua_remove(L, -2);
        
        switch (type)
        {
			case GMS_LOGIN_ERROR_EVENT:
                lua_pushstring(L, LOGIN_ERROR);
                break;
			case GMS_LOGIN_COMPLETE_EVENT:
                lua_pushstring(L, LOGIN_COMPLETE);
                break;
			case GMS_LOAD_ACHIEVEMENTS_COMPLETE_EVENT:
                lua_pushstring(L, LOAD_ACHIEVEMENTS_COMPLETE);
                break;
			case GMS_REPORT_ACHIEVEMENT_COMPLETE_EVENT:
                lua_pushstring(L, REPORT_ACHIEVEMENT_COMPLETE);
                break;
			case GMS_LOAD_SCORES_COMPLETE_EVENT:
                lua_pushstring(L, LOAD_SCORES_COMPLETE);
                break;
			case GMS_REPORT_SCORE_COMPLETE_EVENT:
                lua_pushstring(L, REPORT_SCORE_COMPLETE);
                break;
			case GMS_STATE_LOADED_EVENT:
                lua_pushstring(L, STATE_LOADED);
				 break;
			case GMS_STATE_ERROR_EVENT:
                lua_pushstring(L, STATE_ERROR);
				 break;
			case GMS_STATE_CONFLICT_EVENT:
                lua_pushstring(L, STATE_CONFLICT);
                break;
			case GMS_STATE_DELETED_EVENT:
                lua_pushstring(L, STATE_DELETED);
                break;
        }

        lua_call(L, 1, 1);
		
		if (type == GMS_REPORT_ACHIEVEMENT_COMPLETE_EVENT)
        {
            gms_SimpleEvent *event2 = (gms_SimpleEvent*)event;
            
			lua_pushstring(L, event2->id);
			lua_setfield(L, -2, "achievementId");
        }
		else if (type == GMS_LOAD_ACHIEVEMENTS_COMPLETE_EVENT)
        {
            gms_Achievements *event2 = (gms_Achievements*)event;
            
			lua_newtable(L);
			
			for (int i = 0; i < event2->count; ++i)
			{					
				lua_newtable(L);
				
				lua_pushstring(L, event2->achievements[i].id);
				lua_setfield(L, -2, "id");
	
				lua_pushstring(L, event2->achievements[i].name);
				lua_setfield(L, -2, "name");
				
				lua_pushstring(L, event2->achievements[i].description);
				lua_setfield(L, -2, "description");
	
				lua_pushnumber(L, event2->achievements[i].status);
				lua_setfield(L, -2, "status");
				
				lua_pushnumber(L, event2->achievements[i].lastUpdate);
				lua_setfield(L, -2, "lastUpdate");
				
				lua_pushnumber(L, event2->achievements[i].currentSteps);
				lua_setfield(L, -2, "currentSteps");
				
				lua_pushnumber(L, event2->achievements[i].totalSteps);
				lua_setfield(L, -2, "totalSteps");
				
				lua_rawseti(L, -2, i + 1);
			}
			lua_setfield(L, -2, "achievements");
        }
		else if (type == GMS_LOAD_SCORES_COMPLETE_EVENT)
        {
            gms_Leaderboard *event2 = (gms_Leaderboard*)event;
			
			lua_pushstring(L, event2->id);
			lua_setfield(L, -2, "leaderboardId");
			
			lua_pushstring(L, event2->name);
			lua_setfield(L, -2, "name");
            
			lua_newtable(L);
			
			for (int i = 0; i < event2->count; ++i)
			{				
				lua_newtable(L);
				
				lua_pushstring(L, event2->scores[i].rank);
				lua_setfield(L, -2, "rank");
	
				lua_pushstring(L, event2->scores[i].score);
				lua_setfield(L, -2, "score");
				
				lua_pushstring(L, event2->scores[i].name);
				lua_setfield(L, -2, "name");
				
				lua_pushstring(L, event2->scores[i].playerId);
				lua_setfield(L, -2, "playerId");
	
				lua_pushnumber(L, event2->scores[i].timestamp);
				lua_setfield(L, -2, "timestamp");
				
				lua_rawseti(L, -2, i + 1);
			}
			lua_setfield(L, -2, "scores");
        }
		else if (type == GMS_STATE_LOADED_EVENT)
		{
            gms_StateLoaded *event2 = (gms_StateLoaded*)event;
		
			lua_pushnumber(L, event2->key);
			lua_setfield(L, -2, "key");
			
			lua_pushboolean(L, (bool)event2->fresh);
			lua_setfield(L, -2, "isFresh");

			lua_pushlstring(L, (const char*)event2->data, event2->size);
			lua_setfield(L, -2, "data");
		}
		else if (type == GMS_STATE_ERROR_EVENT)
		{
            gms_StateError *event2 = (gms_StateError*)event;
			
			lua_pushstring(L, event2->error);
			lua_setfield(L, -2, "error");
		
			lua_pushnumber(L, event2->key);
			lua_setfield(L, -2, "key");
		}
		else if (type == GMS_STATE_DELETED_EVENT)
		{
            gms_StateDeleted *event2 = (gms_StateDeleted*)event;
			
			lua_pushnumber(L, event2->key);
			lua_setfield(L, -2, "key");
		}
		else if (type == GMS_STATE_CONFLICT_EVENT)
		{
            gms_StateConflict *event2 = (gms_StateConflict*)event;
		
			lua_pushnumber(L, event2->key);
			lua_setfield(L, -2, "key");
			
			lua_pushstring(L, event2->ver);
			lua_setfield(L, -2, "version");
			
			lua_pushlstring(L, (const char*)event2->localData, event2->localSize);
			lua_setfield(L, -2, "localData");
			
			lua_pushlstring(L, (const char*)event2->serverData, event2->serverSize);
			lua_setfield(L, -2, "serverData");
		}

		lua_call(L, 2, 0);
		
		lua_pop(L, 2);
	}

private:
    lua_State *L;
    bool initialized_;
};

static int destruct(lua_State* L)
{
	void *ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	GooglePlay *gms = static_cast<GooglePlay*>(object->proxy());
	
	gms->unref();
	
	return 0;
}

static GooglePlay *getInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "GooglePlay", index));
	GooglePlay *gms = static_cast<GooglePlay*>(object->proxy());
    
	return gms;
}

static int isAvailable(lua_State *L)
{
	GooglePlay *gms = getInstance(L, 1);
	bool result = gms->isAvailable();
	lua_pushboolean(L, result);
    return 1;
}

static int login(lua_State *L)
{
	GooglePlay *gms = getInstance(L, 1);
	gms->login();
    return 0;
}

static int logout(lua_State *L)
{
	GooglePlay *gms = getInstance(L, 1);
	gms->logout();
    return 0;
}

static int showSettings(lua_State *L)
{
	GooglePlay *gms = getInstance(L, 1);
	gms->showSettings();
    return 0;
}

static int showLeaderboard(lua_State *L)
{
	GooglePlay *gms = getInstance(L, 1);
	const char *id = luaL_checkstring(L, 2);
	gms->showLeaderboard(id);
    return 0;
}

static int reportScore(lua_State *L)
{
	GooglePlay *gms = getInstance(L, 1);
	const char *id = luaL_checkstring(L, 2);
	long score = luaL_checklong(L, 3);
	int immediate = lua_toboolean(L, 4);
	gms->reportScore(id, score, immediate);
    return 0;
}

static int showAchievements(lua_State *L)
{
	GooglePlay *gms = getInstance(L, 1);
	gms->showAchievements();
    return 0;
}

static int reportAchievement(lua_State* L)
{
	GooglePlay *gms = getInstance(L, 1);

	const char* id = luaL_checkstring(L, 2);
	int numSteps = 0;
	int immediate = 0;
	if (!lua_isnoneornil(L, 3))
	{
		if(lua_isboolean(L, 3))
		{
			immediate = lua_toboolean(L, 3);
		}
		else
		{
			numSteps = luaL_checkinteger(L, 3);
			immediate = lua_toboolean(L, 4);
		}
	}
	
	gms->reportAchievement(id, numSteps, immediate);
	
	return 0;
}

static int loadAchievements(lua_State *L)
{
	GooglePlay *gms = getInstance(L, 1);
	gms->loadAchievements();
    return 0;
}

static int luaL_checktimescope(lua_State* L, int index)
{
    int timeScope = 2;
    const char* timeScopeStr = luaL_checkstring(L, index);

    if (strcmp(timeScopeStr, TODAY) == 0)
    {
        timeScope = 0;
    }
    else if (strcmp(timeScopeStr, WEEK) == 0)
    {
        timeScope = 1;
    }
    else if (strcmp(timeScopeStr, ALL_TIME) == 0)
    {
        timeScope = 2;
    }
    else
    {
        luaL_error(L, "Parameter 'timeScope' must be one of the accepted values.");
    }
    return timeScope;
}

static int luaL_checkplayerscope(lua_State* L, int index)
{
    int playerScope = 0;
    const char* playScopeStr = luaL_checkstring(L, index);

    if (strcmp(playScopeStr, FRIENDS) == 0)
    {
        playerScope = 1;
    }
    else if (strcmp(playScopeStr, ALL_PLAYERS) == 0)
    {
        playerScope = 0;
    }
    else
    {
        luaL_error(L, "Parameter 'playerScope' must be one of the accepted values.");
    }
    return playerScope;
}

static int loadScores(lua_State* L)
{
	GooglePlay *gms = getInstance(L, 1);

    int span = 2;
	int collection = 0;
    int maxEntries = 25; // Default

	const char *id = luaL_checkstring(L, 2);

	if (!lua_isnoneornil(L, 3))
		span = luaL_checktimescope(L, 3);

    if (!lua_isnoneornil(L, 4))
   		collection = luaL_checkplayerscope(L, 4);

    if (!lua_isnoneornil(L, 5))
   		maxEntries= luaL_checknumber(L, 5);

    gms->loadScores(id, span, collection, maxEntries);

	return 0;
}

static int loadPlayerScores(lua_State* L)
{
	GooglePlay *gms = getInstance(L, 1);

    int span = 2;
	int collection = 0;
    int maxEntries = 25; // Default

	const char *id = luaL_checkstring(L, 2);

	if (!lua_isnoneornil(L, 3))
		span = luaL_checktimescope(L, 3);

    if (!lua_isnoneornil(L, 4))
   		collection = luaL_checkplayerscope(L, 4);

    if (!lua_isnoneornil(L, 5))
   		maxEntries= luaL_checknumber(L, 5);

    gms->loadPlayerScores(id, span, collection, maxEntries);

	return 0;
}

static int getCurrentPlayer(lua_State *L)
{
	GooglePlay *gms = getInstance(L, 1);
		
	const char *name = gms->getCurrentPlayer();
	
	lua_pushstring(L, name);
	
    return 1;
}

static int getCurrentPlayerId(lua_State *L)
{
	GooglePlay *gms = getInstance(L, 1);
		
	const char *name = gms->getCurrentPlayerId();
	
	lua_pushstring(L, name);
	
    return 1;
}

static int loadState(lua_State *L)
{
	GooglePlay *gms = getInstance(L, 1);
	
	int key = luaL_checknumber(L, 2);
	
	gms->loadState(key);
	
    return 0;
}

static int updateState(lua_State *L)
{
	GooglePlay *gms = getInstance(L, 1);
	
	int key = luaL_checknumber(L, 2);
	
	size_t size;
    const void *data = luaL_checklstring(L, 3, &size);
	
    int immediate = 0;
	if (!lua_isnoneornil(L, 4))
	{
		if(lua_isboolean(L, 4))
		{
			immediate = lua_toboolean(L, 4);
		}
	}
	
	gms->updateState(key, data, size, immediate);
	
    return 0;
}

static int resolveState(lua_State *L)
{
	GooglePlay *gms = getInstance(L, 1);
	
	int key = luaL_checknumber(L, 2);
	
	const char* ver = luaL_checkstring(L, 3);
	
	size_t size;
    const void *data = luaL_checklstring(L, 4, &size);
	
	gms->resolveState(key, ver, data, size);
	
    return 0;
}

static int deleteState(lua_State *L)
{
	GooglePlay *gms = getInstance(L, 1);
	
	int key = luaL_checknumber(L, 2);
	
	gms->deleteState(key);
	
    return 0;
}

static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
        {"isAvailable", isAvailable},
        {"login", login},
        {"logout", logout},
        {"showLeaderboard", showLeaderboard},
        {"reportScore", reportScore},
        {"showAchievements", showAchievements},
        {"reportAchievement", reportAchievement},
        {"loadAchievements", loadAchievements},
        {"loadScores", loadScores},
        {"loadPlayerScores", loadPlayerScores},
        {"getCurrentPlayer", getCurrentPlayer},
        {"getCurrentPlayerId", getCurrentPlayerId},
		{"loadState", loadState},
        {"updateState", updateState},
        {"resolveState", resolveState},
        {"deleteState", deleteState},
		{NULL, NULL},
	};
    
    g_createClass(L, "GooglePlay", "EventDispatcher", NULL, destruct, functionlist);
    
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    
	lua_getglobal(L, "Event");
	lua_pushstring(L, LOGIN_ERROR);
	lua_setfield(L, -2, "LOGIN_ERROR");
	lua_pushstring(L, LOGIN_COMPLETE);
	lua_setfield(L, -2, "LOGIN_COMPLETE");
	lua_pushstring(L, LOAD_SCORES_COMPLETE);
	lua_setfield(L, -2, "LOAD_SCORES_COMPLETE");
	lua_pushstring(L, REPORT_SCORE_COMPLETE);
	lua_setfield(L, -2, "REPORT_SCORE_COMPLETE");
	lua_pushstring(L, LOAD_ACHIEVEMENTS_COMPLETE);
	lua_setfield(L, -2, "LOAD_ACHIEVEMENTS_COMPLETE");
	lua_pushstring(L, REPORT_ACHIEVEMENT_COMPLETE);
	lua_setfield(L, -2, "REPORT_ACHIEVEMENT_COMPLETE");
	lua_pushstring(L, STATE_LOADED);
	lua_setfield(L, -2, "STATE_LOADED");
	lua_pushstring(L, STATE_ERROR);
	lua_setfield(L, -2, "STATE_ERROR");
	lua_pushstring(L, STATE_CONFLICT);
	lua_setfield(L, -2, "STATE_CONFLICT");
	lua_pushstring(L, STATE_DELETED);
	lua_setfield(L, -2, "STATE_DELETED");
	lua_pop(L, 1);
	
	lua_getglobal(L, "GooglePlay");
	lua_pushstring(L, TODAY);
	lua_setfield(L, -2, "TODAY");
	lua_pushstring(L, WEEK);
	lua_setfield(L, -2, "WEEK");
	lua_pushstring(L, ALL_TIME);
	lua_setfield(L, -2, "ALL_TIME");
	lua_pushstring(L, FRIENDS);
	lua_setfield(L, -2, "FRIENDS");
	lua_pushstring(L, ALL_PLAYERS);
	lua_setfield(L, -2, "ALL_PLAYERS");
	lua_pushnumber(L, UNLOCKED);
	lua_setfield(L, -2, "UNLOCKED");
	lua_pushnumber(L, REVEALED);
	lua_setfield(L, -2, "REVEALED");
	lua_pushnumber(L, HIDDEN);
	lua_setfield(L, -2, "HIDDEN");
	lua_pop(L, 1);

    GooglePlay *gms = new GooglePlay(L);
	g_pushInstance(L, "GooglePlay", gms->object());
    
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, gms);
	lua_pop(L, 1);
    
	lua_pushvalue(L, -1);
	lua_setglobal(L, "googleplay");
    
    return 1;
}
    
static void g_initializePlugin(lua_State *L)
{
    lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "googleplay");
	
	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
    
}

REGISTER_PLUGIN("GooglePlay", "1.0")
