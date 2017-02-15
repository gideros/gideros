#include "binder.h"
#include "lua.h"
#include "luautil.h"
#include "lauxlib.h"

#include "examplepluginbinder.h"

static lua_State* luaState = NULL;
static const char *EXAMPLEPLUGIN_STATE = "exampleplugin_state";
static const char *EXAMPLEPLUGIN_WIFI = "exampleplugin_wifi";

class Exampleplugin : public GEventDispatcherProxy
{
public:
	Exampleplugin()
    {
    }
    ~Exampleplugin()
    {
    }
};
static int start(lua_State* L)
{
	exampleplugin::start();
	return 0;
}
static int stop(lua_State* L)
{
	exampleplugin::stop();
	return 0;
}
static int test(lua_State* L)
{
	bool result = exampleplugin::test_binder();
	lua_pushboolean(L,result);
	return 1;/*nb value return*/
}
void gexampleplugin_dispatch(int type, void *event) {
	lua_State* L = luaState;
	if (L == NULL)
		return;
	lua_getglobal(L, "exampleplugin");// instance
	lua_getfield(L, -1, "dispatchEvent");
	lua_pushvalue(L, -2);

	lua_getglobal(L, "Event");
	lua_getfield(L, -1, "new");
	lua_remove(L, -2);

	 switch (type)
	{
		case GEXAMPLEPLUGIN_EVENT_STATE:
			lua_pushstring(L, EXAMPLEPLUGIN_STATE);
			break;
		case GEXAMPLEPLUGIN_EVENT_WIFI:
			lua_pushstring(L, EXAMPLEPLUGIN_WIFI);
			break;
	}
	lua_call(L, 1, 1);
	if (type == GEXAMPLEPLUGIN_EVENT_STATE)
    {
    	gexampleplugin_state *e = (gexampleplugin_state*)event;
    	/*push in lua stack from the event, then, set refs which can be called in lua */
    	//from int
    	lua_pushnumber(L, e->state);
    	lua_setfield(L, -2, "state_lua");
    	//from char
    	lua_pushstring(L, e->description);
		lua_setfield(L, -2, "description_lua");
    }
	else if (type == GEXAMPLEPLUGIN_EVENT_WIFI)
	{
		gexampleplugin_wifi *e = (gexampleplugin_wifi*)event;

		/*push in lua stack from the event, then, set refs which can be called in lua */

		//from char
		lua_pushstring(L, e->action);
		lua_setfield(L, -2, "action_lua");
		//from int
		lua_pushnumber(L, e->count);
		lua_setfield(L, -2, "count_lua");
		//from boolean
		lua_pushboolean(L, e->granted);
		lua_setfield(L, -2, "granted_lua");

		//from a fixed size struct to a table
		if (e->permissions_all) {
			lua_newtable(L);//create a table in stack
			lua_pushstring(L, e->permissions_all->change);//set the string value
			lua_rawseti(L,-2,1);//at index 1 // start at 1 (=lua)
			lua_pushstring(L, e->permissions_all->coarse);//set the string value
			lua_rawseti(L,-2,2);//at index 2
			lua_pushstring(L, e->permissions_all->fine);//set the string value
			lua_rawseti(L,-2,3);//at index 3
			lua_setfield(L, -2,"permissions_all_lua");//key of table
		}
		//from a variable size struct to a table of tables
		if (e->permissions_checked) {
			int size = (int)e->checked;//get len of list (only get, not pushed)
			lua_newtable(L);//create a table in stack
			for (int i = 0; i < size; ++i)
			{
				lua_newtable(L);//create a sub table in stack
				lua_pushstring(L, e->permissions_checked[i].permission);
				lua_setfield(L, -2, "permission_lua");
				lua_rawseti(L, -2, i + 1);//at index i+ 1 // start at 1 (=lua)
			}
			lua_setfield(L, -2,"permissions_checked_lua");//key of table
		}
	}
	lua_call(L, 2, 0);
	lua_pop(L, 2);
}
static int loader(lua_State* L)
{
	const luaL_Reg functionlist[] = {
		/*functions defined above (refs can be called by lua)*/
		{"start_lua", start},
		{"stop_lua", stop},
		{"test_lua", test},
		{NULL, NULL},
	};
	exampleplugin::init();
	/*L, class name to call in lua, dispatcher, constructor, destructor, functions*/
	g_createClass(L, "Exampleplugin", "EventDispatcher", NULL, NULL, functionlist);

	Exampleplugin *p = new Exampleplugin();
    g_pushInstance(L, "Exampleplugin", p->object());
	lua_setglobal(L,"exampleplugin");

    lua_getglobal(L, "Event");
    lua_pushstring(L, EXAMPLEPLUGIN_STATE);
    lua_setfield(L, -2, "EXAMPLEPLUGIN_STATE");
    lua_pushstring(L, EXAMPLEPLUGIN_WIFI);
    lua_setfield(L, -2, "EXAMPLEPLUGIN_WIFI");
    lua_pop(L, 1);

	return 0;
}
static void g_initializePlugin(lua_State* L)
{
	luaState = L;
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "exampleplugin");

	lua_pop(L, 2);
}
static void g_deinitializePlugin(lua_State *L)
{
	exampleplugin::deinit();
}

/*register plugin = class name in .gplugin*/
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
REGISTER_PLUGIN_STATICNAMED_CPP("Exampleplugin", "1.0",Exampleplugin)
#else
REGISTER_PLUGIN("Exampleplugin", "1.0")
#endif
