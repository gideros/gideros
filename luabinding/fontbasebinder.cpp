#include "fontbasebinder.h"
#include <fontbase.h>
#include "luaapplication.h"
#include <luautil.h>
#include <utf8.h>

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
    lua_pushinteger(L,FontBase::TLF_RTL); lua_setfield(L,-2,"TLF_RTL");
    lua_pushinteger(L,FontBase::TLF_REF_BASELINE); lua_setfield(L,-2,"TLF_REF_BASELINE");
    lua_pushinteger(L,FontBase::TLF_REF_TOP); lua_setfield(L,-2,"TLF_REF_TOP");
    lua_pushinteger(L,FontBase::TLF_REF_MIDDLE); lua_setfield(L,-2,"TLF_REF_MIDDLE");
    lua_pushinteger(L,FontBase::TLF_REF_BOTTOM); lua_setfield(L,-2,"TLF_REF_BOTTOM");
    lua_pushinteger(L,FontBase::TLF_REF_LINETOP); lua_setfield(L,-2,"TLF_REF_LINETOP");
    lua_pushinteger(L,FontBase::TLF_REF_LINEBOTTOM); lua_setfield(L,-2,"TLF_REF_LINEBOTTOM");
	lua_pushinteger(L,FontBase::TLF_BREAKWORDS); lua_setfield(L,-2,"TLF_BREAKWORDS");
    lua_pushinteger(L,FontBase::TLF_LTR); lua_setfield(L,-2,"TLF_LTR");
    lua_pushinteger(L,FontBase::TLF_NOSHAPING); lua_setfield(L,-2,"TLF_NOSHAPING");
    lua_pushinteger(L,FontBase::TLF_NOBIDI); lua_setfield(L,-2,"TLF_NOBIDI");
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

int FontBaseBinder::getDescender(lua_State *L)
{
    Binder binder(L);

    FontBase *font = static_cast<FontBase*>(binder.getInstance("FontBase", 1));

    lua_pushnumber(L, font->getDescender());

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
    	lua_createtable(L,0,11);
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
        char seputf[8];
        int sepsz=wchar_to_utf8(&cl.sep,1,seputf,8,0);
        lua_pushlstring(L,seputf,sepsz);
        lua_setfield(L,-2,"sep");
        lua_pushnumber(L,cl.sepl);
        lua_setfield(L,-2,"sepl");
        lua_pushinteger(L,cl.line);
        lua_setfield(L,-2,"line");

        lua_createtable(L,cl.shaped.size(),0);
        for (size_t l=0;l<cl.shaped.size();l++)
        {
        	FontBase::GlyphLayout gl=cl.shaped[l];
        	lua_createtable(L,0,4);
        	lua_pushinteger(L,gl.glyph);
            lua_setfield(L,-2,"glyph");
        	lua_pushinteger(L,gl.srcIndex);
            lua_setfield(L,-2,"srcIndex");
            lua_pushnumber(L,gl.advX);
            lua_setfield(L,-2,"advX");
            lua_pushnumber(L,gl.advY);
            lua_setfield(L,-2,"advY");
            lua_rawseti(L,-2,l+1);
        }
        lua_setfield(L,-2,"glyphs");

        lua_rawseti(L,-2,k+1);
    }
    lua_setfield(L,-2,"parts");

    return 1;
}

CompositeFontBinder::CompositeFontBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{NULL, NULL},
	};

	binder.createClass("CompositeFont", "FontBase", create, destruct, functionList);
}

int CompositeFontBinder::create(lua_State* L)
{
    LuaApplication *luaapplication = static_cast<LuaApplication*>(luaL_getdata(L));
    Application *application = luaapplication->getApplication();

    Binder binder(L);

    CompositeFont *font;

	std::vector<CompositeFont::CompositeFontSpec> fonts;
	CompositeFont::CompositeFontSpec fspec;

	luaL_checktype(L,1,LUA_TTABLE);
	int ll=lua_objlen(L,1);
	for (int i=1;i<=ll;i++)
	{
		lua_rawgeti(L,1,i);
		luaL_checktype(L,-1,LUA_TTABLE);

		lua_getfield(L,-1,"font");
		fspec.font = static_cast<BMFontBase*>(binder.getInstance("FontBase", -1));
		if (fspec.font->getType()==FontBase::eTTFont)
		{
			lua_pushstring(L,"TTFont with nil charset specification are unsupported");
			lua_error(L);
		}
		lua_pop(L,1);

		lua_getfield(L,-1,"x");
		fspec.offsetX=luaL_optnumber(L,-1,0.0);
		lua_pop(L,1);
		lua_getfield(L,-1,"y");
		fspec.offsetY=luaL_optnumber(L,-1,0.0);
		lua_pop(L,1);
		lua_getfield(L,-1,"color");
		int color=luaL_optinteger(L,-1,-1);
		if (color>=0) {
			int r = (color >> 16) & 0xff;
			int g = (color >> 8) & 0xff;
			int b = color & 0xff;
			fspec.colorR=r / 255.f;
			fspec.colorG=g / 255.f;
			fspec.colorB=b / 255.f;
		}
		else {
			fspec.colorR=-1;
			fspec.colorG=-1;
			fspec.colorB=-1;
		}
		lua_pop(L,1);
		lua_getfield(L,-1,"alpha");
		fspec.colorA=luaL_optnumber(L,-1,-1);
		lua_pop(L,1);

		fonts.push_back(fspec);
		lua_pop(L,1);
	}

	font = new CompositeFont(application, fonts);

    binder.pushInstance("CompositeFont", font);
	return 1;
}


int CompositeFontBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
    FontBase *font = static_cast<FontBase*>(ptr);
    font->unref();

	return 0;
}

