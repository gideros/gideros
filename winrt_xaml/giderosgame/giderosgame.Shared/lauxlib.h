#ifndef LUA_AUXLIB_COMPART_H_
#define LUA_AUXLIB_COMPART_H_
#include "lualib.h"
typedef luaL_Reg luaL_reg;
#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))
#endif
