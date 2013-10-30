#include "texturepackbinder.h"
#include "texturepack.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include "giderosexception.h"
#include <string.h>
#include <luautil.h>

TexturePackBinder::TexturePackBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"getLocation", TexturePackBinder::getLocation},
		{NULL, NULL},
	};

	binder.createClass("TexturePack", "TextureBase", create, destruct, functionList);
}

int TexturePackBinder::create(lua_State* L)
{
	StackChecker checker(L, "TexturePackBinder::create", 1);

	LuaApplication* luaapplication = static_cast<LuaApplication*>(luaL_getdata(L));
	Application* application = luaapplication->getApplication();

	if (lua_type(L, 1) == LUA_TTABLE)
	{
		// collect the filenames to vector<string>
		std::vector<std::string> fileNames;
		int n = lua_objlen(L, 1);
		for (int i = 1; i <= n; ++i)
		{
			lua_rawgeti(L, 1, i);
			fileNames.push_back(luaL_checkstring(L, -1));
			lua_pop(L, 1);
		}

		int padding = luaL_optint(L, 2, 2);

		bool smoothing = lua_toboolean(L, 3);

		bool maketransparent = false;
		unsigned int transparentcolor = 0x00000000;
        Format format = eRGBA8888;
        if (!lua_isnoneornil(L, 4))
		{
			if (lua_type(L, 4) != LUA_TTABLE)
				return luaL_typerror(L, 3, "table");

			lua_getfield(L, 4, "transparentColor");
			if (!lua_isnil(L, -1))
			{
				maketransparent = true;
				transparentcolor = luaL_checkinteger(L, -1);
			}
			lua_pop(L, 1);

            lua_getfield(L, 4, "format");
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


		// collect the pointers to filenames into vector<const char*>
		std::vector<const char*> fileNamePointers;
		for (std::size_t i = 0; i < fileNames.size(); ++i)
			fileNamePointers.push_back(fileNames[i].c_str());
		fileNamePointers.push_back(0);
		
		Binder binder(L);

		TexturePack* texturePack = 0;
		try
		{
            texturePack = new TexturePack(application,
                                          &fileNamePointers[0],
                                          padding,
                                          smoothing ? eLinear : eNearest,
                                          eClamp,
                                          format,
                                          maketransparent,
                                          transparentcolor);
		}
		catch (const GiderosException& e)
		{
			luaL_error(L, e.what());				// TODO: burada luaL_error dedigimiz icin longjmp yuzunden vectorlerin destructor'lari cagirilmiyor
			return 0;
		}

		binder.pushInstance("TexturePack", texturePack);
	}
	else if (lua_type(L, 1) == LUA_TSTRING && lua_type(L, 2) == LUA_TSTRING)
	{
		const char* texturelistfile = lua_tostring(L, 1);
		const char* imagefile = lua_tostring(L, 2);
		bool smoothing = lua_toboolean(L, 3);

		bool maketransparent = false;
		unsigned int transparentcolor = 0x00000000;
        Format format = eRGBA8888;
		if (!lua_isnoneornil(L, 4))
		{
			if (lua_type(L, 4) != LUA_TTABLE)
				return luaL_typerror(L, 3, "table");

			lua_getfield(L, 4, "transparentColor");
			if (!lua_isnil(L, -1))
			{
				maketransparent = true;
				transparentcolor = luaL_checkinteger(L, -1);
			}
			lua_pop(L, 1);

            lua_getfield(L, 4, "format");
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

		TexturePack* texturePack = 0;
		try
		{
            texturePack = new TexturePack(application,
                                          texturelistfile,
                                          imagefile,
                                          smoothing ? eLinear : eNearest,
                                          eClamp,
                                          format,
                                          maketransparent,
                                          transparentcolor);
		}
		catch (const GiderosException& e)
		{
			luaL_error(L, e.what());
			return 0;
		}

		binder.pushInstance("TexturePack", texturePack);
	}
	else
	{
		return luaL_error(L, "Bad argument to 'TexturePack.new'. Candidates are TexturePack.new(table) and TexturePack.new(string, string).");
	}

	return 1;
}


int TexturePackBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	TexturePack* texturePack = static_cast<TexturePack*>(ptr);
	texturePack->unref();

	return 0;
}


int TexturePackBinder::getLocation(lua_State* L)
{
	StackChecker checker(L, "TexturePackBinder::getLocation", 8);
	
	Binder binder(L);
	TexturePack* texturePack = static_cast<TexturePack*>(binder.getInstance("TexturePack", 1));

	if (lua_type(L, 2) != LUA_TNUMBER && lua_type(L, 2) != LUA_TSTRING)
		luaL_typerror(L, 2, "number or string");

	int x = 0, y = 0, width = 0, height = 0, dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
	bool success;

	if (lua_type(L, 2) == LUA_TNUMBER)
	{
		int index = lua_tointeger(L, 2);
		success = texturePack->location(index - 1, &x, &y, &width, &height, &dx1, &dy1, &dx2, &dy2); // indices are started from 1 in lua
	}
	else
	{
		const char* name = lua_tostring(L, 2);
		success = texturePack->location(name, &x, &y, &width, &height, &dx1, &dy1, &dx2, &dy2);
	}

	if (success == true)
	{
		lua_pushinteger(L, x);
		lua_pushinteger(L, y);
		lua_pushinteger(L, width);
		lua_pushinteger(L, height);
		lua_pushinteger(L, dx1);
		lua_pushinteger(L, dy1);
		lua_pushinteger(L, dx2);
		lua_pushinteger(L, dy2);
	}
	else
	{
		lua_pushnil(L);			// in fact, this is unnecassary, we should just return 0. But stackchecker requires this sillyness :)
		lua_pushnil(L);			// note 2: I think there is difference :)
		lua_pushnil(L);
		lua_pushnil(L);
		lua_pushnil(L);
		lua_pushnil(L);
		lua_pushnil(L);
		lua_pushnil(L);
	}

	return 8;
}





