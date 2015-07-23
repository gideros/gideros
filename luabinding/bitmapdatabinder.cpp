#include "bitmapdatabinder.h"
#include "bitmapdata.h"
#include "texturebase.h"
#include "stackchecker.h"

BitmapDataBinder::BitmapDataBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
        {"setRegion", setRegion},
        {"getRegion", getRegion},
        {NULL, NULL},
    };

	binder.createClass("TextureRegion", NULL, create, destruct, functionList);
}

int BitmapDataBinder::create(lua_State* L)
{
	StackChecker checker(L, "BitmapDataBinder::create", 1);

	Binder binder(L);

	TextureBase* textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase", 1));

	BitmapData* bitmapData = 0;
	if (lua_gettop(L) > 1)
	{
		int x = luaL_checkinteger(L, 2);
		int y = luaL_checkinteger(L, 3);
		int width = luaL_checkinteger(L, 4);
		int height = luaL_checkinteger(L, 5);
		int dx1 = luaL_optint(L, 6, 0);
		int dy1 = luaL_optint(L, 7, 0);
		int dx2 = luaL_optint(L, 8, 0);
		int dy2 = luaL_optint(L, 9, 0);

		bitmapData = new BitmapData(textureBase, x, y, width, height, dx1, dy1, dx2, dy2);
	}
	else
		bitmapData = new BitmapData(textureBase);
	
	binder.pushInstance("TextureRegion", bitmapData);

	return 1;
}

int BitmapDataBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	BitmapData* bitmapData = static_cast<BitmapData*>(ptr);
	bitmapData->unref();

	return 0;
}

int BitmapDataBinder::setRegion(lua_State *L)
{
    Binder binder(L);

    BitmapData* bitmapData = static_cast<BitmapData*>(binder.getInstance("TextureRegion", 1));

    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int width = luaL_checkinteger(L, 4);
    int height = luaL_checkinteger(L, 5);
    int dx1 = luaL_optint(L, 6, 0);
    int dy1 = luaL_optint(L, 7, 0);
    int dx2 = luaL_optint(L, 8, 0);
    int dy2 = luaL_optint(L, 9, 0);

    bitmapData->setRegion(x, y, width, height, dx1, dy1, dx2, dy2);

    return 0;
}

int BitmapDataBinder::getRegion(lua_State *L)
{
    Binder binder(L);

    BitmapData* bitmapData = static_cast<BitmapData*>(binder.getInstance("TextureRegion", 1));

    int x, y, width, height, dx1, dy1, dx2, dy2;
    bitmapData->getRegion(&x, &y, &width, &height, &dx1, &dy1, &dx2, &dy2);

    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    lua_pushinteger(L, dx1);
    lua_pushinteger(L, dy1);
    lua_pushinteger(L, dx2);
    lua_pushinteger(L, dy2);

    return 8;
}
