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

	TextureBase* textureBase = static_cast<TextureBase*>(g_getInstance(L,"TextureBase",1));
	if (cameraplugin::cameraTexture)
		cameraplugin::cameraTexture->unref();
	textureBase->ref();
	cameraplugin::cameraTexture=textureBase;


	Orientation orientation=eFixed;
	lua_getglobal(L,"application");
#ifndef FIXED_ORIENTATION
	LuaApplication* application=static_cast<LuaApplication *>(g_getInstance(L,"Application", -1));
	lua_pop(L,1);
	orientation = application->orientation();
#else
	lua_getfield(L,-1,"getOrientation");
	lua_getglobal(L,"application");
	lua_call(L,1,1);
	const char *ors=lua_tostring(L,-1);
	lua_pop(L,1);
	orientation=ePortrait;
	if (!strcmp(ors,"landscapeLeft"))
		orientation=eLandscapeLeft;
	else if (!strcmp(ors,"landscapeRight"))
		orientation=eLandscapeRight;
	else if (!strcmp(ors,"portraitUpsideDown"))
		orientation=ePortraitUpsideDown;
#endif

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
