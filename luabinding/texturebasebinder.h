#ifndef TEXTUREBASEBINDER_H
#define TEXTUREBASEBINDER_H

#include "binder.h"

class TextureBaseBinder
{
public:
	TextureBaseBinder(lua_State* L);
	
private:
	static int getWidth(lua_State* L);
	static int getHeight(lua_State* L);
	static int getSize(lua_State* L);
	static int getTexelSize(lua_State* L);
	static int update(lua_State* L);
};


#endif
