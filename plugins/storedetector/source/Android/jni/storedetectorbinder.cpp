#include "storedetector.h"
#include "gideros.h"
#include <glog.h>

// some Lua helper functions
#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif

static lua_State *L = NULL;

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

static char keyWeak = ' ';

class StoreDetector : public GProxy
{
public:
    StoreDetector(lua_State *L)
    {
        sd_init();
    }
    
    ~StoreDetector()
    {
		sd_cleanup();
    }
	
    std::string getStore()
	{
		return sd_getStore();
	}
	
	int isConsole()
	{
		return sd_isConsole();
	}
};

static int destruct(lua_State* L)
{
	void *ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	StoreDetector *sd = static_cast<StoreDetector*>(object->proxy());
	
	sd->unref();
	
	return 0;
}

static StoreDetector *getInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "StoreDetector", index));
	StoreDetector *sd = static_cast<StoreDetector*>(object->proxy());
    
	return sd;
}


static int isConsole(lua_State *L)
{
    StoreDetector *sd = getInstance(L, 1);
	lua_pushboolean(L, sd->isConsole());
    return 1;
}

static int getStore(lua_State *L)
{
	StoreDetector *sd = getInstance(L, 1);
	lua_pushstring(L, sd->getStore().c_str());
    return 1;
}

static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
        {"isConsole", isConsole},
        {"getStore", getStore},
		{NULL, NULL},
	};
    
    g_createClass(L, "StoreDetector", NULL, NULL, destruct, functionlist);
    
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	
	StoreDetector *sd = new StoreDetector(L);
	g_pushInstance(L, "StoreDetector", sd->object());
    
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, sd);
	lua_pop(L, 1);
    
	lua_pushvalue(L, -1);
	lua_setglobal(L, "storedetector");
    
    return 1;
}

static void g_initializePlugin(lua_State *L)
{
	::L = L;
    lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "storedetector");
	
	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
	::L = NULL;
}

REGISTER_PLUGIN("StoreDetector", "1.0")
