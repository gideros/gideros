#include "fontbinder.h"
#include "font.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include "giderosexception.h"
#include "application.h"
#include <luautil.h>

FontBinder::FontBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"getDefault",FontBinder::getDefault},
		{NULL, NULL},
	};

	binder.createClass("Font", "FontBase", create, destruct, functionList);
}

int FontBinder::create(lua_State* L)
{
	StackChecker checker(L, "FontBinder::create", 1);

	LuaApplication* luaapplication = static_cast<LuaApplication*>(luaL_getdata(L));
	Application* application = luaapplication->getApplication();

	const char* glympfile = luaL_checkstring(L, 1);
	const char* imagefile = luaL_checkstring(L, 2);
	bool smoothing = lua_toboolean(L, 3) != 0;

	Binder binder(L);
	
    GStatus status;
    Font *font = new Font(application, glympfile, imagefile, smoothing, &status);
    if (status.error())
    {
        delete font;
        return luaL_error(L, status.errorString());
	}

	binder.pushInstance("Font", font);
	return 1;
}

int FontBinder::getDefault(lua_State* L)
{
	StackChecker checker(L, "FontBinder::getDefault", 1);

	LuaApplication* luaapplication = static_cast<LuaApplication*>(luaL_getdata(L));
	Application* application = luaapplication->getApplication();

	Binder binder(L);

    Font *font = application->getDefaultFont();
    font->ref();

	binder.pushInstance("Font", font);
	return 1;
}

int FontBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	Font* font = static_cast<Font*>(ptr);
	font->unref();

	return 0;
}
