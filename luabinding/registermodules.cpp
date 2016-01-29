//extern "C"
//{
//#include "lua.h"
//#include "lauxlib.h"
//#include "lualib.h"
//}

#include "lua.hpp"

void register_finishedGroups(lua_State* L);
void register_zlib(lua_State *L);
void register_crypto(lua_State *L);

void registerModules(lua_State* L)
{
//	register_finishedGroups(L);
	register_zlib(L);
	register_crypto(L);
}
