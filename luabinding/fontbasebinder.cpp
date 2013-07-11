#include "fontbasebinder.h"
#include <fontbase.h>

FontBaseBinder::FontBaseBinder(lua_State *L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
        {"getBounds", getBounds},
        {"getAdvanceX", getAdvanceX},
        {"getAscender", getAscender},
        {"getLineHeight", getLineHeight},
        {NULL, NULL},
	};

	binder.createClass("FontBase", NULL, NULL, NULL, functionList);
}

int FontBaseBinder::getBounds(lua_State *L)
{
    Binder binder(L);

    FontBase *font = static_cast<FontBase*>(binder.getInstance("FontBase", 1));

    const char *text = luaL_checkstring(L, 2);
    lua_Number letterSpacing = luaL_optnumber(L, 3, 0);

    float minx, miny, maxx, maxy;
    font->getBounds(text, letterSpacing, &minx, &miny, &maxx, &maxy);

    if (minx > maxx || miny > maxy)
    {
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
    }
    else
    {
        lua_pushnumber(L, minx);
        lua_pushnumber(L, miny);
        lua_pushnumber(L, maxx - minx);
        lua_pushnumber(L, maxy - miny);
    }

    return 4;
}

int FontBaseBinder::getAdvanceX(lua_State *L)
{
    Binder binder(L);

    FontBase *font = static_cast<FontBase*>(binder.getInstance("FontBase", 1));

    const char *text = luaL_checkstring(L, 2);
    lua_Number letterSpacing = luaL_optnumber(L, 3, 0);
    int size = luaL_optinteger(L, 4, -1);

    lua_pushnumber(L, font->getAdvanceX(text, letterSpacing, size));

    return 1;
}

int FontBaseBinder::getAscender(lua_State *L)
{
    Binder binder(L);

    FontBase *font = static_cast<FontBase*>(binder.getInstance("FontBase", 1));

    lua_pushnumber(L, font->getAscender());

    return 1;
}

int FontBaseBinder::getLineHeight(lua_State *L)
{
    Binder binder(L);

    FontBase *font = static_cast<FontBase*>(binder.getInstance("FontBase", 1));

    lua_pushnumber(L, font->getLineHeight());

    return 1;
}
