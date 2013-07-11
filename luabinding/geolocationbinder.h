#ifndef _GEOLOCATIONBINDER_H_
#define _GEOLOCATIONBINDER_H_

#include "binder.h"

class GeolocationBinder
{
public:
    GeolocationBinder(lua_State *L);

private:
    static int create(lua_State *L);
    static int destruct(lua_State *L);

    static int isAvailable(lua_State *L);
    static int isHeadingAvailable(lua_State *L);
    static int setAccuracy(lua_State *L);
    static int getAccuracy(lua_State *L);
    static int setThreshold(lua_State *L);
    static int getThreshold(lua_State *L);
    static int start(lua_State *L);
    static int stop(lua_State *L);
    static int startUpdatingLocation(lua_State *L);
    static int stopUpdatingLocation(lua_State *L);
    static int startUpdatingHeading(lua_State *L);
    static int stopUpdatingHeading(lua_State *L);
};

#endif
