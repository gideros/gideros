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
	const char *mime=luaL_checkstring(L,1);
	size_t datasize;
	const char *data=luaL_checklstring(L,2,&datasize);
    lua_pushboolean(L, gshare_Share(mime,data,datasize));
    
    return 1;
}

static int loader(lua_State* L)
{
    lua_newtable(L);
    lua_pushcfunction(L, share);
    lua_setfield(L, -2, "share");

    return 1;
}

static void g_initializePlugin(lua_State *L)
{
	gshare_Init();
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    
    lua_pushcfunction(L, loader);
    lua_setfield(L, -2, "Share");
    
    lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
	gshare_Cleanup();
}

REGISTER_PLUGIN("Share", "1.0")

