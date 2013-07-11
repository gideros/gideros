#include "dibbinder.h"
#include "stackchecker.h"
#include <dib.h>
#include "luaapplication.h"

DibBinder::DibBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"getPixel", getPixel},
		{NULL, NULL},
	};

	binder.createClass("Dib", NULL, create, destruct, functionList);
}

int DibBinder::create(lua_State* L)
{
	StackChecker checker(L, "DibBinder::create", 1);

	LuaApplication* luaapplication = static_cast<LuaApplication*>(lua_getdata(L));
	Application* application = luaapplication->getApplication();

	const char* filename = luaL_checkstring(L, 1);
	
	Binder binder(L);

	Dib* dib = new Dib(application, filename, false);
	binder.pushInstance("Dib", dib);

	return 1;
}

int DibBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	Dib* dib = static_cast<Dib*>(ptr);
	delete dib;

	return 0;
}

int DibBinder::getPixel(lua_State* L)
{
	StackChecker checker(L, "DibBinder::getPixel", 1);

	Binder binder(L);
	Dib* dib = static_cast<Dib*>(binder.getInstance("Dib", 1));

	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);

	unsigned char rgba[4];
	dib->getPixel(x, y, rgba);

	unsigned char r = rgba[0];
	unsigned char g = rgba[1];
	unsigned char b = rgba[2];
	unsigned char a = rgba[3];

	unsigned int c = (a << 24) | (r << 16) | (g << 8) | b;

	lua_pushinteger(L, c);

	return 1;
}

/*int DibBinder::setPixel(lua_State* L)
{
	StackChecker checker(L, "DibBinder::setPixel", 1);

	Binder binder(L);
	Dib* dib = static_cast<Dib*>(binder.getInstance("Dib", 1));

	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	unsigned int c = luaL_checkinteger(L, 3);





	return 0;
}
*/
