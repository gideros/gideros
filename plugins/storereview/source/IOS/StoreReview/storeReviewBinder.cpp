//
//  storeReviewBinder.cpp
//  OnlineWordHunt
//
//  Created by Mert Can KURUM on 13/02/17.
//  Copyright © 2017 Gideros Mobile. All rights reserved.
//

#include "gstoreReview.h"
#include "gideros.h"


static int requestReview(lua_State *L)
{
    auto isAvailable = (bool)gstorereview_requestReview();
    lua_pushboolean(L, isAvailable);
    
    return 1;
}

static int loader(lua_State* L)
{
    lua_newtable(L);
    
    lua_pushcfunction(L, requestReview);
    lua_setfield(L, -2, "requestReview");
    
    lua_pushvalue(L, -1);
    lua_setglobal(L, "storeReview");
    
    return 1;
}

static void g_initializePlugin(lua_State *L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    
    lua_pushcfunction(L, loader);
    lua_setfield(L, -2, "storeReview");
    
    lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
}

REGISTER_PLUGIN("StoreReview", "1.0")

