#ifndef PATH2DBINDER_H
#define PATH2DBINDER_H

#include "binder.h"

class Path2DBinder
{
public:
	Path2DBinder(lua_State* L);

private:
	static int create(lua_State* L);
	static int destruct(lua_State* L);

	static int setLineColor(lua_State* L);
	static int setFillColor(lua_State* L);
	static int setPath(lua_State* L);
	static int setSvgPath(lua_State* L);
	static int setFontPath(lua_State* L);
	static int setTexture(lua_State* L);
	static int setLineThickness(lua_State* L);
	static int setConvex(lua_State* L);
};

#endif
