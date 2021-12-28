#ifndef TEXTUREBINDER_H
#define TEXTUREBINDER_H

#include "binder.h"

class TextureBinder
{
public:
	TextureBinder(lua_State* L);
	
private:
	static int create(lua_State* L);
    static int loadAsync(lua_State *L);
	static int destruct(void *p);
};

#endif
