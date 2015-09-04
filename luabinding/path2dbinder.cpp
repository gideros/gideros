#include "path2dbinder.h"
#include <path.h>
#include <string.h>
#include "stackchecker.h"
#include "luaapplication.h"
#include <luautil.h>


Path2DBinder::Path2DBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"setLineColor", setLineColor},
		{"setFillColor", setFillColor},
		{"setPath", setPath},
		{"setTexture", setTexture},
		{"setLineThickness", setLineThickness },
		{NULL, NULL},
	};

	binder.createClass("Path2D", "Sprite", create, destruct, functionList);

	lua_getglobal(L, "Path2D");	// get Path2D metatable

/*	lua_pushstring(L, NONE);
	lua_setfield(L, -2, "NONE");

	lua_pushstring(L, SOLID);
	lua_setfield(L, -2, "SOLID");

	lua_pushstring(L, TEXTURE);
	lua_setfield(L, -2, "TEXTURE");

	lua_pushstring(L, EVEN_ODD);
	lua_setfield(L, -2, "EVEN_ODD");

	lua_pushstring(L, NON_ZERO);
	lua_setfield(L, -2, "NON_ZERO");
*/
	lua_pop(L, 1);
}

int Path2DBinder::create(lua_State* L)
{
	StackChecker checker(L, "Path2DBinder::create", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);

    Path2D* shape = new Path2D(application->getApplication());
	binder.pushInstance("Path2D", shape);

	return 1;
}

int Path2DBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	Path2D* shape = static_cast<Path2D*>(ptr);
	shape->unref();

	return 0;
}

int Path2DBinder::setFillColor(lua_State* L)
{
	Binder binder(L);
	Path2D* shape = static_cast<Path2D*>(binder.getInstance("Path2D", 1));
	
	unsigned int color = luaL_optinteger(L, 2, 0);
	lua_Number alpha = luaL_optnumber(L, 3, 1.0);
	shape->setFillColor(color, alpha);

	return 0;
}

int Path2DBinder::setLineColor(lua_State* L)
{
	StackChecker checker(L, "Path2DBinder::setLineStyle", 0);

	Binder binder(L);
	Path2D* shape = static_cast<Path2D*>(binder.getInstance("Path2D"));

	unsigned int color = luaL_optinteger(L, 2, 0);
	lua_Number alpha = luaL_optnumber(L, 3, 1.0);

	shape->setLineColor(color, alpha);

	return 0;
}

int Path2DBinder::setTexture(lua_State *L)
{
    Binder binder(L);

    Path2D *bitmap = static_cast<Path2D*>(binder.getInstance("Path2D", 1));
    TextureBase *textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase", 2));
    bitmap->setTexture(textureBase);

    return 0;
}

int Path2DBinder::setLineThickness(lua_State* L)
{
	StackChecker checker(L, "Path2DBinder::setLineStyle", 0);

	Binder binder(L);
	Path2D* shape = static_cast<Path2D*>(binder.getInstance("Path2D"));

	double thickness = luaL_checknumber(L, 2);

	shape->setLineThickness(thickness);

	return 0;
}

int Path2DBinder::setPath(lua_State* L)
{
	Binder binder(L);
	Path2D* shape = static_cast<Path2D*>(binder.getInstance("Path2D", 1));

	const char* commands = luaL_checkstring(L, 2);

	std::vector<float> coords;
    if (lua_type(L, 3) == LUA_TTABLE)
    {
        int n = lua_objlen(L, 3);
        coords.resize(n);
        for (int i = 0; i < n; ++i)
        {
            lua_rawgeti(L, 3, i + 1);
            coords[i] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
    }
    else
    {
        int n = lua_gettop(L) - 2;
        coords.resize(n);
        for (int i = 0; i < n; ++i)
            coords[i] = luaL_checknumber(L, i + 3);
    }

    shape->setPath(strlen(commands),(const unsigned char *)commands,coords.size(),&(coords[0]));

	return 0;
}
