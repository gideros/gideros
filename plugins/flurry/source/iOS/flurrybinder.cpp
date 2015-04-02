#include "gflurry.h"
#include "gideros.h"
#include <vector>
#include <string>

#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif

static int isSessionStarted = 0;

static int isAvailable(lua_State *L)
{
	lua_pushboolean(L, 1);
	return 1;
}

static int startSession(lua_State *L)
{
	if (isSessionStarted)
		return 0;
    
    gflurry_StartSession(luaL_checkstring(L, 1));

	isSessionStarted = 1;
    
	return 0;
}

static char **copyParameters(lua_State *L, int index)
{
	luaL_checktype(L, index, LUA_TTABLE);

	int t = abs_index(L, index);
    
    std::vector<std::string> parameters;
	lua_pushnil(L);
	while (lua_next(L, t) != 0)
	{
        parameters.push_back(luaL_checkstring(L, -2));
        parameters.push_back(luaL_checkstring(L, -1));
		lua_pop(L, 1);
    }
    
    char **parameters2 = (char**)malloc((parameters.size() + 1) * sizeof(char*));
    for (std::size_t i = 0; i < parameters.size(); ++i)
        parameters2[i] = strdup(parameters[i].c_str());
    parameters2[parameters.size()] = NULL;

    return parameters2;
}

static void freeParameters(char **parameters)
{
    if (parameters == NULL)
        return;
    
    char **parameters2 = parameters;
    
    while (*parameters2)
        free(*parameters2++);

    free(parameters);
}

static int logEvent(lua_State *L)
{
    const char *eventName = luaL_checkstring(L, 1);
    
    char **parameters = NULL;
    if (!lua_isnoneornil(L, 2))
        parameters = copyParameters(L, 2);
    
    int timed = lua_toboolean(L, 3);
    
    gflurry_LogEvent(eventName, (const char **)parameters, timed);
    
    freeParameters(parameters);
    
    return 0;
}

static int endTimedEvent(lua_State *L)
{
    const char *eventName = luaL_checkstring(L, 1);
    
    char **parameters = NULL;
    if (!lua_isnoneornil(L, 2))
        parameters = copyParameters(L, 2);

    gflurry_EndTimedEvent(eventName, (const char **)parameters);
    
    freeParameters(parameters);
    
    return 0;
}

static int loader(lua_State* L)
{
	lua_newtable(L);
    
	lua_pushcfunction(L, isAvailable);
	lua_setfield(L, -2, "isAvailable");
	lua_pushcfunction(L, startSession);
	lua_setfield(L, -2, "startSession");
	lua_pushcfunction(L, logEvent);
	lua_setfield(L, -2, "logEvent");
	lua_pushcfunction(L, endTimedEvent);
	lua_setfield(L, -2, "endTimedEvent");
    
	lua_pushvalue(L, -1);
	lua_setglobal(L, "flurry");
    
	return 1;
}

static void g_initializePlugin(lua_State *L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
    
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "flurry");
    
	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
}

REGISTER_PLUGIN("Flurry", "1.0")

