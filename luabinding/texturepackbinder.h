#ifndef TEXTUREPACKBINDER_H
#define TEXTUREPACKBINDER_H

#include "binder.h"

class TexturePackBinder
{
public:
	TexturePackBinder(lua_State* L);
	
private:
    static int createCommon(lua_State* L,bool async);
    static int create(lua_State* L);
	static int destruct(lua_State* L);
    static int loadAsync(lua_State* L);
    static int getRegionsNames(lua_State* L);
	static int getLocation(lua_State* L);
};

class TexturePackFontBinder
{
public:
    TexturePackFontBinder(lua_State* L);

private:
    static int create(lua_State* L);
    static int destruct(lua_State* L);
};

#endif
