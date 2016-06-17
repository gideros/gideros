#ifndef URLLOADERBINDER_H
#define URLLOADERBINDER_H

#include "binder.h"

class UrlLoaderBinder
{
public:
    UrlLoaderBinder(lua_State* L);

private:
    static int create(lua_State* L);
    static int destruct(lua_State* L);

    static int load(lua_State* L);
    static int close(lua_State* L);
    static int ignoreSslErrors(lua_State* L);
    static int setProxy(lua_State* L);
};

#endif
