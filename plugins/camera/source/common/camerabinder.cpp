#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"
#include "camerabinder.h"
#include "binder.h"

TextureBase *cameraplugin::cameraTexture=NULL;

static int start(lua_State* L)
{
	Binder binder(L);

	TextureBase* textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase"));
	if (cameraplugin::cameraTexture)
		cameraplugin::cameraTexture->unref();
	textureBase->ref();
	cameraplugin::cameraTexture=textureBase;

	cameraplugin::start();

	return 0;
}

static int stop(lua_State* L)
{
	cameraplugin::stop();

	if (cameraplugin::cameraTexture)
	{
		cameraplugin::cameraTexture->unref();
		cameraplugin::cameraTexture=NULL;
	}
	return 0;
}

static int destruct(lua_State* L)
{
	stop(L);
}

static int loader(lua_State* L)
{
	const luaL_Reg functionlist[] = {
		{"start", start},
		{"stop", stop},
		{NULL, NULL},
	};

	cameraplugin::cameraTexture=NULL;
	g_createClass(L, "Camera", NULL, NULL, destruct, functionlist);

	return 0;
}

static void g_initializePlugin(lua_State* L)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "camera");

	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
}

REGISTER_PLUGIN("Camera", "1.0")
