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
	 lua_rawseti(L,-2,(int)(k+1));
 }
 return 1;
}

static int setOrientation(lua_State *L)
{
    Orientation orientation=eFixed;
    const char *ors;
    if (lua_isnoneornil(L, 1)) {
        lua_getglobal(L,"application");
        lua_getfield(L,-1,"getOrientation");
        lua_getglobal(L,"application");
        lua_call(L,1,1);
        ors=lua_tostring(L,-1);
        lua_pop(L,1);
    }
    else
        ors=luaL_checkstring(L, 1);
    orientation=ePortrait;
    if (!strcmp(ors,"landscapeLeft"))
        orientation=eLandscapeLeft;
    else if (!strcmp(ors,"landscapeRight"))
        orientation=eLandscapeRight;
    else if (!strcmp(ors,"portraitUpsideDown"))
        orientation=ePortraitUpsideDown;
    cameraplugin::setOrientation(orientation);
    return 0;
}

static int setFlash(lua_State* L) {
	lua_pushboolean(L,cameraplugin::setFlash((int)luaL_optinteger(L,1,0)));
	return 1;
}

static int takePicture(lua_State* L) {
	lua_pushboolean(L,cameraplugin::takePicture());
	return 1;
}

static int queryCamera(lua_State* L) {
	const char *name=luaL_optstring(L,1,NULL);
	Orientation orientation=eFixed;
	lua_getglobal(L,"application");
	LuaApplication* application=static_cast<LuaApplication *>(luaL_getdata(L));
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
	cameraplugin::CameraInfo ci=cameraplugin::queyCamera(name,orientation);
	 lua_newtable(L);

	 lua_createtable(L,(int)ci.previewSizes.size(),0);
	 for (size_t k=0;k<ci.previewSizes.size();k++) {
		 lua_pushinteger(L,ci.previewSizes[k]);
		 lua_rawseti(L,-2,(int)k+1);
	 }
	 lua_setfield(L,-2,"previewSizes");

	 lua_createtable(L,(int)ci.pictureSizes.size(),0);
	 for (size_t k=0;k<ci.pictureSizes.size();k++) {
		 lua_pushinteger(L,ci.pictureSizes[k]);
		 lua_rawseti(L,-2,(int)k+1);
	 }
	 lua_setfield(L,-2,"pictureSizes");

	 const char *oname=NULL;
	 switch (ci.angle)
	 {
	 case 0: oname="portrait"; break;
	 case 90: oname="landscapeLeft"; break;
	 case 180: oname="portraitUpsideDown"; break;
	 case 270: oname="landscapeRight"; break;
	 }
	 if (oname) {
		 lua_pushstring(L,oname);
		 lua_setfield(L,-2,"angle");
	 }

	 lua_createtable(L,(int)ci.flashModes.size(),0);
	 for (size_t k=0;k<ci.flashModes.size();k++) {
		 lua_pushinteger(L,ci.flashModes[k]);
		 lua_rawseti(L,-2,(int)k+1);
	 }
	 lua_setfield(L,-2,"flashModes");

	return 1;
}

static int start(lua_State* L)
{
	GRenderTarget* textureBase = static_cast<GRenderTarget*>(g_getInstance(L,"RenderTarget",1));
	const char *name=luaL_optstring(L,2,NULL);
	int picwidth=(int)luaL_optinteger(L,3,0);
	int picheight=(int)luaL_optinteger(L,4,0);
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
	cameraplugin::start(orientation,&camwidth,&camheight,name,&picwidth,&picheight);
	lua_pushnumber(L,camwidth);
	lua_pushnumber(L,camheight);
	lua_pushnumber(L,picwidth);
	lua_pushnumber(L,picheight);

	return 4;
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

static lua_State *L=NULL;
void cameraplugin::callback_s(int type, void *event, void *udata)
{
	char *data=((char *)event)+sizeof(int);
	int dsize=*((int *)event);

	lua_getglobal(L,"Camera");
	lua_getfield(L,-1,"onEvent");
	if (!lua_isnoneornil(L,-1)) {
        lua_pushinteger(L,type);
        lua_pushlstring(L,data,dsize);
        lua_call(L, 2, 0);
	}
	else
		lua_pop(L,1);
	lua_pop(L,1);
}

static int loader(lua_State* L)
{
	const luaL_Reg functionlist[] = {
		{"start", start},
		{"stop", stop},
        {"availableDevices", availableDevices},
        {"isAvailable", isAvailable},
        {"setFlash", setFlash},
        {"takePicture", takePicture},
        {"queryCamera", queryCamera},
        {"setOrientation", setOrientation},
		{NULL, NULL},
	};

	cameraplugin::cameraTexture=NULL;
	cameraplugin::init();
	::L=L;
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
