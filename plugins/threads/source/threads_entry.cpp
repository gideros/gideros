/*

    threads_entry.cpp:

        Defines the plugin functions called by Gideros for the Threads plugin

*/

#include "macros.h"
#include "gideros.h"
#include "luathread.h"
#include "lua.hpp"
#include "binder.h"


static int loader(lua_State *L) {
    Binder binder(L);

    binder.createClass(
              LuaThread::class_name,      "",
              LuaThread::lua_create,      LuaThread::lua_destroy,
        {
            { "getNumLogicalCores",       LuaThread::lua_getHardwareConcurrency },
            { "setFunction",              LuaThread::lua_setFunction },
            { "execute",                  LuaThread::lua_execute },
            { "resume",                   LuaThread::lua_resume },
            { "status",                   LuaThread::lua_status },
            { "requestTermination",       LuaThread::lua_requestTermination },
            { "getResult",                LuaThread::lua_getResult },
            { "fetchData",                LuaThread::lua_fetchData },
            { nullptr,                    nullptr }
        }
    );

    return 0;
}

static void g_initializePlugin(lua_State *L) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, loader);
    lua_setfield(L, -2, "Threads");

    lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L) {
    UNUSED(L);
}

// "threads" is a symbol needed for Emscripten if exporting to HTML
REGISTER_PLUGIN_NAMED("Threads", "0.1.0", threads);
