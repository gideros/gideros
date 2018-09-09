#pragma once

#include "lua.hpp"
#include <string>
#include <vector>

class Binder
{
public:
    explicit Binder(lua_State* L) : L(L)
    {
    }

    void createClass(const char* classname,
                     const char* basename,
                     int (*constructor) (lua_State*),
                     int (*destructor) (lua_State*),
                     const luaL_reg* functionlist);


    void pushInstance(const char* classname, void* ptr);

    void* getInstance(const char* classname, int index = 1) const;

    void setInstance(int index, void* ptr);

    bool isInstanceOf(const char* classname, int index) const;

    static void disableTypeChecking();
    static void enableTypeChecking();

    lua_State* L;
};
