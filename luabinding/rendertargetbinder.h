#pragma once

#include "binder.h"

class RenderTargetBinder
{
public:
    RenderTargetBinder(lua_State *L);

private:
    static int create(lua_State *L);
    static int destruct(lua_State *L);

    static int clear(lua_State *L);
    static int draw(lua_State *L);
    static int getPixel(lua_State *L);
    static int getPixels(lua_State *L);
    static int save(lua_State *L);
};
