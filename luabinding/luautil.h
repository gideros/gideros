#ifndef LUAUTILH
#define LUAUTIL_H

//extern "C"
//{
//#include "lua.h"
//#include "lauxlib.h"
//#include "lualib.h"
//}
#include "lua.hpp"

void luaL_newweaktable(lua_State* L);
void luaL_nullifytable(lua_State*L, int index);
int luaC_traceback(lua_State* L);
int lua_pcall_traceback(lua_State* L, int nargs, int nresults, int unused);
void luaL_rawgetptr(lua_State *L, int idx, void* ptr);
void luaL_rawsetptr(lua_State *L, int idx, void* ptr);
int lua_toboolean2(lua_State *L, int idx);


#endif

