#include "texturebasebinder.h"
#include "texturebase.h"
#include "stackchecker.h"

TextureBaseBinder::TextureBaseBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"getWidth", TextureBaseBinder::getWidth},
		{"getHeight", TextureBaseBinder::getHeight},
		{"getTexelSize", TextureBaseBinder::getTexelSize},
		{NULL, NULL},
	};

	binder.createClass("TextureBase", NULL, NULL, NULL, functionList);

    lua_getglobal(L, "TextureBase");
    lua_pushstring(L, "repeat");
    lua_setfield(L, -2, "REPEAT");
    lua_pushstring(L, "clamp");
    lua_setfield(L, -2, "CLAMP");
    lua_pushstring(L, "rgba8888");
    lua_setfield(L, -2, "RGBA8888");
    lua_pushstring(L, "rgb888");
    lua_setfield(L, -2, "RGB888");
    lua_pushstring(L, "rgb565");
    lua_setfield(L, -2, "RGB565");
    lua_pushstring(L, "rgba4444");
    lua_setfield(L, -2, "RGBA4444");
    lua_pushstring(L, "rgba5551");
    lua_setfield(L, -2, "RGBA5551");
    lua_pop(L, 1);
}

int TextureBaseBinder::getWidth(lua_State* L)
{
	StackChecker checker(L, "TextureBaseBinder::getWidth", 1);
	
	Binder binder(L);
	TextureBase* textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase"));

	lua_pushinteger(L, textureBase->data->baseWidth);

	return 1;
}

int TextureBaseBinder::getHeight(lua_State* L)
{
	StackChecker checker(L, "TextureBaseBinder::getHeight", 1);
	
	Binder binder(L);
	TextureBase* textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase"));

	lua_pushinteger(L, textureBase->data->baseHeight);

	return 1;
}

int TextureBaseBinder::getTexelSize(lua_State* L)
{
	StackChecker checker(L, "TextureBaseBinder::getTexelSize", 2);

	Binder binder(L);
	TextureBase* textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase"));

	lua_pushnumber(L, 1.0/textureBase->data->exwidth);
	lua_pushnumber(L, 1.0/textureBase->data->exheight);

	return 2;
}

