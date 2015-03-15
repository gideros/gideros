#include "giftiz.h"
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

static const char* BUTTON_STATE_CHANGE = "buttonStateChange";

static const int INVISIBLE = 0;
static const int DEFAULT = 1;
static const int BADGE = 2;
static const int WARNING = 3;

static char keyWeak = ' ';

class Giftiz : public GEventDispatcherProxy
{
public:
    Giftiz(lua_State *L) : L(L)
    {
        giftiz_init();
		giftiz_addCallback(callback_s, this);		
    }
    
    ~Giftiz()
    {
		giftiz_removeCallback(callback_s, this);
		giftiz_cleanup();
    }
	
	bool isAvailable()
	{
		return giftiz_isAvailable();
	}
	
	void missionComplete()
	{
		giftiz_missionComplete();
	}

	void purchaseMade(float amount)
	{
		giftiz_purchaseMade(amount);
	}
	
	int getButtonState()
	{
		return giftiz_getButtonState();
	}
	
	void buttonClicked()
	{
		giftiz_buttonClicked();
	}

private:
	static void callback_s(int type, void *event, void *udata)
	{
		static_cast<Giftiz*>(udata)->callback(type, event);
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
			case GIFTIZ_BUTTON_STATE_CHANGE:
                lua_pushstring(L, BUTTON_STATE_CHANGE);
                break;
        }

        lua_call(L, 1, 1);
		
		if (type == GIFTIZ_BUTTON_STATE_CHANGE)
        {
            giftiz_Button *event2 = (giftiz_Button*)event;
            
			lua_pushnumber(L, event2->state);
			lua_setfield(L, -2, "buttonState");
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
	Giftiz *gfz = static_cast<Giftiz*>(object->proxy());
	
	gfz->unref();
	
	return 0;
}

static Giftiz *getInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "Giftiz", index));
	Giftiz *gfz = static_cast<Giftiz*>(object->proxy());
    
	return gfz;
}

static int isAvailable(lua_State *L)
{
	Giftiz *gfz = getInstance(L, 1);
	bool result = gfz->isAvailable();
	lua_pushboolean(L, result);
    return 1;
}

static int missionComplete(lua_State *L)
{
	Giftiz *gfz = getInstance(L, 1);
	gfz->missionComplete();
    return 0;
}

static int purchaseMade(lua_State *L)
{
	Giftiz *gfz = getInstance(L, 1);
	float amount = luaL_checknumber(L, 2);
	gfz->purchaseMade(amount);
    return 0;
}

static int getButtonState(lua_State *L)
{
	Giftiz *gfz = getInstance(L, 1);
	int state = gfz->getButtonState();
	lua_pushnumber(L, state);
    return 1;
}

static int buttonClicked(lua_State *L)
{
	Giftiz *gfz = getInstance(L, 1);
	gfz->buttonClicked();
    return 0;
}

static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
        {"isAvailable", isAvailable},
        {"missionComplete", missionComplete},
        {"purchaseMade", purchaseMade},
        {"getButtonState", getButtonState},
        {"buttonClicked", buttonClicked},
		{NULL, NULL},
	};
    
    g_createClass(L, "Giftiz", "EventDispatcher", NULL, destruct, functionlist);
    
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    
	lua_getglobal(L, "Event");
	lua_pushstring(L, BUTTON_STATE_CHANGE);
	lua_setfield(L, -2, "BUTTON_STATE_CHANGE");
	lua_pop(L, 1);
	
	lua_getglobal(L, "Giftiz");
	lua_pushnumber(L, INVISIBLE);
	lua_setfield(L, -2, "INVISIBLE");
	lua_pushnumber(L, DEFAULT);
	lua_setfield(L, -2, "DEFAULT");
	lua_pushnumber(L, BADGE);
	lua_setfield(L, -2, "BADGE");
	lua_pushnumber(L, WARNING);
	lua_setfield(L, -2, "WARNING");
	lua_pop(L, 1);

    Giftiz *gms = new Giftiz(L);
	g_pushInstance(L, "Giftiz", gms->object());
    
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, gms);
	lua_pop(L, 1);
    
	lua_pushvalue(L, -1);
	lua_setglobal(L, "giftiz");
    
    return 1;
}
    
static void g_initializePlugin(lua_State *L)
{
    lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "giftiz");
	
	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
    
}

REGISTER_PLUGIN("Giftiz", "1.0")
