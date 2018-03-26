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

#define ICONST(n,i) lua_pushinteger(L,i); lua_setfield(L,-2,n);
#define SCONST(n,i) lua_pushstring(L,i); lua_setfield(L,-2,n);

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
	static void revoke(LuaCallbackBase *c) {
		for (std::vector<LuaCallbackBase *>::iterator it = LuaCallbackBase::cleanup.begin(); it != LuaCallbackBase::cleanup.end(); it++)
			if ((*it)==c) return;
		cleanup.push_back(c);
	};
};

template <typename T> class LuaCallreturn : public LuaCallbackBase
{
private:
	int luacallback;
public:
	LuaCallreturn(lua_State *L, int arg, SteamAPICall_t hSteamAPICall) {
		lua_pushvalue(L, arg);
		luacallback = lua_ref(L, true);
		lua_pop(L, 1);
		steamcb.Set(hSteamAPICall, this,
			&LuaCallreturn::OnResult);
	}
	void OnResult(T *r, bool io) {
		lua_getref(statL,luacallback);
		if (io)
			lua_pushnil(statL);
		else
			UnpackResult(statL, r);
		lua_call(statL, 1, 0);
		LuaCallbackBase::revoke((LuaCallbackBase *)this);
	}
	virtual void UnpackResult(lua_State *L, T* r) = 0;
	virtual ~LuaCallreturn() {
		lua_unref(statL,luacallback);
	}
	CCallResult<LuaCallreturn, T> steamcb;
};

template <typename T> class LuaCallback : public LuaCallbackBase
{
private:
	int luacallback;
public:
	CCallback<LuaCallback, T> steamcb;
	LuaCallback(lua_State *L, int arg) : steamcb(this,&LuaCallback::OnResult) {
		lua_pushvalue(L, arg);
		luacallback = lua_ref(L, true);
		lua_pop(L, 1);
	}
	void OnResult(T *r) {
		lua_getref(statL,luacallback);
		bool last=UnpackResult(statL, r);
		lua_call(statL, 1, 0);
		if (last)
			LuaCallbackBase::revoke((LuaCallbackBase *)this);
	}
	virtual bool UnpackResult(lua_State *L, T* r) = 0;
	virtual ~LuaCallback() {
		lua_unref(statL,luacallback);
	}
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

static int getCurrentGameLanguage(lua_State *L) {
	lua_pushstring(L, SteamApps()->GetCurrentGameLanguage());
	return 1;
}

static int getSteamName(lua_State *L) {
	lua_pushstring(L,SteamFriends()->GetPersonaName());
	return 1;
}

static int getFriendName(lua_State *L) {
	CSteamID u((uint64)getInt(L,1));
	lua_pushstring(L,SteamFriends()->GetFriendPersonaName(u));
	return 1;
}

static int getFriendState(lua_State *L) {
	CSteamID u((uint64)getInt(L,1));
	lua_pushinteger(L,SteamFriends()->GetFriendPersonaState(u));
	return 1;
}

static int getPlayerNickname(lua_State *L) {
	CSteamID u((uint64)getInt(L,1));
	lua_pushstring(L,SteamFriends()->GetPlayerNickname(u));
	return 1;
}

/*// returns the sort method of the leaderboard
virtual ELeaderboardSortMethod GetLeaderboardSortMethod( SteamLeaderboard_t hSteamLeaderboard ) = 0;
// returns the display type of the leaderboard
virtual ELeaderboardDisplayType GetLeaderboardDisplayType( SteamLeaderboard_t hSteamLeaderboard ) = 0;
*/
class DownloadLeaderboardLuaCB : LuaCallreturn<LeaderboardScoresDownloaded_t> {
	void UnpackResult(lua_State *L, LeaderboardScoresDownloaded_t* r) {
		lua_newtable(L);
		for ( int index = 0; index < r->m_cEntryCount; index++ )
		{
			LeaderboardEntry_t l;
			int32 details[16];		// Handle up to 16 details XXX (enough ?)
			SteamUserStats()->GetDownloadedLeaderboardEntry( r->m_hSteamLeaderboardEntries, index, &l, details, 16);
			lua_newtable(L);
			lua_pushinteger(L, l.m_nGlobalRank);
			lua_setfield(L, -2, "rank");
			lua_pushinteger(L, l.m_nScore);
			lua_setfield(L, -2, "score");
			pushInt(L,l.m_steamIDUser.ConvertToUint64());
			lua_setfield(L, -2, "userId");
			lua_newtable(L);
			for (int k=0;k<l.m_cDetails;k++)
			{
				lua_pushinteger(L,details[k]);
				lua_rawseti(L,-2,k+1);
			}
			lua_setfield(L, -2, "details");
			lua_rawseti(L,-2,index+1);
		}
	}
public:
	DownloadLeaderboardLuaCB(lua_State *L, int arg, SteamAPICall_t hSteamAPICall) : LuaCallreturn(L, arg, hSteamAPICall) { }
};

static int downloadLeaderboardEntries(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	int mode = luaL_checkinteger(L, 2);
	int start = luaL_checkinteger(L, 3);
	int end = luaL_checkinteger(L, 4);
	lua_getfield(L, 1, "leaderboardId");
	Int lid = getInt(L, -1);
	lua_pop(L, 1);
	new DownloadLeaderboardLuaCB(L, 5,
		SteamUserStats()->DownloadLeaderboardEntries(lid, (ELeaderboardDataRequest) mode,start,end));
	return 0;
}

static int downloadLeaderboardEntriesForUsers(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	luaL_checktype(L, 2, LUA_TTABLE);
	int uc=lua_objlen(L,2);
	CSteamID *us=new CSteamID[uc];
	for (int k=0;k<uc;k++)
	{
		lua_rawgeti(L,2,k+1);
		us[k].SetFromUint64(getInt(L,-1));
		lua_pop(L,1);
	}
	int mode = luaL_checkinteger(L, 2);
	int start = luaL_checkinteger(L, 3);
	int end = luaL_checkinteger(L, 4);
	lua_getfield(L, 1, "leaderboardId");
	Int lid = getInt(L, -1);
	lua_pop(L, 1);
	new DownloadLeaderboardLuaCB(L, 3,
		SteamUserStats()->DownloadLeaderboardEntriesForUsers(lid, us,uc));
	delete us;
	return 0;
}

static int leaderboardEntriesCount(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_getfield(L, 1, "leaderboardId");
	Int lid = getInt(L, -1);
	lua_pop(L, 1);
	lua_pushinteger(L,SteamUserStats()->GetLeaderboardEntryCount(lid));
	return 1;
}

static int leaderboardName(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_getfield(L, 1, "leaderboardId");
	Int lid = getInt(L, -1);
	lua_pop(L, 1);
	lua_pushstring(L,SteamUserStats()->GetLeaderboardName(lid));
	return 1;
}

class UploadScoreLuaCB : LuaCallreturn<LeaderboardScoreUploaded_t> {
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
	UploadScoreLuaCB(lua_State *L, int arg, SteamAPICall_t hSteamAPICall) : LuaCallreturn(L, arg, hSteamAPICall) { }
};

static int uploadScore(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);	
	int mode= luaL_checkinteger(L, 2);
	int score = luaL_checkinteger(L, 3);
	luaL_checktype(L, 4, LUA_TTABLE);
	int dc=lua_objlen(L,4);
	int32 *details=new int32[dc];
	for (int k=0;k<dc;k++) {
		lua_rawgeti(L,4,k+1);
		details[k]=luaL_checkinteger(L,-1);
		lua_pop(L,1);
	}
	lua_getfield(L, 1, "leaderboardId");
	Int lid = getInt(L, -1);
	lua_pop(L, 1);
	new UploadScoreLuaCB(L, 5,
		SteamUserStats()->UploadLeaderboardScore(lid, (ELeaderboardUploadScoreMethod)mode, score, details,dc));
	delete details;
	return 0;
}

class FindLeaderboardLuaCB : LuaCallreturn<LeaderboardFindResult_t> {
	void UnpackResult(lua_State *L, LeaderboardFindResult_t* r) {
		if (r->m_bLeaderboardFound)
		{
			const luaL_Reg functionlist[] = {
				{ "uploadScore", uploadScore },
				{ "downloadEntries", downloadLeaderboardEntries },
				{ "downloadEntriesForUsers", downloadLeaderboardEntriesForUsers },
				{ "getEntriesCount", leaderboardEntriesCount },
				{ "getName", leaderboardName },
				{ NULL, NULL },
			};

			lua_newtable(L);
			luaL_register(L, NULL, functionlist);
			pushInt(L, r->m_hSteamLeaderboard);
			lua_setfield(L, -2, "leaderboardId");
			ICONST("GLOBAL",0);
			ICONST("GLOBAL_AROUND_USER",1);
			ICONST("FRIENDS",2);
			ICONST("USERS",3);
			ICONST("NONE",0);
			ICONST("KEEP_BEST",1);
			ICONST("FORCE_UPDATE",2);
		}
		else
			lua_pushnil(L);
	}
public:
	FindLeaderboardLuaCB(lua_State *L, int arg, SteamAPICall_t hSteamAPICall) : LuaCallreturn(L, arg, hSteamAPICall) { }
};


static int findLeaderboard(lua_State *L) {
	new FindLeaderboardLuaCB(L,2,
		SteamUserStats()->FindLeaderboard(luaL_checkstring(L,1)));
	return 0;
}

static int findOrCreateLeaderboard(lua_State *L) {
	new FindLeaderboardLuaCB(L,4,
		SteamUserStats()->FindOrCreateLeaderboard(luaL_checkstring(L,1),
				(ELeaderboardSortMethod) luaL_checkinteger(L,2),
				(ELeaderboardDisplayType) luaL_checkinteger(L,3)
				));
	return 0;
}

//Achievements/Stats
class UserStatsReceivedLuaCB : public LuaCallback<UserStatsReceived_t> {
	bool UnpackResult(lua_State *L, UserStatsReceived_t* r) {
		lua_newtable(L);
		pushInt(L,r->m_nGameID);	lua_setfield(L,-2,"gameId");
		lua_pushinteger(L,(int)r->m_eResult); 	lua_setfield(L,-2,"result");
		pushInt(L,r->m_steamIDUser.ConvertToUint64());	lua_setfield(L,-2,"userId");
		return true;
	}
public:
	UserStatsReceivedLuaCB(lua_State *L, int arg) : LuaCallback(L, arg) { }
};

static int requestCurrentStats(lua_State *L) {
	LuaCallbackBase *t=new UserStatsReceivedLuaCB(L,1);
	if (SteamUserStats()->RequestCurrentStats())
		lua_pushboolean(L,true);
	else {
		lua_pushboolean(L,false);
		delete t;
	}
	return 1;
}

class UserStatsStoredLuaCB : public LuaCallback<UserStatsStored_t> {
	bool UnpackResult(lua_State *L, UserStatsStored_t* r) {
		lua_newtable(L);
		pushInt(L,r->m_nGameID);	lua_setfield(L,-2,"gameId");
		lua_pushinteger(L,(int)r->m_eResult); 	lua_setfield(L,-2,"result");
		LuaCallbackBase::revoke(token);
		return true;
	}
	LuaCallbackBase *token;
public:
	UserStatsStoredLuaCB(lua_State *L, int arg, LuaCallbackBase *token) : LuaCallback(L, arg) { this->token=token; }
};

class UserAchievementStoredLuaCB : public LuaCallback<UserAchievementStored_t> {
	bool UnpackResult(lua_State *L, UserAchievementStored_t* r) {
		lua_newtable(L);
		pushInt(L,r->m_nGameID);	lua_setfield(L,-2,"gameId");
		lua_pushstring(L,r->m_rgchAchievementName); lua_setfield(L,-2,"achievementName");
		lua_pushinteger(L,r->m_nCurProgress); 	lua_setfield(L,-2,"currentProgress");
		lua_pushinteger(L,r->m_nMaxProgress); 	lua_setfield(L,-2,"maxProgress");
		return false;
	}
public:
	UserAchievementStoredLuaCB(lua_State *L, int arg) : LuaCallback(L, arg) { }
};

static int storeStats(lua_State *L) {
	LuaCallbackBase *token=new UserAchievementStoredLuaCB(L,2);
	LuaCallbackBase *t2=new UserStatsStoredLuaCB(L,1,token);
	if (!SteamUserStats()->StoreStats())
	{
		lua_pushboolean(L,false);
		delete t2;
		delete token;
	}
	else
		lua_pushboolean(L,true);
	return 1;
}

static int setIntStat(lua_State *L) {
	lua_pushboolean(L,SteamUserStats()->SetStat(luaL_checkstring(L,1),(int32) luaL_checkinteger(L,2)));
	return 1;
}

static int setFloatStat(lua_State *L) {
	lua_pushboolean(L,SteamUserStats()->SetStat(luaL_checkstring(L,1),(float) luaL_checknumber(L,2)));
	return 1;
}

static int setAchievement(lua_State *L) {
	lua_pushboolean(L,SteamUserStats()->SetAchievement(luaL_checkstring(L,1)));
	return 1;
}

static int getIntStat(lua_State *L) {
	int32 v;
	bool res=SteamUserStats()->GetStat(luaL_checkstring(L,1),&v);
	if (res)
		lua_pushinteger(L,v);
	else
		lua_pushnil(L);
	return 1;
}

static int getFloatStat(lua_State *L) {
	float v;
	bool res=SteamUserStats()->GetStat(luaL_checkstring(L,1),&v);
	if (res)
		lua_pushnumber(L,v);
	else
		lua_pushnil(L);
	return 1;
}

static int getAchievement(lua_State *L) {
	bool v;
	uint32 utime;
	bool res=SteamUserStats()->GetAchievementAndUnlockTime(luaL_checkstring(L,1),&v,&utime);
	if (res) {
		lua_pushboolean(L,v);
		lua_pushinteger(L,(int)utime);
	}
	else
	{
		lua_pushnil(L);
		lua_pushnil(L);
	}
	return 2;
}

static int indicateAchievementProgress(lua_State *L) {
	LuaCallbackBase *token=new UserAchievementStoredLuaCB(L,5);
	LuaCallbackBase *t2=new UserStatsStoredLuaCB(L,4,token);
	if (!SteamUserStats()->IndicateAchievementProgress(luaL_checkstring(L,1),luaL_checkinteger(L,2),luaL_checkinteger(L,3)))
	{
		lua_pushboolean(L,false);
		delete t2;
		delete token;
	}
	else
		lua_pushboolean(L,true);
	return 1;
}

static int getNumAchievements(lua_State *L) {
	lua_pushinteger(L,SteamUserStats()->GetNumAchievements());
	return 1;
}

static int getAchievementName(lua_State *L) {
	lua_pushstring(L,SteamUserStats()->GetAchievementName(luaL_checkinteger(L,1)));
	return 1;
}

static int getAchievementDisplayAttribute(lua_State *L) {
	lua_pushstring(L,SteamUserStats()->GetAchievementDisplayAttribute(luaL_checkstring(L,1),luaL_checkstring(L,2)));
	return 1;
}

static int getAchievementAchievedPercent(lua_State *L) {
	float v;
	bool res=SteamUserStats()->GetAchievementAchievedPercent(luaL_checkstring(L,1),&v);
	if (res)
		lua_pushnumber(L,v);
	else
		lua_pushnil(L);
	return 1;
}

static int loader(lua_State* L)
{
	const luaL_Reg functionlist[] = {
		{ "getSteamID", getSteamID },
		{ "getAppID", getAppID },
		{ "getPlayerNickname", getPlayerNickname },
		{ "getSteamName", getSteamName },
		{ "getFriendName", getFriendName },
		{ "getFriendState", getFriendState },
		{ "findLeaderboard", findLeaderboard},
		{ "findOrCreateLeaderboard", findOrCreateLeaderboard},
		{ "requestCurrentStats", requestCurrentStats},
		{ "storeStats", storeStats},
		{ "setIntStat", setIntStat },
		{ "setFloatStat", setFloatStat },
		{ "setAchievement", setAchievement },
		{ "getIntStat", getIntStat },
		{ "getFloatStat", getFloatStat },
		{ "getAchievement", getAchievement },
		{ "indicateAchievementProgress", indicateAchievementProgress },
		{ "getNumAchievements", getNumAchievements },
		{ "getAchievementName", getAchievementName },
		{ "getAchievementDisplayAttribute", getAchievementDisplayAttribute },
		{ "getAchievementAchievedPercent", getAchievementAchievedPercent },
		{ "getCurrentGameLanguage", getCurrentGameLanguage },
		{ NULL, NULL },
	};

	lua_newtable(L);
	luaL_register(L,NULL, functionlist);
	ICONST("LEADERBOARD_ASCENDING",1);
	ICONST("LEADERBOARD_DESCENDING",2);
	ICONST("LEADERBOARD_NUMERIC",1);
	ICONST("LEADERBOARD_TIME_SECONDS",2);
	ICONST("LEADERBOARD_TIME_MILLISECONDS",3);

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
