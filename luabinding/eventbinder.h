#ifndef EVENTBINDER_H
#define EVENTBINDER_H

#include "binder.h"

class EventBinder
{
public:
	EventBinder(lua_State* L);

private:
	static int create(lua_State* L);

	static int getType(lua_State* L);
	static int getTarget(lua_State* L);
	static int stopPropagation(lua_State* L);
};

#endif
