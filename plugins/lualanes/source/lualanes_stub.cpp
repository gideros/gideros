#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"

extern "C" {
LUALIB_API int luaopen_lanes_core(lua_State *L);
//LUALIB_API int luaopen_lanes_embedded(lua_State *L);
} // extern C

static void g_initializePlugin(lua_State *L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
    
    lua_pushcfunction(L, luaopen_lanes_core);
    lua_setfield(L, -2, "lanes.core");

	lua_pop(L, 1);
}

static void g_deinitializePlugin(lua_State *L)
{
}
REGISTER_PLUGIN_NAMED("LuaLanes", "3.12", lualanes)
