/*

    StateToState.h:

        Copies either individual stack entries or multiple entries (a "stack slice")
        from a source to a destination Lua state.

    Author: Paul Reilly

*/

#pragma once
#include "lua.hpp"
#include <cstdlib>


class StateToState
{
public:
    static void copyStackEntry(lua_State* source, lua_State* dest, int stack_pos);
    static void copyStackSlice(lua_State* source, lua_State* dest, int lo_stack_pos, int hi_stack_pos);

private:
    static void copyFunction(lua_State* source, lua_State* dest, int stack_pos);
    static void copyTable(lua_State* source, lua_State* dest, int stack_pos);

    // for lua_dump
    static int bufferWriter(lua_State* L, const void* p, size_t size, void* ud);
};

