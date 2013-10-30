#include "texturebinder.h"
#include "texture.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include "giderosexception.h"
#include <string.h>
#include <luautil.h>

TextureBinder::TextureBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{NULL, NULL},
	};

	binder.createClass("Texture", "TextureBase", create, destruct, functionList);
}

int TextureBinder::create(lua_State* L)
{
	StackChecker checker(L, "TextureBinder::create", 1);

	LuaApplication* luaapplication = static_cast<LuaApplication*>(luaL_getdata(L));
	Application* application = luaapplication->getApplication();

	const char* filename = luaL_checkstring(L, 1);

	bool smoothing = lua_toboolean(L, 2);

	bool maketransparent = false;
	unsigned int transparentcolor = 0x00000000;
    Wrap wrap = eClamp;
    Format format = eRGBA8888;
	if (!lua_isnoneornil(L, 3))
	{
		if (lua_type(L, 3) != LUA_TTABLE)
			return luaL_typerror(L, 3, "table");

		lua_getfield(L, 3, "transparentColor");
		if (!lua_isnil(L, -1))
		{
			maketransparent = true;
			transparentcolor = luaL_checkinteger(L, -1);
		}
		lua_pop(L, 1);

        lua_getfield(L, 3, "wrap");
        if (!lua_isnil(L, -1))
        {
            const char *wrapstr = luaL_checkstring(L, -1);
            if (strcmp(wrapstr, "clamp") == 0)
                wrap = eClamp;
            else if (strcmp(wrapstr, "repeat") == 0)
                wrap = eRepeat;
            else
            {
                GStatus status(2008, "wrap");		// Error #2008: Parameter %s must be one of the accepted values.
                luaL_error(L, status.errorString());
            }
        }
        lua_pop(L, 1);

        lua_getfield(L, 3, "format");
        if (!lua_isnil(L, -1))
        {
            const char *formatstr = luaL_checkstring(L, -1);
            if (strcmp(formatstr, "rgba8888") == 0)
                format = eRGBA8888;
            else if (strcmp(formatstr, "rgb888") == 0)
                format = eRGB888;
            else if (strcmp(formatstr, "rgb565") == 0)
                format = eRGB565;
            else if (strcmp(formatstr, "rgba4444") == 0)
                format = eRGBA4444;
            else if (strcmp(formatstr, "rgba5551") == 0)
                format = eRGBA5551;
            else
            {
                GStatus status(2008, "format");		// Error #2008: Parameter %s must be one of the accepted values.
                luaL_error(L, status.errorString());
            }
        }
        lua_pop(L, 1);
    }

	
	Binder binder(L);

	Texture* texture = 0;
	try
	{
        texture = new Texture(application, filename, smoothing ? eLinear : eNearest, wrap, format, maketransparent, transparentcolor);
	}
	catch (const GiderosException& e)
	{
		return luaL_error(L, e.what());
	}

	binder.pushInstance("Texture", texture);
	return 1;
}

int TextureBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	Texture* texture = static_cast<Texture*>(ptr);
	texture->unref();

	return 0;
}
