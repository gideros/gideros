#ifndef VIEWPORTBINDER_H
#define VIEWPORTBINDER_H

#include "binder.h"

class ViewportBinder
{
public:
	ViewportBinder(lua_State* L);

private:
	static int create(lua_State* L);
	static int destruct(lua_State* L);

	static int setContent(lua_State* L);
	static int setTransform(lua_State* L);
	static int setProjection(lua_State* L);
	static int getContent(lua_State* L);
	static int getTransform(lua_State* L);
	static int getProjection(lua_State* L);
	static int lookAt(lua_State* L);
	static int lookAngles(lua_State* L);
};

#endif
