#include "binder.h"
#include "stackchecker.h"
#include <gideros.h>


void Binder::disableTypeChecking()
{
	g_disableTypeChecking();
}

void Binder::enableTypeChecking()
{
	g_enableTypeChecking();
}

void Binder::createClass(const char* classname,
						 const char* basename,
						 int (*constructor) (lua_State*),
						 int (*destructor) (lua_State*),
						 const luaL_reg* functionlist)
{
	g_createClass(L, classname, basename, constructor, destructor, functionlist);
}

void Binder::pushInstance(const char* classname, void* ptr)
{
	g_pushInstance(L, classname, ptr);
}

bool Binder::isInstanceOf(const char* classname, int index) const
{
	return g_isInstanceOf(L, classname, index) != 0;
}

void* Binder::getInstance(const char* classname, int index) const
{
	return g_getInstance(L, classname, index);
}

void Binder::setInstance(int index, void* ptr)
{
	g_setInstance(L, index, ptr);
}

