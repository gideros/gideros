#ifndef PIXELBINDER_H
#define PIXELBINDER_H

#include "binder.h"

class PixelBinder
{
public:
	PixelBinder(lua_State* L);
	
private:
	static int create(lua_State* L);
	static int destruct(lua_State* L);

	static int setWidth(lua_State* L);
	static int setHeight(lua_State* L);
    static int setDimensions(lua_State *L);
    static int getDimensions(lua_State *L);
    static int setColor(lua_State *L);
    static int getColor(lua_State *L);
    static int setTexture(lua_State *L);
    static int setTextureMatrix(lua_State *L);
    static int setTexturePosition(lua_State *L);
    static int getTexturePosition(lua_State *L);
    static int setTextureScale(lua_State *L);
    static int getTextureScale(lua_State *L);
};

#endif
