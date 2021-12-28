#pragma once

#include "lua.hpp"
#include "luautil.h"
#include <string>
#include <vector>
#include "gplugin.h"

class Binder
{
public:
    explicit Binder(lua_State* L) : L(L)
    {
    }

    void createClass(const char* classname,
                     const char* basename,
                     int (*constructor) (lua_State*),
                     int (*destructor) (void*),
                     const luaL_reg* functionlist);

    // eg: createClass("Blah", "", nullptr, nullptr, {{ "bling", bling }, { nullptr, nullptr }});
    // avoids creating a separate luaL_reg array (always end vector with {nullptr, nullptr})
    void createClass(std::string classname,
                     std::string basename,
                     int (*constructor) (lua_State*),
                     int (*destructor) (void*),
                     std::vector<luaL_Reg> functionlist);

    void pushInstance(const char* classname, void* ptr);

    void* getInstance(const char* classname, int index = 1) const;

    void setInstance(int index, void* ptr);

    bool isInstanceOf(const char* classname, int index) const;

    static void disableTypeChecking();
    static void enableTypeChecking();

    lua_State* L;
};
