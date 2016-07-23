#ifndef SHADERBINDER_H
#define SHADERBINDER_H

#include "binder.h"

class ShaderBinder
{
public:
	ShaderBinder(lua_State* L);
	
private:
	static int create(lua_State* L);
	static int destruct(lua_State* L);

	static int setConstant(lua_State* L);
	static int isValid(lua_State* L);
	static int getEngineVersion(lua_State* L);
	static int getShaderLanguage(lua_State* L);
};

#endif
