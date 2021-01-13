#include "gideros.h"
#include "lua.h"
#include "luautil.h"
#include "lauxlib.h"
#include "camerabinder.h"
#include "binder.h"
#include "luaapplication.h"

GRenderTarget *cameraplugin::cameraTexture=NULL;
LuaApplication *cameraplugin::application=NULL;

static int availableDevices(lua_State* L)
{
 std::vector<cameraplugin::CameraDesc> cams=cameraplugin::availableDevices();
 lua_newtable(L);
 for (size_t k=0;k<cams.size();k++)
 {
	 cameraplugin::CameraDesc cam=cams[k];
	 lua_newtable(L);
	 lua_pushstring(L,cam.name.c_str());
	 lua_setfield(L,-2,"name");
	 lua_pushstring(L,cam.description.c_str());
	 lua_setfield(L,-2,"description");
	 if (cam.pos==cameraplugin::CameraDesc::POS_FRONTFACING)
		 lua_pushstring(L,"front");
	 else if (cam.pos==cameraplugin::CameraDesc::POS_BACKFACING)
		 lua_pushstring(L,"back");
	 else
		 lua_pushstring(L,"unknown");
	 lua_setfield(L,-2,"position");
	 lua_rawseti(L,-2,k+1);
 }
 return 1;
}

static int start(lua_State* L)
{
	GRenderTarget* textureBase = static_cast<GRenderTarget*>(g_getInstance(L,"RenderTarget",1));
	const char *name=luaL_optstring(L,2,NULL);
	if (cameraplugin::cameraTexture)
		cameraplugin::cameraTexture->unref();
	textureBase->ref();
	cameraplugin::cameraTexture=textureBase;


	Orientation orientation=eFixed;
	lua_getglobal(L,"application");
	LuaApplication* application=static_cast<LuaApplication *>(luaL_getdata(L));
	cameraplugin::application=application;
#ifndef FIXED_ORIENTATION
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
	cameraplugin::start(orientation,&camwidth,&camheight,name);
	lua_pushnumber(L,camwidth);
	lua_pushnumber(L,camheight);

	return 2;
}

static int stop(lua_State* L)
{
    G_UNUSED(L);
    cameraplugin::stop();
    
    if (cameraplugin::cameraTexture)
    {
        cameraplugin::cameraTexture->unref();
        cameraplugin::cameraTexture=NULL;
    }
    return 0;
}

static int isAvailable(lua_State* L)
{
    lua_pushboolean(L,cameraplugin::isAvailable());
    return 1;
}


static int loader(lua_State* L)
{
	const luaL_Reg functionlist[] = {
		{"start", start},
		{"stop", stop},
        {"availableDevices", availableDevices},
        {"isAvailable", isAvailable},
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
	G_UNUSED(L);
	if (cameraplugin::cameraTexture)
	{
		cameraplugin::cameraTexture->unref();
		cameraplugin::cameraTexture=NULL;
	}
	cameraplugin::deinit();
}


#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || (TARGET_OS_OSX && !defined(QT_CORE_LIB))
REGISTER_PLUGIN_STATICNAMED_CPP("Camera", "1.0",Camera)
#else
REGISTER_PLUGIN("Camera", "1.0")
#endif
