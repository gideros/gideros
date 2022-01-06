#ifndef DIBBINDER_H
#define DIBBINDER_H

#include "binder.h"

class DibBinder
{
public:
	DibBinder(lua_State* L);

private:
	static int create(lua_State* L);
	static int destruct(void *p);

	static int getPixel(lua_State* L);
//	static int setPixel(lua_State* L);
};

#endif