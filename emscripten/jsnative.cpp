#include "gideros.h"
#include "lua.hpp"
#include <lauxlib.h>
#include "emscripten.h"
#include "html5.h"

static int JSNative_eval(lua_State *L) {

	const char *str=luaL_checkstring(L,-1);

	char *ret=(char *) EM_ASM_INT({
	 return allocate(intArrayFromString(String(eval(Pointer_stringify($0)))), 'i8', ALLOC_STACK);
	},str);

	lua_pushstring(L,ret);

	return 1;
}

static void g_initializePlugin(lua_State *L) {
	luaL_Reg reg[] = { { "eval", JSNative_eval }, { NULL, NULL } };

	lua_newtable(L);
	//luaL_setfuncs(L, reg, 0);
	luaL_register(L,NULL,reg);
	lua_setglobal(L, "JS");
}

static void g_deinitializePlugin(lua_State *L) {
}

REGISTER_PLUGIN_NAMED("JSNative", "1.0", JSNative)
