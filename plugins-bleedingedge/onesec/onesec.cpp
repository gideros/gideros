#define HAVE_ENTER_FRAME

#include <gideros.h>
#include <set>


static const char KEY_OBJECTS = ' ';
static const char KEY_EVENT = ' ';

class OneSec : public GEventDispatcherProxy
{
public:
	OneSec()
	{
		id = gid++;
	}

	int id;
	static int gid;
};

int OneSec::gid = 0;

static int create(lua_State* L)
{
	OneSec* proxy = new OneSec;

	g_pushInstance(L, "OneSec", proxy->object());

	lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushinteger(L, proxy->id);
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pop(L, 1);

	return 1;
}

static int destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	Referenced* object = static_cast<Referenced*>(ptr);
	OneSec* proxy = static_cast<OneSec*>(object->proxy());

	proxy->unref();

	return 0;
}

static int loader(lua_State* L)
{
	const luaL_Reg functionlist[] = {
		{NULL, NULL},
	};

	g_createClass(L, "OneSec", "EventDispatcher", create, destruct, functionlist);

	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of KEY
	lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
	lua_newtable(L);                  // create a table
	lua_pushliteral(L, "v");
	lua_setfield(L, -2, "__mode");    // set as weak-value table
	lua_pushvalue(L, -1);             // duplicate table
	lua_setmetatable(L, -2);          // set itself as metatable
	lua_rawset(L, LUA_REGISTRYINDEX);

	// create an Event object to be used again and again
	lua_pushlightuserdata(L, (void *)&KEY_EVENT);
	lua_getglobal(L, "Event");
	lua_getfield(L, -1, "new");
	lua_remove(L, -2);
	lua_pushliteral(L, "onesec");
	lua_call(L, 1, 1);
	lua_rawset(L, LUA_REGISTRYINDEX);

	lua_getglobal(L, "OneSec");

	return 1;
}


static void g_initializePlugin(lua_State *L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "onesec");

	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{


}


static void g_enterFrame(lua_State* L)
{
	static int frame = 0;
	frame++;
	if (frame != 60)
		return;
	frame = 0;

	std::vector<int> keys;

	lua_pushlightuserdata(L, (void *)&KEY_OBJECTS);
	lua_rawget(L, LUA_REGISTRYINDEX);

	int t = lua_gettop(L);
	lua_pushnil(L);
	while (lua_next(L, t) != 0)
	{
		keys.push_back(lua_tointeger(L, -2));
		lua_pop(L, 1);
	}

	for (size_t i = 0; i < keys.size(); ++i)
	{
		lua_rawgeti(L, -1, keys[i]);
		if (!lua_isnil(L, -1))
		{
			lua_getfield(L, -1, "dispatchEvent");
			lua_pushvalue(L, -2);
			lua_pushlightuserdata(L, (void *)&KEY_EVENT);
			lua_rawget(L, LUA_REGISTRYINDEX);
			lua_call(L, 2, 0);
		}
		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

REGISTER_PLUGIN("onesec", "1.0")
