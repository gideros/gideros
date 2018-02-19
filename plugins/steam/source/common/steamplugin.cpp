// steamplugin.cpp : Defines the exported functions for the DLL application.
//

#ifdef __APPLE__
#include <TargetConditionals.h>
#else
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#endif

#ifndef G_UNUSED
#define G_UNUSED(x) (void)(x)
#endif


#include "gideros.h"
#include "lua.h"
//#include "luautil.h"
#include "lauxlib.h"
#include "steam_api.h"

// EXTRACT from lua int64 lib to push/get int64 values
#define Int		long long
#define FMT		"%lld"
#define atoI		atoll
#define MYNAME		"int64"
#define MYTYPE		MYNAME

static Int getInt(lua_State *L, int i)
{
	switch (lua_type(L, i))
	{
	case LUA_TNUMBER:
		return luaL_checknumber(L, i);
	case LUA_TSTRING:
		return atoI(luaL_checkstring(L, i));
	default:
		return *((Int*)luaL_checkudata(L, i, MYTYPE));
	}
}

static int pushInt(lua_State *L, Int z)
{
	Int *p = (Int*)lua_newuserdata(L, sizeof(Int));
	*p = z;
	luaL_getmetatable(L, MYTYPE);
	lua_setmetatable(L, -2);
	return 1;
}

static int lua_print(lua_State* L, const char* str)
{
	lua_getglobal(L, "_G");
	lua_getfield(L, -1, "print");
	lua_pushstring(L, str);
	lua_call(L, 1, 0);
	lua_pop(L, 1);
	return 0;
}


class LuaCallbackBase {
public:
	virtual ~LuaCallbackBase() { };
	static lua_State *statL;
	static std::vector<LuaCallbackBase *> cleanup;
};

template <typename T> class LuaCallback : LuaCallbackBase
{
private:
	int luacallback;
public:
	LuaCallback(lua_State *L, int arg, SteamAPICall_t hSteamAPICall) {
		lua_pushvalue(L, arg);
		luacallback = lua_ref(L, true);
		lua_pop(L, 1);
		steamcb.Set(hSteamAPICall, this,
			&LuaCallback::OnResult);
	}
	void OnResult(T *r, bool io) {
		lua_getref(statL,luacallback);
		if (io)
			lua_pushnil(statL);
		else
			UnpackResult(statL, r);
		lua_call(statL, 1, 0);
		cleanup.push_back(this);
	}
	virtual void UnpackResult(lua_State *L, T* r) = 0;
	virtual ~LuaCallback() {
		lua_unref(statL,luacallback);
	}
	CCallResult<LuaCallback, T> steamcb;
};

std::vector<LuaCallbackBase*> LuaCallbackBase::cleanup;
lua_State *LuaCallbackBase::statL;
static g_id tickerId;

static void tick(int type, void *event, void *udata) {
	if (type == GEVENT_PRE_TICK_EVENT)
	{
		SteamAPI_RunCallbacks();
		for (std::vector<LuaCallbackBase *>::iterator it = LuaCallbackBase::cleanup.begin(); it != LuaCallbackBase::cleanup.end(); it++)
			delete *it;
		LuaCallbackBase::cleanup.clear();
	}
}


static int getSteamID(lua_State *L) {
	ISteamUser *user = SteamUser();
	if (user == NULL) {
		lua_pushnil(L);
		return 1;
	}
	CSteamID c=user->GetSteamID();
	pushInt(L, c.ConvertToUint64());
	return 1;
}

static int getAppID(lua_State *L) {
	int id = SteamUtils()->GetAppID();
	lua_pushinteger(L, id);
	return 1;
}

class UploadScoreLuaCB : LuaCallback<LeaderboardScoreUploaded_t> {
	void UnpackResult(lua_State *L, LeaderboardScoreUploaded_t* r) {
		if (r->m_bSuccess)
		{
			lua_newtable(L);
			lua_pushinteger(L, r->m_nGlobalRankNew);
			lua_setfield(L, -2, "newRank");
			lua_pushinteger(L, r->m_nGlobalRankPrevious);
			lua_setfield(L, -2, "previousRank");
			lua_pushboolean(L, r->m_bScoreChanged);
			lua_setfield(L, -2, "scoreChanged");
		}
		else
			lua_pushboolean(L,false);
	}
public:
	UploadScoreLuaCB(lua_State *L, int arg, SteamAPICall_t hSteamAPICall) : LuaCallback(L, arg, hSteamAPICall) { }
};


static int uploadScore(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);	
	int score = luaL_checkinteger(L, 2);
	lua_getfield(L, 1, "leaderboardId");
	Int lid = getInt(L, -1);
	lua_pop(L, 1);
	new UploadScoreLuaCB(L, 3,
		SteamUserStats()->UploadLeaderboardScore(lid, k_ELeaderboardUploadScoreMethodKeepBest, score, NULL, 0));
	return 0;
}

class FindLeaderboardLuaCB : LuaCallback<LeaderboardFindResult_t> {
	void UnpackResult(lua_State *L, LeaderboardFindResult_t* r) {
		if (r->m_bLeaderboardFound)
		{
			const luaL_Reg functionlist[] = {
				{ "uploadScore", uploadScore },
				{ NULL, NULL },
			};

			lua_newtable(L);
			luaL_register(L, NULL, functionlist);
			pushInt(L, r->m_hSteamLeaderboard);
			lua_setfield(L, -2, "leaderboardId");
		}
		else
			lua_pushnil(L);
	}
public:
	FindLeaderboardLuaCB(lua_State *L, int arg, SteamAPICall_t hSteamAPICall) : LuaCallback(L, arg, hSteamAPICall) { }
};


static int findLeaderboard(lua_State *L) {
	new FindLeaderboardLuaCB(L,2,
		SteamUserStats()->FindLeaderboard(luaL_checkstring(L,1)));
	return 0;
}

static int loader(lua_State* L)
{
	const luaL_Reg functionlist[] = {
		{ "getSteamID", getSteamID },
		{ "getAppID", getAppID },
		{ "findLeaderboard", findLeaderboard},
		{ NULL, NULL },
	};

	lua_newtable(L);
	luaL_register(L,NULL, functionlist);

	return 1;
}

static void g_initializePlugin(lua_State* L)
{
	LuaCallbackBase::statL = L;
	SteamAPI_Init();
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "steam");

	lua_pop(L, 2);
	tickerId=gevent_AddCallback(&tick, NULL);
}

static void g_deinitializePlugin(lua_State *L)
{
	gevent_RemoveCallbackWithGid(tickerId);
	SteamAPI_Shutdown();
	G_UNUSED(L);
}


REGISTER_PLUGIN("Steam", "1.0")
