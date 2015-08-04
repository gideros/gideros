#ifndef ACCELEROMETERBINDER_H
#define ACCELEROMETERBINDER_H

#include "binder.h"

class AccelerometerBinder
{
public:
	AccelerometerBinder(lua_State* L);

private:
    static int create(lua_State* L);
    static int destruct(lua_State* L);

    static int isAvailable(lua_State* L);
    static int start(lua_State* L);
    static int stop(lua_State* L);
    static int getAcceleration(lua_State* L);
};



#endif
