#pragma once

#include "binder.h"

class ScreenBinder
{
public:
    ScreenBinder(lua_State *L);

private:
    static int create(lua_State *L);
    static int destruct(void *p);

    static int clear(lua_State *L);
    static int setContent(lua_State *L);
    static int setSize(lua_State *L);
    static int getSize(lua_State *L);
    static int setPosition(lua_State *L);
    static int getPosition(lua_State *L);
    static int setState(lua_State *L);
    static int getState(lua_State *L);
    static int getMaxSize(lua_State *L);
    static int getId(lua_State *L);
};
