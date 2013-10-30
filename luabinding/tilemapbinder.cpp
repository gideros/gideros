#include "tilemapbinder.h"
#include "tilemap.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include <luautil.h>

TileMapBinder::TileMapBinder(lua_State* L)
{
	Binder binder(L);
	
	static const luaL_Reg functionList[] = {
		{"getTile", TileMapBinder::getTile},
		{"setTile", TileMapBinder::setTile},
		{"clearTile", TileMapBinder::clearTile},
		{"shift", TileMapBinder::shift},
		{NULL, NULL},
	};
	
	binder.createClass("TileMap", "Sprite", create, destruct, functionList);

    lua_getglobal(L, "TileMap");
    lua_pushinteger(L, TileMap::FLIP_HORIZONTAL);
    lua_setfield(L, -2, "FLIP_HORIZONTAL");
    lua_pushinteger(L, TileMap::FLIP_VERTICAL);
    lua_setfield(L, -2, "FLIP_VERTICAL");
    lua_pushinteger(L, TileMap::FLIP_DIAGONAL);
    lua_setfield(L, -2, "FLIP_DIAGONAL");
    lua_pop(L, 1);
}

int TileMapBinder::create(lua_State* L)
{
	StackChecker checker(L, "TileMapBinder::create", 1);
	
	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	Binder binder(L);
	
	int width = luaL_checkinteger(L, 1);
	int height = luaL_checkinteger(L, 2);
	TextureBase* texturebase = static_cast<TextureBase*>(binder.getInstance("TextureBase", 3));	
	int tilewidth = luaL_checkinteger(L, 4);
	int tileheight = luaL_checkinteger(L, 5);
	int spacingx = luaL_optinteger(L, 6, 0);
	int spacingy = luaL_optinteger(L, 7, 0);
	int marginx = luaL_optinteger(L, 8, 0);
	int marginy = luaL_optinteger(L, 9, 0);
	int displaywidth = luaL_optinteger(L, 10, tilewidth);
	int displayheight = luaL_optinteger(L, 11, tileheight);

	TileMap* tilemap = new TileMap(application->getApplication(),
								   width, height,
								   texturebase,
								   tilewidth, tileheight,
								   spacingx, spacingy,
								   marginx, marginy,
								   displaywidth, displayheight);
	binder.pushInstance("TileMap", tilemap);
		
	return 1;
}

int TileMapBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	TileMap* tilemap = static_cast<TileMap*>(ptr);
	tilemap->unref();
	
	return 0;
}


int TileMapBinder::getTile(lua_State* L)
{
    StackChecker checker(L, "TileMapBinder::getTile", 3);
	
	Binder binder(L);
	TileMap* tilemap = static_cast<TileMap*>(binder.getInstance("TileMap", 1));	 

	
	int x = luaL_checkinteger(L, 2) - 1;
	int y = luaL_checkinteger(L, 3) - 1;
    int tx, ty, flip;

	GStatus status;
    tilemap->get(x, y, &tx, &ty, &flip, &status);
	if (status.error() == true)
	{
		luaL_error(L, status.errorString());
		return 0;
	}

	if (TileMap::isEmpty(tx, ty))
	{
		lua_pushnil(L);
		lua_pushnil(L);
        lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, tx + 1);
        lua_pushinteger(L, ty + 1);
        lua_pushinteger(L, flip);
    }
	
    return 3;
}

int TileMapBinder::setTile(lua_State* L)
{
	StackChecker checker(L, "TileMapBinder::setTile", 0);
	
	Binder binder(L);
	TileMap* tilemap = static_cast<TileMap*>(binder.getInstance("TileMap", 1));	
	
	int x = luaL_checkinteger(L, 2) - 1;
	int y = luaL_checkinteger(L, 3) - 1;
	int tx = luaL_checkinteger(L, 4) - 1;
	int ty = luaL_checkinteger(L, 5) - 1;
    int flip = luaL_optinteger(L, 6, 0);


	GStatus status;
    tilemap->set(x, y, tx, ty, flip, &status);
	if (status.error() == true)
	{
		luaL_error(L, status.errorString());
		return 0;
	}	
	
	return 0;
}


int TileMapBinder::shift(lua_State* L)
{
	StackChecker checker(L, "TileMapBinder::shift", 0);
	
	Binder binder(L);
	TileMap* tilemap = static_cast<TileMap*>(binder.getInstance("TileMap", 1));	

	int dx = luaL_checkinteger(L, 2);
	int dy = luaL_checkinteger(L, 3);

	tilemap->shift(dx, dy);

	return 0;
}

int TileMapBinder::clearTile(lua_State* L)
{
	StackChecker checker(L, "TileMapBinder::clearTile", 0);

	Binder binder(L);
	TileMap* tilemap = static_cast<TileMap*>(binder.getInstance("TileMap", 1));

	int x = luaL_checkinteger(L, 2) - 1;
	int y = luaL_checkinteger(L, 3) - 1;

	GStatus status;
    tilemap->set(x, y, TileMap::EMPTY_TILE, TileMap::EMPTY_TILE, 0, &status);
	if (status.error() == true)
	{
		luaL_error(L, status.errorString());
		return 0;
	}

	return 0;
}
