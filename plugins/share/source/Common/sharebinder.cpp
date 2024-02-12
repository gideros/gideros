//
//  storeReviewBinder.cpp
//  OnlineWordHunt
//
//  Created by Mert Can KURUM on 13/02/17.
//  Copyright © 2017 Gideros Mobile. All rights reserved.
//

#include "gideros.h"
#include "sharebinder.h"
#include "lua.h"
#include "lauxlib.h"

namespace {

static const char* IMPORT_RESULT = "fileshareImportResult";
static const char* EXPORT_RESULT = "fileshareExportResult";

// some Lua helper functions
#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif

static lua_State *mainL=NULL;

static void luaL_newweaktable(lua_State *L, const char *mode)
{
    lua_newtable(L);			// create table for instance list
    lua_pushstring(L, mode);
    lua_setfield(L, -2, "__mode");	  // set as weak-value table
    lua_pushvalue(L, -1);             // duplicate table
    lua_setmetatable(L, -2);          // set itself as metatable
}

static void luaL_rawgetptr(lua_State *L, int idx, void *ptr)
{
    idx = abs_index(L, idx);
    lua_pushlightuserdata(L, ptr);
    lua_rawget(L, idx);
}

static void luaL_rawsetptr(lua_State *L, int idx, void *ptr)
{
    idx = abs_index(L, idx);
    lua_pushlightuserdata(L, ptr);
    lua_insert(L, -2);
    lua_rawset(L, idx);
}

static char keyStrong = ' ';
static char keyWeak = ' ';

class GFileShare : public GEventDispatcherProxy
{
public:
	GFileShare(lua_State *L) : L(L)
    {
        gshare_AddCallback(callback_s, this);
    }

    ~GFileShare()
    {
        gshare_RemoveCallback(callback_s, this);
    }

private:
    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<GFileShare*>(udata)->callback(type, event);
    }

    void callback(int type, void *event)
    {

        luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
        luaL_rawgetptr(L, -1, this);

        if (lua_isnil(L, -1))
        {
            lua_pop(L, 2);
            return;
        }
        lua_getfield(L, -1, "dispatchEvent");
        lua_pushvalue(L, -2); //KW,self,dispatchEvent,self

        if (type == GFILESHARE_IMPORT_RESULT_EVENT)
        {
            gfileshare_ResultEvent* event2 = (gfileshare_ResultEvent*)event;

            lua_getfield(L, -1, "__importResultEvent");
            if (event2->name)
				lua_pushstring(L, event2->name);
            else
                lua_pushnil(L);
			lua_setfield(L, -2, "name");
            if (event2->mime)
				lua_pushstring(L, event2->mime);
            else
                lua_pushnil(L);
			lua_setfield(L, -2, "mime");

			if (event2->dataSize)
				lua_pushlstring(L,(char *)(event2+1),event2->dataSize);
			else
				lua_pushnil(L);
            lua_setfield(L, -2, "data");

            lua_pushinteger(L,event2->status);
            lua_setfield(L, -2, "status");

            lua_call(L, 2, 0);
        }
        else if (type == GFILESHARE_EXPORT_RESULT_EVENT)
        {
            gfileshare_ResultEvent* event2 = (gfileshare_ResultEvent*)event;

            lua_getfield(L, -1, "__exportResultEvent");
            lua_pushinteger(L,event2->status);
            lua_setfield(L, -2, "status");

            lua_call(L, 2, 0);
        }
        else
            lua_pop(L,2);
        lua_pop(L, 2);
    }

private:
    lua_State *L;
};

}

static int create(lua_State *L)
{
    GFileShare *share = new GFileShare(mainL);

    g_pushInstance(L, "Share", share->object());


    lua_getglobal(L, "Event");
    lua_getfield(L, -1, "new");
    lua_remove(L, -2);

    lua_pushvalue(L,-1);
    lua_pushstring(L, IMPORT_RESULT);
    lua_call(L, 1, 1);
    lua_setfield(L, -3, "__importResultEvent");

    lua_pushstring(L, EXPORT_RESULT);
    lua_call(L, 1, 1);
    lua_setfield(L, -2, "__exportResultEvent");

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, share);
    lua_pop(L, 1);

    return 1;
}

static int destruct(void *p)
{
	void *ptr = GIDEROS_DTOR_UDATA(p);
    GReferenced* object = static_cast<GReferenced*>(ptr);
    GReferenced* proxy = object->proxy();
    proxy->unref();

    return 0;
}

static GFileShare *getInstance(lua_State *L, int index)
{
    GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "Share", index));
    GReferenced *proxy = object->proxy();

    return static_cast<GFileShare*>(proxy);
}

static int importFile(lua_State *L)
{
	//GFileShare * share = getInstance(L, 1);

    lua_pushboolean(L,gshare_Import(luaL_optstring(L,2,NULL),luaL_optstring(L,3,NULL)));

    return 1;
}

static int exportFile(lua_State *L)
{
	//GFileShare * share = getInstance(L, 1);
	size_t dsz;
	const char *data=luaL_checklstring(L,2,&dsz);

    lua_pushboolean(L,gshare_Export(data,dsz,luaL_checkstring(L,3),luaL_checkstring(L,4)));

    return 1;
}


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

static int capabilities(lua_State *L)
{
    lua_pushinteger(L,gshare_Capabilities());

    return 1;
}


static int loader(lua_State* L)
{

   const luaL_Reg functionlist[] = {
		{"import", importFile},
		{"export", exportFile},
		{"share", share},
		{"getCapabilities", capabilities},
		{NULL, NULL},
	};

	g_createClass(L, "Share", "EventDispatcher", create, destruct, functionlist);

	lua_getglobal(L, "Event");
	lua_pushstring(L, IMPORT_RESULT);
	lua_rawsetfield(L, -2, "SHARE_IMPORT_RESULT");
	lua_pushstring(L, EXPORT_RESULT);
	lua_rawsetfield(L, -2, "SHARE_EXPORT_RESULT");
	lua_pop(L, 1);

	lua_newtable(L);
	luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyStrong);

	luaL_newweaktable(L, "v");
	luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);

    lua_getglobal(L, "Share");

    return 1;
}

static void g_initializePlugin(lua_State *L)
{
	gshare_Init();
	mainL=L;
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

