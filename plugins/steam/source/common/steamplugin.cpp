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

static int loader(lua_State* L)
{
	const luaL_Reg functionlist[] = {
		{ "getSteamID", getSteamID },
		{ NULL, NULL },
	};

	lua_newtable(L);
	luaL_register(L,NULL, functionlist);

	return 1;
}

static void g_initializePlugin(lua_State* L)
{
	SteamAPI_Init();
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "steam");

	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
	SteamAPI_Shutdown();
	G_UNUSED(L);
}


REGISTER_PLUGIN("Steam", "1.0")
