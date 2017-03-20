/*

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2012 Gideros Mobile 

*/

#include "gideros.h"

extern "C"
{
LUALIB_API int luaopen_lsqlite3(lua_State *L);
}

static void g_initializePlugin(lua_State* L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, luaopen_lsqlite3);
	lua_setfield(L, -2, "lsqlite3");
	
	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State* L)
{

}

REGISTER_PLUGIN_NAMED("LuaSQLite3", "1.0", lsqlite3)
