#include "StateToState.h"


int StateToState::bufferWriter(lua_State* L, const void* p, size_t size, void* ud)
{
    (void)L; // suppress annoying unused variable warning
    luaL_addlstring(static_cast<luaL_Buffer*>(ud), static_cast<const char*>(p), size, -1);
	return 0;
}

void StateToState::copyFunction(lua_State* source, lua_State* dest, int stack_pos)
{
    // get binary blob of function at top of stack...
    luaL_Buffer buffer;
    luaL_buffinit(source, &buffer);
    // ... and dump it on source stack via our buffer
    lua_dump(source, StateToState::bufferWriter, &buffer);
    luaL_pushresult(&buffer);

    // serialize blob to transferrable string
    size_t size;
    char const* serial_string = lua_tolstring(source, stack_pos, &size);

    // ensure enough space in stack in destination state
    lua_checkstack(dest, 2);
    // attempt to compile the string onto destination stack
    char const* name = nullptr;
    if (luaL_loadbuffer(dest, serial_string, size, name) != 0)
    {
        // handle error compiling function
        lua_checkstack(source, 1);
        luaL_error(source, "Transfer Error: cannot compile function: %s", lua_tolstring(dest, -1, nullptr));
    }
    // move the serialized string to top of stack...
    lua_replace(source, -2);
    // ... and pop it off
    lua_pop(source, 1);
}

void StateToState::copyTable(lua_State* source, lua_State* dest, int stack_pos)
{
    lua_newtable(dest);
    lua_pushvalue(source, stack_pos); // copy table ref to top of stack
    lua_pushnil(source);
    while (lua_next(source, -2))
    {
        // stack now contains: -1 => value; -2 => key; -3 => table
        // copy the key so that lua_tostring does not modify the original
        lua_pushvalue(source, -2);
        // stack now contains: -1 => key; -2 => value; -3 => key; -4 => table

        StateToState::copyStackEntry(source, dest, -2);

        // TODO: keys of different types than number or string
        if (lua_isnumber(source, -3)) {
            lua_rawseti(dest, -2, static_cast<int>(lua_tonumber(source, -3)));
        } else {
            lua_setfield(dest, -2, lua_tolstring(source, -3, nullptr));
        }
        lua_pop(source, 2);
        // stack now contains: -1 => key; -2 => table
    }
    // stack now contains: -1 => table (when lua_next returns 0 it pops the key
    // but does not push anything.)
    // Pop table
    lua_pop(source, 1);
    // Stack is now the same as it was on entry to this function
}

void StateToState::copyStackEntry(lua_State* source, lua_State* dest, int stack_pos)
{
    lua_checkstack(dest, 1);
    int type = lua_type(source, stack_pos);
    switch (type)
    {
        case LUA_TNUMBER:
            lua_pushnumber(dest, lua_tonumber(source, stack_pos));
            break;
        case LUA_TSTRING:
            lua_pushstring(dest, lua_tolstring(source, stack_pos, nullptr));
            break;
        case LUA_TBOOLEAN:
            lua_pushboolean(dest, lua_toboolean(source, stack_pos));
            break;
        case LUA_TTABLE:
            StateToState::copyTable(source, dest, stack_pos);
            break;
        case LUA_TFUNCTION:
            StateToState::copyFunction(source, dest, stack_pos);
            break;
        case LUA_TUSERDATA:
            lua_pushlightuserdata(dest, lua_touserdata(source, stack_pos));
            break;
        case LUA_TNIL:
            lua_pushnil(dest);
            break;
        default:
            lua_checkstack(source, 1);
            luaL_error(source, "Transfer Error: Unknown type");
    }
}

void StateToState::copyStackSlice(lua_State* source, lua_State* dest, int lo_stack_pos, int hi_stack_pos) {
    for (int i = lo_stack_pos; i <= hi_stack_pos; ++i)
        StateToState::copyStackEntry(source, dest, i);
}

