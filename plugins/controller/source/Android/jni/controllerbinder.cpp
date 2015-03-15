#include "controller.h"
#include "gideros.h"

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

static const char* RIGHT_JOYSTICK = "rightJoystick";
static const char* LEFT_JOYSTICK = "leftJoystick";
static const char* RIGHT_TRIGGER = "rightTrigger";
static const char* LEFT_TRIGGER = "leftTrigger";
static const char* KEY_DOWN = "keyDown";
static const char* KEY_UP = "keyUp";
static const char* CONNECTED = "connected";
static const char* DISCONNECTED = "disconnected";

static const int BUTTON_B = 97;
static const int DPAD_DOWN = 20;
static const int DPAD_LEFT = 21;
static const int DPAD_RIGHT = 22;
static const int DPAD_UP = 19;
static const int BUTTON_L1 = 102;
static const int BUTTON_L2 = 104;
static const int BUTTON_L3 = 106;
static const int BUTTON_MENU = 82;
static const int BUTTON_START = 108;
static const int BUTTON_BACK = 4;
static const int BUTTON_A = 96;
static const int BUTTON_R1 = 103;
static const int BUTTON_R2 = 105;
static const int BUTTON_R3 = 107;
static const int BUTTON_X = 99;
static const int BUTTON_Y = 100;

static char keyWeak = ' ';

class Controller : public GEventDispatcherProxy
{
public:
    Controller(lua_State *L) : L(L)
    {
        ghid_init();
		ghid_addCallback(callback_s, this);		
		initialized_ = false;
    }
    
    ~Controller()
    {
		ghid_removeCallback(callback_s, this);
		ghid_cleanup();
    }
	
	int isAnyAvailable()
	{
		return ghid_isAnyAvailable();
	}
	
	int getPlayerCount()
	{
		return ghid_getPlayerCount();
	}
	
	const char* getControllerName(int player)
	{
		return ghid_getControllerName(player);
	}
	
	void vibrate(int player, long ms)
	{
		ghid_vibrate(player, ms);
	}
	
	int* getPlayers(int* size)
	{
		return ghid_getPlayers(size);
	}
	
private:
	static void callback_s(int type, void *event, void *udata)
	{
		static_cast<Controller*>(udata)->callback(type, event);
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
			case GHID_KEY_DOWN_EVENT:
                lua_pushstring(L, KEY_DOWN);
                break;
			case GHID_KEY_UP_EVENT:
                lua_pushstring(L, KEY_UP);
                break;
			case GHID_RIGHT_JOYSTICK_EVENT:
                lua_pushstring(L, RIGHT_JOYSTICK);
                break;
			case GHID_LEFT_JOYSTICK_EVENT:
                lua_pushstring(L, LEFT_JOYSTICK);
                break;
			case GHID_RIGHT_TRIGGER_EVENT:
                lua_pushstring(L, RIGHT_TRIGGER);
                break;
			case GHID_LEFT_TRIGGER_EVENT:
                lua_pushstring(L, LEFT_TRIGGER);
                break;
			case GHID_CONNECTED_EVENT:
                lua_pushstring(L, CONNECTED);
                break;
			case GHID_DISCONNECTED_EVENT:
                lua_pushstring(L, DISCONNECTED);
                break;
        }

        lua_call(L, 1, 1);
		
		if (type == GHID_KEY_DOWN_EVENT || type == GHID_KEY_UP_EVENT)
        {
            ghid_KeyEvent *event2 = (ghid_KeyEvent*)event;
            
			lua_pushnumber(L, event2->keyCode);
			lua_setfield(L, -2, "keyCode");
			
			lua_pushnumber(L, event2->playerId);
			lua_setfield(L, -2, "playerId");
        }
		else if (type == GHID_RIGHT_JOYSTICK_EVENT || type == GHID_LEFT_JOYSTICK_EVENT)
        {
            ghid_JoystickEvent *event2 = (ghid_JoystickEvent*)event;
			
			lua_pushnumber(L, event2->x);
			lua_setfield(L, -2, "x");
			
			lua_pushnumber(L, event2->y);
			lua_setfield(L, -2, "y");
			
			lua_pushnumber(L, event2->angle);
			lua_setfield(L, -2, "angle");
			
			lua_pushnumber(L, event2->strength);
			lua_setfield(L, -2, "strength");
			
			lua_pushnumber(L, event2->playerId);
			lua_setfield(L, -2, "playerId");
        }
		else if (type == GHID_RIGHT_TRIGGER_EVENT || type == GHID_LEFT_TRIGGER_EVENT)
        {
            ghid_TriggerEvent *event2 = (ghid_TriggerEvent*)event;
			
			lua_pushnumber(L, event2->strength);
			lua_setfield(L, -2, "strength");
			
			lua_pushnumber(L, event2->playerId);
			lua_setfield(L, -2, "playerId");
        }
		else if (type == GHID_CONNECTED_EVENT || type == GHID_DISCONNECTED_EVENT)
        {
            ghid_DeviceEvent *event2 = (ghid_DeviceEvent*)event;
			
			lua_pushnumber(L, event2->playerId);
			lua_setfield(L, -2, "playerId");
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
	Controller *c = static_cast<Controller*>(object->proxy());
	
	c->unref();
	
	return 0;
}

static Controller *getInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "Controller", index));
	Controller *c = static_cast<Controller*>(object->proxy());
    
	return c;
}

static int isAnyAvailable(lua_State *L)
{
    Controller *c = getInstance(L, 1);
	lua_pushboolean(L, (bool)c->isAnyAvailable());
    return 1;
}

static int getPlayerCount(lua_State *L)
{
    Controller *c = getInstance(L, 1);
	lua_pushnumber(L, c->getPlayerCount());
    return 1;
}

static int getControllerName(lua_State *L)
{
    Controller *c = getInstance(L, 1);
	lua_pushstring(L, c->getControllerName(lua_tonumber(L, 2)));
    return 1;
}

static int vibrate(lua_State *L)
{
    Controller *c = getInstance(L, 1);
	c->vibrate(lua_tonumber(L, 2), lua_tonumber(L, 3));
    return 0;
}

static int getPlayers(lua_State *L)
{
    Controller *c = getInstance(L, 1);
	int size = 0;
	int *players = c->getPlayers(&size);
	lua_newtable(L);
	if (players != NULL)
	{
		for (int i = 0; i < size; ++i)
		{
			//set key
			lua_pushinteger(L, i+1);
			lua_pushnumber(L, players[i]);
			//back to table
			lua_settable(L, -3);
		}
	}
	lua_pushvalue(L, -1);
    return 1;
}

static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
		{"isAnyAvailable", isAnyAvailable},
		{"getPlayerCount", getPlayerCount},
		{"getControllerName", getControllerName},
		{"getPlayers", getPlayers},
		{"vibrate", vibrate},
		{NULL, NULL},
	};
    
    g_createClass(L, "Controller", "EventDispatcher", NULL, destruct, functionlist);
    
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    
	lua_getglobal(L, "Event");
	lua_pushstring(L, RIGHT_JOYSTICK);
	lua_setfield(L, -2, "RIGHT_JOYSTICK");
	lua_pushstring(L, LEFT_JOYSTICK);
	lua_setfield(L, -2, "LEFT_JOYSTICK");
	lua_pushstring(L, RIGHT_TRIGGER);
	lua_setfield(L, -2, "RIGHT_TRIGGER");
	lua_pushstring(L, LEFT_TRIGGER);
	lua_setfield(L, -2, "LEFT_TRIGGER");
	lua_pushstring(L, CONNECTED);
	lua_setfield(L, -2, "CONNECTED");
	lua_pushstring(L, DISCONNECTED);
	lua_setfield(L, -2, "DISCONNECTED");
	lua_pop(L, 1);
	
	lua_getglobal(L, "KeyCode");
	lua_pushnumber(L, DPAD_DOWN);
	lua_setfield(L, -2, "DPAD_DOWN");
	lua_pushnumber(L, DPAD_LEFT);
	lua_setfield(L, -2, "DPAD_LEFT");
	lua_pushnumber(L, DPAD_RIGHT);
	lua_setfield(L, -2, "DPAD_RIGHT");
	lua_pushnumber(L, DPAD_UP);
	lua_setfield(L, -2, "DPAD_UP");
	lua_pushnumber(L, BUTTON_L1);
	lua_setfield(L, -2, "BUTTON_L1");
	lua_pushnumber(L, BUTTON_L2);
	lua_setfield(L, -2, "BUTTON_L2");
	lua_pushnumber(L, BUTTON_L3);
	lua_setfield(L, -2, "BUTTON_L3");
	lua_pushnumber(L, BUTTON_R1);
	lua_setfield(L, -2, "BUTTON_R1");
	lua_pushnumber(L, BUTTON_R2);
	lua_setfield(L, -2, "BUTTON_R2");
	lua_pushnumber(L, BUTTON_R3);
	lua_setfield(L, -2, "BUTTON_R3");
	lua_pushnumber(L, BUTTON_MENU);
	lua_setfield(L, -2, "BUTTON_MENU");
	lua_pushnumber(L, BUTTON_BACK);
	lua_setfield(L, -2, "BUTTON_BACK");
	lua_pushnumber(L, BUTTON_A);
	lua_setfield(L, -2, "BUTTON_A");
	lua_pushnumber(L, BUTTON_X);
	lua_setfield(L, -2, "BUTTON_X");
	lua_pushnumber(L, BUTTON_Y);
	lua_setfield(L, -2, "BUTTON_Y");
	lua_pushnumber(L, BUTTON_B);
	lua_setfield(L, -2, "BUTTON_B");
	lua_pop(L, 1);

    Controller *c = new Controller(L);
	g_pushInstance(L, "Controller", c->object());
    
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, c);
	lua_pop(L, 1);
    
	lua_pushvalue(L, -1);
	lua_setglobal(L, "controller");
    
    return 1;
}
    
static void g_initializePlugin(lua_State *L)
{
    lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "controller");
	
	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
    
}

REGISTER_PLUGIN("Controller", "1.0")
