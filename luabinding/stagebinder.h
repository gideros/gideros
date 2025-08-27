#ifndef STAGEBINDER_H
#define STAGEBINDER_H

#include "binder.h"

class Application;

class StageBinder
{
public:
	StageBinder(lua_State* L, Application* application);

private:
	static int destruct(void *p);

	static int getOrientation(lua_State* L);
	static int setOrientation(lua_State* L);

	static int getClearColorBuffer(lua_State* L);
	static int setClearColorBuffer(lua_State* L);

	static int setBackgroundColor(lua_State* L);
	static int getBackgroundColor(lua_State* L);
    static int validateLayout(lua_State* L);
};

#endif
