//
//  Copyright © 2017 Gideros Mobile. All rights reserved.
//

#include "gideros.h"
#include "megacoolbinder.h"

static g_id gid_;


static int share(lua_State *L)
{
	const char *fallback=luaL_optstring(L,1,NULL);
    lua_pushboolean(L, gmegacool_Share(fallback));
    return 1;
}

static int startRecording(lua_State *L)
{
    lua_pushboolean(L, gmegacool_StartRecording());
    return 1;
}

static int stopRecording(lua_State *L)
{
    gmegacool_StopRecording();
    return 0;
}

static int setSharingText(lua_State *L)
{
	const char *sharingText=luaL_optstring(L,1,NULL);
    lua_pushboolean(L, gmegacool_SetSharingText(sharingText));
    return 1;
}

static int cbRef=LUA_NOREF;
static int setCallback(lua_State *L)
{
	if (cbRef!=LUA_NOREF)
		lua_unref(L,cbRef);
	lua_pushvalue(L,1);
	cbRef=lua_ref(L,1);
	lua_pop(L,1);
    return 0;
}

static lua_State *L;
static int loader(lua_State* L)
{
    lua_newtable(L);
    lua_pushcnfunction(L, startRecording, "startRecording");
    lua_setfield(L, -2, "startRecording");
    lua_pushcnfunction(L, stopRecording, "stopRecording");
    lua_setfield(L, -2, "stopRecording");
    lua_pushcnfunction(L, share, "share");
    lua_setfield(L, -2, "share");
    lua_pushcnfunction(L, setSharingText, "setSharingText");
    lua_setfield(L, -2, "setSharingText");
    lua_pushcnfunction(L, setCallback, "setCallback");
    lua_setfield(L, -2, "setCallback");

    return 1;
}

static void g_initializePlugin(lua_State *L)
{
    gmegacool_Init();
    gid_ = g_NextId();

	::L=L;
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    
	lua_pushcnfunction(L, loader, "plugin_init_megacool");
    lua_setfield(L, -2, "Megacool");
    
    lua_pop(L, 2);

}

static void g_deinitializePlugin(lua_State *L)
{
	if (cbRef!=LUA_NOREF)
		lua_unref(L,cbRef);
	cbRef=LUA_NOREF;
    gmegacool_Destroy();
	gevent_RemoveEventsWithGid(gid_);
}


static void callback_s(int type, void *event, void *udata)
{
	if (cbRef!=LUA_NOREF) {
		lua_getref(L,cbRef);
		lua_pushinteger(L,type);
		lua_call(L,1,0);
	}
}

void gmegacool_Event(int type)
{
	gevent_EnqueueEvent(gid_, callback_s, type, NULL, 0, NULL);
}



REGISTER_PLUGIN("Megacool", "1.1")

