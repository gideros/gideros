#ifndef STACKCHECKER_H
#define STACKCHECKER_H

//extern "C"
//{
//#include "lua.h"
//#include "lauxlib.h"
//#include "lualib.h"
//}
#include "lua.hpp"

class PrintStackChecker
{
public:
	PrintStackChecker(lua_State* L, const char* pre = "", int delta = 0);
	~PrintStackChecker();
	
private:
	lua_State* L;
	int begin_;
	int delta_;
	const char* pre_;
};

class EmptyStackChecker
{
public:
	EmptyStackChecker(lua_State* L, const char* pre = "", int delta = 0)
	{
	}
};

typedef PrintStackChecker StackChecker;

#endif
