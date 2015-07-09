#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"

extern "C" {
LUALIB_API int luaopen_socket_core(lua_State *L);
LUALIB_API int luaopen_mime_core(lua_State *L);
}

static void g_initializePlugin(lua_State *L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, luaopen_socket_core);
    lua_setfield(L, -2, "socket.core");

    lua_pushcfunction(L, luaopen_mime_core);
    lua_setfield(L, -2, "mime.core");

	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
}
REGISTER_PLUGIN("LuaSocket", "1.1")
