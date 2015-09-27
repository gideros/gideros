#ifndef LUAUTIL_H
#define LUAUTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "gexport.h"

GIDEROS_API void luaL_newweaktable(lua_State* L);
GIDEROS_API void luaL_nullifytable(lua_State*L, int index);
GIDEROS_API int luaC_traceback(lua_State* L);
GIDEROS_API int lua_pcall_traceback(lua_State* L, int nargs, int nresults, int unused);
GIDEROS_API void luaL_rawgetptr(lua_State *L, int idx, void* ptr);
GIDEROS_API void luaL_rawsetptr(lua_State *L, int idx, void* ptr);
GIDEROS_API int lua_toboolean2(lua_State *L, int idx);
GIDEROS_API void luaL_setdata(lua_State *L, void *data);
GIDEROS_API void *luaL_getdata(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif
