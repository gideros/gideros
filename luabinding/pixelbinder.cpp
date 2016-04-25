#include "pixelbinder.h"
#include "pixel.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include <luautil.h>

PixelBinder::PixelBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"setWidth", setWidth},
		{"setHeight", setHeight},
        {"setDimensions", setDimensions},
        {"setColor", setColor},
        {NULL, NULL},
	};

	binder.createClass("Pixel", "Sprite", create, destruct, functionList);
}

int PixelBinder::create(lua_State* L)
{
	StackChecker checker(L, "PixelBinder::create", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);

    Pixel* bitmap = new Pixel(application->getApplication());
	unsigned int color = luaL_optinteger(L, 1, 0);
	lua_Number alpha = luaL_optnumber(L, 2, 1.0);
	lua_Number w = luaL_optnumber(L, 3, 1.0);
	lua_Number h = luaL_optnumber(L, 4, w);
	int r = (color >> 16) & 0xff;
	int g = (color >> 8) & 0xff;
	int b = color & 0xff;

	bitmap->setColor(r/255.f,g/255.f,b/255.f,alpha);
	bitmap->setDimensions(w,h);

	binder.pushInstance("Pixel", bitmap);
	return 1;
}

int PixelBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	Pixel* bitmap = static_cast<Pixel*>(ptr);
	bitmap->unref();

	return 0;
}

int PixelBinder::setWidth(lua_State* L)
{
	Binder binder(L);

	Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

	lua_Number w = luaL_checknumber(L, 2);

	bitmap->setWidth(w);

	return 0;
}

int PixelBinder::setHeight(lua_State* L)
{
	Binder binder(L);

	Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

	lua_Number h = luaL_checknumber(L, 2);

	bitmap->setHeight(h);

	return 0;
}

int PixelBinder::setDimensions(lua_State* L)
{
	Binder binder(L);

	Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

	lua_Number w = luaL_checknumber(L, 2);
	lua_Number h = luaL_checknumber(L, 3);

	bitmap->setDimensions(w,h);

	return 0;
}

int PixelBinder::setColor(lua_State* L)
{
	Binder binder(L);

	Pixel* bitmap = static_cast<Pixel*>(binder.getInstance("Pixel", 1));

	unsigned int color = luaL_optinteger(L, 2, 0);
	lua_Number alpha = luaL_optnumber(L, 3, 1.0);

	int r = (color >> 16) & 0xff;
	int g = (color >> 8) & 0xff;
	int b = color & 0xff;

	bitmap->setColor(r/255.f,g/255.f,b/255.f,alpha);

	return 0;
}

