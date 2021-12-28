#ifndef BITMAPBINDER_H
#define BITMAPBINDER_H

#include "binder.h"

class BitmapBinder
{
public:
	BitmapBinder(lua_State* L);
	
private:
	static int create(lua_State* L);
	static int destruct(void *p);

	static int setAnchorPoint(lua_State* L);
	static int getAnchorPoint(lua_State* L);
    static int setTexture(lua_State *L);
    static int setTextureRegion(lua_State *L);
};

#endif
