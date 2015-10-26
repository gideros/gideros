#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"

extern "C" {
LUALIB_API int luaopen_cjson(lua_State *L);
LUALIB_API int luaopen_cjson_safe(lua_State *L);
}

static void g_initializePlugin(lua_State *L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, luaopen_cjson);
	lua_setfield(L, -2, "json");
	
	lua_pushcfunction(L, luaopen_cjson_safe);
	lua_setfield(L, -2, "json.safe");
	
	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
}
REGISTER_PLUGIN_NAMED("Json", "1.0",json)