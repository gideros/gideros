#ifndef STACKCHECKER_H
#define STACKCHECKER_H

//extern "C"
//{
//#include "lua.h"
//#include "lauxlib.h"
//#include "lualib.h"
//}
#include "lua.hpp"

#ifndef G_UNUSED
#define G_UNUSED(x) (void)(x)
#endif

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
		G_UNUSED(L);
		G_UNUSED(pre);
		G_UNUSED(delta);
	}
};

typedef PrintStackChecker StackChecker;

#endif
