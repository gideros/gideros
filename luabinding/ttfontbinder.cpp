#include "ttfontbinder.h"
#include "ttfont.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include "giderosexception.h"
#include "ttbmfont.h"

TTFontBinder::TTFontBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{NULL, NULL},
	};

	binder.createClass("TTFont", "FontBase", create, destruct, functionList);
}

int TTFontBinder::create(lua_State* L)
{
    LuaApplication *luaapplication = static_cast<LuaApplication*>(lua_getdata(L));
    Application *application = luaapplication->getApplication();

    Binder binder(L);
    const char *filename = luaL_checkstring(L, 1);
	lua_Number size = luaL_checknumber(L, 2);

    GStatus status;

    FontBase *font;

    if (lua_type(L, 3) == LUA_TSTRING)
    {
        const char *chars = luaL_checkstring(L, 3);
        bool smoothing = lua_toboolean(L, 4);
        font = new TTBMFont(application, filename, size, chars, smoothing, &status);
    }
    else
    {
        bool smoothing = lua_toboolean(L, 3);
        font = new TTFont(application, filename, size, smoothing, &status);
    }

    if (status.error())
    {
        delete font;
        return luaL_error(L, status.errorString());
    }

    binder.pushInstance("TTFont", font);
	return 1;
}


int TTFontBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
    FontBase *font = static_cast<FontBase*>(ptr);
    font->unref();

	return 0;
}

