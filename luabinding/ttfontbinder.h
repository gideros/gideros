#ifndef TTFONTBINDER_H
#define TTFONTBINDER_H

#include "binder.h"

class TTFontBinder
{
public:
	TTFontBinder(lua_State* L);

private:
	static int create(lua_State* L);
	static int destruct(lua_State* L);
};


#endif
