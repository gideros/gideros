#ifndef SHAPEBINDER_H
#define SHAPEBINDER_H

#include "binder.h"

class ShapeBinder
{
public:
	ShapeBinder(lua_State* L);

private:
	static int create(lua_State* L);
	static int destruct(void *p);

	static int setLineStyle(lua_State* L);
	static int setFillStyle(lua_State* L);
	static int beginPath(lua_State* L);
	static int moveTo(lua_State* L);
	static int lineTo(lua_State* L);
	static int endPath(lua_State* L);
	static int closePath(lua_State* L);

    static int tesselate(lua_State* L);
    static int clear(lua_State* L);
};

#endif
