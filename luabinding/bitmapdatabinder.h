#ifndef BITMAPDATABINDER_H
#define BITMAPDATABINDER_H

#include "binder.h"

class BitmapDataBinder
{
public:
	BitmapDataBinder(lua_State* L);
	
private:
	static int create(lua_State* L);
	static int destruct(void *p);

    static int setRegion(lua_State *L);
    static int getRegion(lua_State *L);
    static int getScale(lua_State *L);
};

#endif
