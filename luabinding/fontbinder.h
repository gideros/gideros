#ifndef FONTBINDER_H
#define FONTBINDER_H

#include "binder.h"

class FontBinder
{
public:
	FontBinder(lua_State* L);
	
private:
	static int create(lua_State* L);
	static int destruct(lua_State* L);
};

#endif
