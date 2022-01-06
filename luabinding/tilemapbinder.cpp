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
		{"setRepeat", TileMapBinder::setRepeat},
		{"setTexture", TileMapBinder::setTexture},
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

int TileMapBinder::destruct(void *p)
{
	void *ptr = GIDEROS_DTOR_UDATA(p);
	TileMap* tilemap = static_cast<TileMap*>(ptr);
	tilemap->unref();
	
	return 0;
}


int TileMapBinder::getTile(lua_State* L)
{
    StackChecker checker(L, "TileMapBinder::getTile", 5);
	
	Binder binder(L);
	TileMap* tilemap = static_cast<TileMap*>(binder.getInstance("TileMap", 1));	 

	
	int x = luaL_checkinteger(L, 2) - 1;
	int y = luaL_checkinteger(L, 3) - 1;
    int flip;
    uint16_t tx,ty;
    uint32_t tint;

	GStatus status;
    tilemap->get(x, y, &tx, &ty, &flip, &tint, &status);
	if (status.error() == true)
	{
		luaL_error(L, "%s", status.errorString());
		return 0;
	}

	if (flip&TileMap::FLIP_EMPTY)
	{
		lua_pushnil(L);
		lua_pushnil(L);
        lua_pushnil(L);
        lua_pushnil(L);
        lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, tx + 1);
        lua_pushinteger(L, ty + 1);
        lua_pushinteger(L, flip);
        lua_pushinteger(L, tint&0xFFFFFF);
        lua_pushnumber(L,(1.0/255)*((tint>>24)&0xFF));
    }
	
    return 5;
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
    unsigned int color = luaL_optinteger(L, 7, 0xFFFFFF);
    lua_Number alpha = luaL_optnumber(L, 8, 1.0);
    uint32_t tint = color|((((int)(255*alpha))&0xFF)<<24);


	GStatus status;
    tilemap->set(x, y, tx, ty, flip&(~TileMap::FLIP_EMPTY), tint, &status);
	if (status.error() == true)
	{
		luaL_error(L, "%s", status.errorString());
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
    tilemap->set(x, y, 0,0,TileMap::FLIP_EMPTY, 0xFFFFFFFF, &status);
	if (status.error() == true)
	{
		luaL_error(L, "%s", status.errorString());
		return 0;
	}

	return 0;
}

int TileMapBinder::setRepeat(lua_State* L)
{
	StackChecker checker(L, "TileMapBinder::setRepeat", 0);

	Binder binder(L);
	TileMap* tilemap = static_cast<TileMap*>(binder.getInstance("TileMap", 1));

	int x = lua_toboolean(L,2);
	int y = lua_isnoneornil(L,3)?x:lua_toboolean(L,3);

	GStatus status;
    tilemap->setRepeat(x,y);

	return 0;
}

int TileMapBinder::setTexture(lua_State* L)
{
	StackChecker checker(L, "TileMapBinder::setTexture", 0);

	Binder binder(L);
	TileMap* tilemap = static_cast<TileMap*>(binder.getInstance("TileMap", 1));

	TextureBase* texturebase = static_cast<TextureBase*>(binder.getInstance("TextureBase", 2));
	int tilewidth = luaL_checkinteger(L, 3);
	int tileheight = luaL_checkinteger(L, 4);
	int spacingx = luaL_optinteger(L, 5, 0);
	int spacingy = luaL_optinteger(L, 6, 0);
	int marginx = luaL_optinteger(L, 7, 0);
	int marginy = luaL_optinteger(L, 8, 0);

	tilemap->setTexture(texturebase,
								   tilewidth, tileheight,
								   spacingx, spacingy,
								   marginx, marginy);

	return 0;
}
