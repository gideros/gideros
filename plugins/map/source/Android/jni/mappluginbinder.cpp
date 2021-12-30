/*

Plugin methods

new
setDimensions
setPosition
hide
show
clear
setCenterCoordinates
setZoom
mapClicked
getMapClickLatitude
getMapClickLongitude
setType
getCenterLatitude
getCenterLongitude
addMarker
setMarkerTitle
setMarkerHue
setMarkerAlpha
setMarkerCoordinates
getMarkerTitle
getMarkerLatitude
getMarkerLongitude
getClickedMarkerIndex


A note from the developer:

A major pet peeve is code in which function doIt() calls object.doIt() which calls ThingDoer->doIt() which calls do_it() which calls do(it) which calls it->do(), etc.
This makes it terribly difficult to pinpoint where things actually happen, where things broke, and where changes need to be made.  The IAB plugin on which this
interface was based is quite guilty of that sort of re-use of function names through many levels of code.

To avoid that, while still following the structure and levels used in IAB, every function or method in this plugin will be named with a prefix:
mpbs_ for map plugin binder static functions in this module
mpb_ for MAPPLUGINBINDER methods implemented in this module
gmapplugin_ for static methods implemented in mapplugin.cpp
mpcpp_ for methods of class MapPluginCPPClass

For example, looking at the setDimensions function through all these layers:
Lua method "setDimensions" maps to mpbs_setDimensions (mpbs for map pluging binder static, static function)
mpbs_setDimensions calls mpb_setDimensions (mpb for map plugin binder (non static), member function of class MAPPLUGINBINDER
mpb_setDimensions calls gmapplugin_setDimensions(), static function in mapplugin.cpp
gmapplugin_setDimensions calls method mpcpp_setDimensions() of class MapPluginCPPClass
mpcpp_setDimensions calls the native java class setDimensions

PJH

*/

#include "mapplugin.h"
#include "gideros.h"
#include <map>
#include <string>
#include <glog.h>

// some Lua helper functions
#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif


class MAPPLUGINBINDER : public GEventDispatcherProxy
{
public:
    MAPPLUGINBINDER(lua_State *L) : L(L)
    {
		gmapplugin_initialize();
   }
    
    ~MAPPLUGINBINDER()
    {
		gmapplugin_destroy(mapplugin_);
    }

	void mpb_setDimensions(int width, int height)
	{
		gmapplugin_setDimensions(mapplugin_, width, height);
	}

	void mpb_setPosition(int x, int y)
	{
		gmapplugin_setPosition(mapplugin_, x, y);
	}

	void mpb_hide()
	{
		gmapplugin_hide(mapplugin_);
	}

	void mpb_show()
	{
		gmapplugin_show(mapplugin_);
	}

	void mpb_clear()
	{
		gmapplugin_clear(mapplugin_);
	}

	void mpb_setCenterCoordinates(double lat, double lon)
	{
		gmapplugin_setCenterCoordinates(mapplugin_, lat, lon);
	}

	void mpb_setZoom(int z)
	{
		gmapplugin_setZoom(mapplugin_, z);
	}

	int mpb_mapClicked()
	{
		return gmapplugin_mapClicked(mapplugin_);
	}

	double mpb_getMapClickLatitude()
	{
		return gmapplugin_getMapClickLatitude(mapplugin_);
	}

	double mpb_getMapClickLongitude()
	{
		return gmapplugin_getMapClickLongitude(mapplugin_);
	}

	void mpb_setType(int t)
	{
		gmapplugin_setType(mapplugin_, t);
	}

	double mpb_getCenterLatitude()
	{
		return gmapplugin_getCenterLatitude(mapplugin_);
	}

	double mpb_getCenterLongitude()
	{
		return gmapplugin_getCenterLongitude(mapplugin_);
	}

	int mpb_addMarker(double lat, double lon, const char *title)
	{
		return gmapplugin_addMarker(mapplugin_, lat, lon, title);
	}

	void mpb_addMarkerAtIndex(double lat, double lon, const char *title, int index)
	{
		gmapplugin_addMarkerAtIndex(mapplugin_, lat, lon, title, index);
	}

	void mpb_setMarkerTitle(int idx, const char *title)
	{
		gmapplugin_setMarkerTitle(mapplugin_, idx, title);
	}

	void mpb_setMarkerHue(int idx, double hue)
	{
		gmapplugin_setMarkerHue(mapplugin_, idx, hue);
	}

	void mpb_setMarkerAlpha(int idx, int alpha)
	{
		gmapplugin_setMarkerAlpha(mapplugin_, idx, alpha);
	}

	void mpb_setMarkerCoordinates(int idx, double lat, double lon)
	{
		gmapplugin_setMarkerCoordinates(mapplugin_, idx, lat, lon);
	}

	const char *mpb_getMarkerTitle(int idx)
	{
		return gmapplugin_getMarkerTitle(mapplugin_, idx);
	}

	double mpb_getMarkerLatitude(int idx)
	{
		return gmapplugin_getMarkerLatitude(mapplugin_, idx);
	}

	double mpb_getMarkerLongitude(int idx)
	{
		return gmapplugin_getMarkerLongitude(mapplugin_, idx);
	}

	int mpb_getClickedMarkerIndex()
	{
		return gmapplugin_getClickedMarkerIndex(mapplugin_);
	}

private:
    lua_State *L;
    const char* mapplugin_;
};

static int destruct(void *p)
{
	void *ptr = GIDEROS_DTOR_UDATA(p);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	MAPPLUGINBINDER *mapplugin = static_cast<MAPPLUGINBINDER*>(object->proxy());
	
	mapplugin->unref();
	
	return 0;
}

static MAPPLUGINBINDER *getInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "MAPPLUGIN", index));
	MAPPLUGINBINDER *mapplugin = static_cast<MAPPLUGINBINDER*>(object->proxy());
	return mapplugin;
}


static int init(lua_State* L)
{
	MAPPLUGINBINDER *mapplugin = new MAPPLUGINBINDER(L);
	g_pushInstance(L, "MAPPLUGIN", mapplugin->object());

	lua_pushvalue(L, -1);
	return 1;
}

static int mpbs_setDimensions(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);
	int width = lua_tonumber(L, 2);
	int height = lua_tonumber(L, 3);

	mapplugin->mpb_setDimensions(width, height);

	return 1;
}



static int mpbs_setPosition(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);
	int x = lua_tonumber(L, 2);
	int y = lua_tonumber(L, 3);

	mapplugin->mpb_setPosition(x, y);

	return 1;
}


static int mpbs_hide(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);

	mapplugin->mpb_hide();

	return 1;
}


static int mpbs_show(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);

	mapplugin->mpb_show();

	return 1;
}

static int mpbs_clear(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);

	mapplugin->mpb_clear();

	return 1;
}


static int mpbs_setCenterCoordinates(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);
	double lat = lua_tonumber(L, 2);
	double lon = lua_tonumber(L, 3);

	mapplugin->mpb_setCenterCoordinates(lat, lon);

	return 1;
}


static int mpbs_setZoom(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);
	int z = lua_tonumber(L, 2);

	mapplugin->mpb_setZoom(z);

	return 1;
}


static int mpbs_mapClicked(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);

	int result = mapplugin->mpb_mapClicked();

	lua_pushnumber(L, result);

	return 1;
}


static int mpbs_getMapClickLatitude(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);

	double result = mapplugin->mpb_getMapClickLatitude();

	lua_pushnumber(L, result);

	return 1;
}


static int mpbs_getMapClickLongitude(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);

	double result = mapplugin->mpb_getMapClickLongitude();

	lua_pushnumber(L, result);

	return 1;
}


static int mpbs_setType(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);
	int t = lua_tonumber(L, 2);

	mapplugin->mpb_setType(t);

	return 1;
}


static int mpbs_getCenterLatitude(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);

	double result = mapplugin->mpb_getCenterLatitude();

	lua_pushnumber(L, result);

	return 1;
}


static int mpbs_getCenterLongitude(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);

	double result = mapplugin->mpb_getCenterLongitude();

	lua_pushnumber(L, result);

	return 1;
}

static int mpbs_addMarker(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);
	double lat = lua_tonumber(L, 2);
	double lon = lua_tonumber(L, 3);
	const char *title = lua_tostring(L, 4);

	mapplugin->mpb_addMarker(lat, lon, title);

	return 1;
}

/*
static int mpbs_addMarkerAtIndex(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);
	double lat = lua_tonumber(L, 2);
	double lon = lua_tonumber(L, 3);
	const char *title = lua_tostring(L, 4);
	int index = lua_tonumber(L, 5);

	mapplugin->mpb_addMarkerAtIndex(lat, lon, title, index);

	return 1;
}
*/
static int mpbs_setMarkerTitle(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);
	int idx = lua_tonumber(L, 2);
	const char *title = lua_tostring(L, 3);

	mapplugin->mpb_setMarkerTitle(idx, title);

	return 1;
}


static int mpbs_setMarkerHue(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);
	int idx = lua_tonumber(L, 2);
	double hue = lua_tonumber(L, 3);

	mapplugin->mpb_setMarkerHue(idx, hue);

	return 1;
}


static int mpbs_setMarkerAlpha(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);
	int idx = lua_tonumber(L, 2);
	double alpha = lua_tonumber(L, 3);

	mapplugin->mpb_setMarkerAlpha(idx, alpha);

	return 1;
}


static int mpbs_setMarkerCoordinates(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);
	int idx = lua_tonumber(L, 2);
	double lat = lua_tonumber(L, 3);
	double lon = lua_tonumber(L, 4);

	mapplugin->mpb_setMarkerCoordinates(idx, lat, lon);

	return 1;
}


static int mpbs_getMarkerTitle(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);
	int idx = lua_tonumber(L, 2);

	const char *result = mapplugin->mpb_getMarkerTitle(idx);

	lua_pushlstring(L, result, strlen(result));

	return 1;
}


static int mpbs_getMarkerLatitude(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);
	int idx = lua_tonumber(L, 2);

	double result = mapplugin->mpb_getMarkerLatitude(idx);

	lua_pushnumber(L, result);

	return 1;
}


static int mpbs_getMarkerLongitude(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);
	int idx = lua_tonumber(L, 2);

	double result = mapplugin->mpb_getMarkerLongitude(idx);

	lua_pushnumber(L, result);

	return 1;
}


static int mpbs_getClickedMarkerIndex(lua_State* L)
{

    MAPPLUGINBINDER *mapplugin = getInstance(L, 1);

	int result = mapplugin->mpb_getClickedMarkerIndex();

	lua_pushnumber(L, result);

	return 1;
}



static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
        {"new", init},
		{"setDimensions", mpbs_setDimensions},
		{"setPosition", mpbs_setPosition},
		{"hide", mpbs_hide},
		{"show", mpbs_show},
		{"clear", mpbs_clear},
		{"setCenterCoordinates", mpbs_setCenterCoordinates},
		{"setZoom", mpbs_setZoom},
		{"mapClicked", mpbs_mapClicked},
		{"getMapClickLatitude", mpbs_getMapClickLatitude},
		{"getMapClickLongitude", mpbs_getMapClickLongitude},
		{"setType", mpbs_setType},
		{"getCenterLatitude", mpbs_getCenterLatitude},
		{"getCenterLongitude", mpbs_getCenterLongitude},
		{"addMarker", mpbs_addMarker},
//		{"addMarkerAtIndex", mpbs_addMarkerAtIndex},
		{"setMarkerTitle", mpbs_setMarkerTitle},
		{"setMarkerHue", mpbs_setMarkerHue},
		{"setMarkerAlpha", mpbs_setMarkerAlpha},
		{"setMarkerCoordinates", mpbs_setMarkerCoordinates},
		{"getMarkerTitle", mpbs_getMarkerTitle},
		{"getMarkerLatitude", mpbs_getMarkerLatitude},
		{"getMarkerLongitude", mpbs_getMarkerLongitude},
		{"getClickedMarkerIndex", mpbs_getClickedMarkerIndex},
		{NULL, NULL},
	};
    
    g_createClass(L, "MAPPLUGIN", "EventDispatcher", NULL, destruct, functionlist);    
    
    return 0;
}
    
static void g_initializePlugin(lua_State *L)
{
    lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcnfunction(L, loader, "plugin_init_mapplugin");
	lua_setfield(L, -2, "MAPPLUGIN");
	
	lua_pop(L, 2);
	gmapplugin_init();
}

static void g_deinitializePlugin(lua_State *L)
{
    gmapplugin_cleanup();
}

REGISTER_PLUGIN("MAPPLUGIN", "1.0")
