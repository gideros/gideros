#ifndef TEXTUREPACKBINDER_H
#define TEXTUREPACKBINDER_H

#include "binder.h"

class TexturePackBinder
{
public:
	TexturePackBinder(lua_State* L);
	
private:
	static int create(lua_State* L);
	static int destruct(lua_State* L);

	static int getLocation(lua_State* L);
};

#endif
