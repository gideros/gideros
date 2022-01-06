#ifndef GYROSCOPEBINDER_H
#define GYROSCOPEBINDER_H

#include "binder.h"

class GyroscopeBinder
{
public:
	GyroscopeBinder(lua_State* L);

private:
    static int create(lua_State* L);
    static int destruct(void *p);

	static int isAvailable(lua_State* L);
	static int start(lua_State* L);
	static int stop(lua_State* L);
	static int getRotationRate(lua_State* L);
};

#endif
