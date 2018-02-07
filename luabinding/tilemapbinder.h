#ifndef TILEMAPBINDER_H
#define TILEMAPBINDER_H


#include "binder.h"

class TileMapBinder
{
public:
	TileMapBinder(lua_State* L);
	
private:
	static int create(lua_State* L);
	static int destruct(lua_State* L);
	
	static int getTile(lua_State* L);
	static int setTile(lua_State* L);
	static int clearTile(lua_State* L);

	static int shift(lua_State* L);
	static int setRepeat(lua_State* L);
	static int setTexture(lua_State* L);
};


#endif
