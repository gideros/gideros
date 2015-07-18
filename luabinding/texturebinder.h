#ifndef TEXTUREBINDER_H
#define TEXTUREBINDER_H

#include "binder.h"

class TextureBinder
{
public:
	TextureBinder(lua_State* L);
	
private:
	static int create(lua_State* L);
	static int destruct(lua_State* L);
};

#endif
