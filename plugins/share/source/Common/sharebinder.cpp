//
//  storeReviewBinder.cpp
//  OnlineWordHunt
//
//  Created by Mert Can KURUM on 13/02/17.
//  Copyright © 2017 Gideros Mobile. All rights reserved.
//

#include "gideros.h"
#include "sharebinder.h"


static int share(lua_State *L)
{
	std::map<std::string,std::string> values;
	size_t datasize;
	if (lua_type(L,1)==LUA_TTABLE) {
	     lua_pushnil(L);  /* first key */
	     while (lua_next(L, 1) != 0) {
			const char *mime=luaL_checkstring(L,-2);
			const char *data=luaL_checklstring(L,-1,&datasize);
			values[std::string(mime)]=std::string(data,datasize);
	       /* removes 'value'; keeps 'key' for next iteration */
	       lua_pop(L, 1);
	     }

	}
	else {
		const char *mime=luaL_checkstring(L,1);
		const char *data=luaL_checklstring(L,2,&datasize);
		values[std::string(mime)]=std::string(data,datasize);
	}
    lua_pushboolean(L, gshare_Share(values));
    
    return 1;
}

static int loader(lua_State* L)
{
    lua_newtable(L);
    lua_pushcnfunction(L, share, "share");
    lua_setfield(L, -2, "share");

    return 1;
}

static void g_initializePlugin(lua_State *L)
{
	gshare_Init();
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    
	lua_pushcnfunction(L, loader, "plugin_init_share");
    lua_setfield(L, -2, "Share");
    
    lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
	gshare_Cleanup();
}

REGISTER_PLUGIN("Share", "1.0")

