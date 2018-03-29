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
        {"layoutText", layoutText},
		{"getCharIndexAtOffset", getCharIndexAtOffset},
        {NULL, NULL},
	};

	binder.createClass("FontBase", NULL, NULL, NULL, functionList);
	lua_getglobal(L,"FontBase");
	lua_pushinteger(L,FontBase::TLF_BOTTOM); lua_setfield(L,-2,"TLF_BOTTOM");
	lua_pushinteger(L,FontBase::TLF_TOP); lua_setfield(L,-2,"TLF_TOP");
	lua_pushinteger(L,FontBase::TLF_CENTER); lua_setfield(L,-2,"TLF_CENTER");
	lua_pushinteger(L,FontBase::TLF_VCENTER); lua_setfield(L,-2,"TLF_VCENTER");
	lua_pushinteger(L,FontBase::TLF_RIGHT); lua_setfield(L,-2,"TLF_RIGHT");
	lua_pushinteger(L,FontBase::TLF_LEFT); lua_setfield(L,-2,"TLF_LEFT");
	lua_pushinteger(L,FontBase::TLF_JUSTIFIED); lua_setfield(L,-2,"TLF_JUSTIFIED");
	lua_pushinteger(L,FontBase::TLF_NOWRAP); lua_setfield(L,-2,"TLF_NOWRAP");
    lua_pushinteger(L,FontBase::TLF_REF_BASELINE); lua_setfield(L,-2,"TLF_REF_BASELINE");
    lua_pushinteger(L,FontBase::TLF_REF_TOP); lua_setfield(L,-2,"TLF_REF_TOP");
    lua_pushinteger(L,FontBase::TLF_REF_MIDDLE); lua_setfield(L,-2,"TLF_REF_MIDDLE");
    lua_pushinteger(L,FontBase::TLF_REF_BOTTOM); lua_setfield(L,-2,"TLF_REF_BOTTOM");
	lua_pushinteger(L,FontBase::TLF_BREAKWORDS); lua_setfield(L,-2,"TLF_BREAKWORDS");
    lua_pop(L,1);
}

int FontBaseBinder::getCharIndexAtOffset(lua_State *L)
{
    Binder binder(L);

    FontBase *font = static_cast<FontBase*>(binder.getInstance("FontBase", 1));

    const char *text = luaL_checkstring(L, 2);
    lua_Number offset = luaL_checknumber(L, 3);
    lua_Number letterSpacing = luaL_optnumber(L, 4, 0);

    float co=font->getCharIndexAtOffset(text,offset,letterSpacing);
    lua_pushnumber(L,co);

    return 1;
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

int FontBaseBinder::layoutText(lua_State *L)
{
    Binder binder(L);

    FontBase *font = static_cast<FontBase*>(binder.getInstance("FontBase", 1));

    FontBase::TextLayoutParameters tp;
    tp.w=luaL_optnumber(L,3,0);
    tp.h=luaL_optnumber(L,4,0);
    tp.flags=luaL_optinteger(L,5,(int)FontBase::TLF_NOWRAP);
    tp.letterSpacing=luaL_optnumber(L,6,0);
    tp.lineSpacing=luaL_optnumber(L,7,0);
    tp.tabSpace=luaL_optnumber(L,8,4);
    tp.breakchar=luaL_optstring(L,9,"");

    FontBase::TextLayout tl=font->layoutText(luaL_checkstring(L,2),&tp);
    lua_createtable(L,0,6);
    lua_pushnumber(L,tl.x);
    lua_setfield(L,-2,"x");
    lua_pushnumber(L,tl.y);
    lua_setfield(L,-2,"y");
    lua_pushnumber(L,tl.w);
    lua_setfield(L,-2,"w");
    lua_pushnumber(L,tl.h);
    lua_setfield(L,-2,"h");
    lua_pushinteger(L,tl.lines);
    lua_setfield(L,-2,"lines");

    lua_createtable(L,tl.parts.size(),0);
    for (size_t k=0;k<tl.parts.size();k++)
    {
    	FontBase::ChunkLayout cl=tl.parts[k];
    	lua_createtable(L,0,9);
        lua_pushnumber(L,cl.x);
        lua_setfield(L,-2,"x");
        lua_pushnumber(L,cl.y);
        lua_setfield(L,-2,"y");
        lua_pushnumber(L,cl.w);
        lua_setfield(L,-2,"w");
        lua_pushnumber(L,cl.h);
        lua_setfield(L,-2,"h");
        lua_pushnumber(L,cl.dx);
        lua_setfield(L,-2,"dx");
        lua_pushnumber(L,cl.dy);
        lua_setfield(L,-2,"dy");
        lua_pushstring(L,cl.text.c_str());
        lua_setfield(L,-2,"text");
        lua_pushlstring(L,&cl.sep,1);
        lua_setfield(L,-2,"sep");
        lua_pushnumber(L,cl.sepl);
        lua_setfield(L,-2,"sepl");
        lua_pushinteger(L,cl.line);
        lua_setfield(L,-2,"line");

        lua_rawseti(L,-2,k+1);
    }
    lua_setfield(L,-2,"parts");

    return 1;
}

