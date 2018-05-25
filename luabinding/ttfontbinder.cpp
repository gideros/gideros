#include "ttfontbinder.h"
#include "ttfont.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include "giderosexception.h"
#include "ttbmfont.h"
#include <luautil.h>

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
    LuaApplication *luaapplication = static_cast<LuaApplication*>(luaL_getdata(L));
    Application *application = luaapplication->getApplication();

    Binder binder(L);
	lua_Number size = luaL_checknumber(L, 2);

    GStatus status;

    FontBase *font;

	std::vector<FontBase::FontSpec> filenames;
	FontBase::FontSpec fspec;

	if (lua_istable(L,1))
	{
		int ll=lua_objlen(L,1);
		for (int i=1;i<=ll;i++)
		{
			lua_rawgeti(L,1,i);
	    	if (lua_istable(L,-1))
	    	{
	    		lua_getfield(L,-1,"file");
	    		fspec.filename=luaL_checkstring(L, -1);
        		lua_pop(L,1);
	    		lua_getfield(L,-1,"sizeMult");
	    		fspec.sizeMult=luaL_optnumber(L,-1,1.0);
        		lua_pop(L,1);
	    	}
	    	else
	    	{
	    		fspec.filename=luaL_checkstring(L, -1);
	    		fspec.sizeMult=1.0f;
	    	}
    		filenames.push_back(fspec);
    		lua_pop(L,1);
		}
	}
	else
	{
		fspec.filename=luaL_checkstring(L, 1);
		fspec.sizeMult=1.0f;
		filenames.push_back(fspec);
	}

    if (lua_type(L, 3) == LUA_TSTRING)
    {
        const char *chars = luaL_checkstring(L, 3);
        float smoothing=0;
        if (lua_isnumber(L,4))
        	smoothing = lua_tonumber(L, 4);
        else if (lua_toboolean(L,4))
        	smoothing=1;
        float outline=luaL_optnumber(L, 5, 0);
        font = new TTBMFont(application, filenames, size, chars, smoothing, outline, &status);
    }
    else
    {
        float smoothing=0;
        if (lua_isnumber(L,3))
        	smoothing = lua_tonumber(L, 3);
        else if (lua_toboolean(L,3))
        	smoothing=1;
        float outline=luaL_optnumber(L, 4, 0);
        font = new TTFont(application, filenames, size, smoothing, outline, &status);
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

