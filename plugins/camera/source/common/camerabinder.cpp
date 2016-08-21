#include "gideros.h"
#include "lua.h"
#include "luautil.h"
#include "lauxlib.h"
#include "camerabinder.h"
#include "binder.h"
#include "luaapplication.h"

TextureBase *cameraplugin::cameraTexture=NULL;

static int start(lua_State* L)
{
	Binder binder(L);

	TextureBase* textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase"));
	if (cameraplugin::cameraTexture)
		cameraplugin::cameraTexture->unref();
	textureBase->ref();
	cameraplugin::cameraTexture=textureBase;

	lua_getglobal(L,"application");
	(void)binder.getInstance("Application", -1);
	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	lua_pop(L,1);

	Orientation orientation = application->orientation();

	int camwidth,camheight;
	cameraplugin::start(orientation,&camwidth,&camheight);
	lua_pushnumber(L,camwidth);
	lua_pushnumber(L,camheight);

	return 2;
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


static int loader(lua_State* L)
{
	const luaL_Reg functionlist[] = {
		{"start", start},
		{"stop", stop},
		{NULL, NULL},
	};

	cameraplugin::cameraTexture=NULL;
	cameraplugin::init();
	g_createClass(L, "Camera", NULL, NULL, NULL, functionlist);

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
	if (cameraplugin::cameraTexture)
	{
		cameraplugin::cameraTexture->unref();
		cameraplugin::cameraTexture=NULL;
	}
	cameraplugin::deinit();
}


#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
REGISTER_PLUGIN_STATICNAMED_CPP("Camera", "1.0",Camera)
#else
REGISTER_PLUGIN("Camera", "1.0")
#endif