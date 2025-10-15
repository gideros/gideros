#include "gideros.h"
#include <glog.h>
#include "agesignal.h"

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

static const char *AGE_SIGNALS = "ageSignalsReceived";

static char keyWeak = ' ';

class GAgeSignal : public GEventDispatcherProxy
{
public:
	GAgeSignal(lua_State *L)
    {
        gagesignal_init();
        gagesignal_addCallback(callback_s, this);
    }
    
    ~GAgeSignal()
    {
    	gagesignal_removeCallback(callback_s, this);
    	gagesignal_cleanup();
    }
	
	void checkAgeSignals()
	{
		gagesignal_checkAgeSignals();
	}
	
private:
	static void callback_s(int type, void *event, void *udata)
	{
		static_cast<GAgeSignal*>(udata)->callback(type, event);
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
            case AGESIGNAL_AGE_SIGNALS_ENV:
                lua_pushstring(L, AGE_SIGNALS);
                break;
        }

        lua_call(L, 1, 1);
		
        if (type == AGESIGNAL_AGE_SIGNALS_ENV)
        {
        	gagesignals_AgeSignalsEvent *event2 = (gagesignals_AgeSignalsEvent*)event;

        	lua_pushstring(L,event2->installId);
			lua_setfield(L, -2, "installId");
        	lua_pushstring(L,event2->status);
			lua_setfield(L, -2, "userStatus");
			lua_pushnumber(L, event2->ageLower);
			lua_setfield(L, -2, "ageLower");
			lua_pushnumber(L, event2->ageUpper);
			lua_setfield(L, -2, "ageUpper");
			lua_pushint64(L, event2->approvalDate);
			lua_setfield(L, -2, "mostRecentApprovalDate");
        }

		lua_call(L, 2, 0);
		
		lua_pop(L, 2);
	}

private:
    bool initialized_;
};

static int destruct(void *p)
{
	void *ptr = GIDEROS_DTOR_UDATA(p);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	GAgeSignal *googlelvl = static_cast<GAgeSignal*>(object->proxy());
	
	googlelvl->unref();
	
	return 0;
}

static GAgeSignal *getInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "GAgeSignal", index));
	GAgeSignal *googlelvl = static_cast<GAgeSignal*>(object->proxy());
    
	return googlelvl;
}

static int checkAgeSignals(lua_State *L)
{
	GAgeSignal *googlelvl = getInstance(L, 1);
    
    googlelvl->checkAgeSignals();
    
    return 0;
}

static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
        {"checkAgeSignals", checkAgeSignals},
		{NULL, NULL},
	};
    
    g_createClass(L, "GAgeSignal", "EventDispatcher", NULL, destruct, functionlist);
    
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    
	lua_getglobal(L, "Event");
	lua_pushstring(L, AGE_SIGNALS);
	lua_setfield(L, -2, "AGE_SIGNALS");
	lua_pop(L, 1);
	
    GAgeSignal *googlelvl = new GAgeSignal(L);
	g_pushInstance(L, "GAgeSignal", googlelvl->object());
    
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, googlelvl);
	lua_pop(L, 1);

    return 1;
}
    
static void g_initializePlugin(lua_State *L)
{
	::L = L;
    lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcnfunction(L, loader, "plugin_init_agesignal");
	lua_setfield(L, -2, "agesignals");
	
	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
    ::L = NULL;
}

REGISTER_PLUGIN("Age Signals plugin", "1.0")
