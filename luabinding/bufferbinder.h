#ifndef BUFFERBINDER_H
#define BUFFERBINDER_H

#include "binder.h"

class BufferBinder
{
public:
	BufferBinder(lua_State* L);
private:
	static int create(lua_State* L);
	static int destruct(void *p);

    static int append(lua_State *L);
    static int prepend(lua_State *L);
    static int trim(lua_State *L);
    static int set(lua_State *L);
    static int get(lua_State *L);
    static int size(lua_State *L);
    static int seek(lua_State *L);
    static int tell(lua_State *L);
};

#endif
