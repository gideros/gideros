#include "gplugin.h"
#include "refptr.h"
#include "eventdispatcher.h"
#include "gproxy.h"
#include "pystring.h"
#include <luautil.h>

bool disableTypeChecking_ = false;
static size_t g_userdataToken=(size_t)-1;
static size_t g_gcToken=(size_t)-1;
static size_t g_inewToken=(size_t)-1;
static size_t g_postInitToken=(size_t)-1;

void g_initializeBinderState(lua_State *L) {
    G_UNUSED(L);
    g_userdataToken=(size_t)-1;
    g_gcToken=(size_t)-1;
    g_inewToken=(size_t)-1;
    g_postInitToken=(size_t)-1;
}

void g_disableTypeChecking()
{
	disableTypeChecking_ = true;
}

void g_enableTypeChecking()
{
	disableTypeChecking_ = false;
}

int g_isTypeCheckingEnabled()
{
	return disableTypeChecking_ ? 0 : 1;
}

static int constructor_postInit(lua_State *L)
{
    if (g_inewToken==((size_t)-1)) {
        g_inewToken=lua_newtoken(L,"__new");
        g_postInitToken=lua_newtoken(L,"postInit");
    }
    int n = lua_gettop(L);

    lua_gettoken(L, lua_upvalueindex(1), g_inewToken);
    for (int i = 1; i <= n; ++i)
        lua_pushvalue(L, i);
    lua_call(L, n, 1);

    if (lua_isnil(L, -1))
        return 1;

    if (lua_gettoken(L, -1, g_postInitToken) == LUA_TFUNCTION)
    {
        lua_pushvalue(L, -2);
        for (int i = 1; i <= n; ++i)
            lua_pushvalue(L, i);
        lua_call(L, n + 1, 0);
    }
    else
    {
        lua_pop(L, 1);
    }

    return 1;
}

void g_createClass(lua_State* L,
					 const char* classname,
					 const char* basename,
					 int (*constructor) (lua_State*),
					 int (*destructor) (void*),
					 const luaL_reg* functionlist)
{
	luaL_newmetatable(L, classname); // registry[classname] = {} and new empty metatable is now at top

	lua_pushvalue(L, -1);			// duplicate metatable
	lua_setfield(L, -2, "__index"); // mt.__index = mt

	if (destructor)
	{
#ifdef LUA_IS_LUAU
        void** userdata = (void**)lua_newuserdata(L,sizeof(void*)); // create userdata and push it onto the stack
        *userdata = (void *)destructor;											// store adress in userdata
#else
		lua_pushcnfunction(L, destructor,"destructor");
#endif
        lua_setfield(L, -2, "__gc"); // mt.__gc = destructor
	}

	luaL_register(L, NULL, functionlist); // simply register all functions into the metatable (which is on top)

    if (basename)
        luaL_getmetatable(L, basename);
    else
        luaL_getmetatable(L, "Object");
    lua_setmetatable(L, -2); // mt.mt = basemt

    if (constructor)
    {
        lua_pushcnfunction(L, constructor, classname);
        lua_setfield(L, -2, "__new");

        lua_pushvalue(L, -1);
        lua_pushcnclosure(L, constructor_postInit, 1, "constructor_postInit");
        lua_setfield(L, -2, "new");
    }

    lua_pushstring(L, classname);
    lua_setfield(L, -2, "__classname");
    lua_pushstring(L, basename);
    lua_setfield(L, -2, "__basename");

	// set metatable as global variable
	std::vector<std::string> result;
	pystring::split(classname, result, ".");

	lua_pushvalue(L, LUA_GLOBALSINDEX); // start with the table of globals
	for (size_t i = 0; i < result.size(); ++i)
	{
		const char* w = result[i].c_str();
		if (i + 1 != result.size()) // not last field
		{
			lua_getfield(L, -1, w);
			if (lua_isnil(L, -1)) // create table if absent
			{
				lua_pop(L, 1);
				lua_newtable(L);
				lua_pushvalue(L, -1);
				lua_setfield(L, -3, w);
			}
			lua_remove(L, -2);
		}
		else	// last field
		{
			lua_pushvalue(L, -2);
			lua_setfield(L, -2, w);
			lua_pop(L, 2);
		}
	}
}

void g_makeInstance(lua_State* L, const char* classname, void* ptr)
{
    if (g_userdataToken==((size_t)-1))
        g_userdataToken=lua_newtoken(L,"__userdata");
    if (!classname) {
        //Figure out class meta from current instance
        lua_gettoken(L,-1, g_userdataToken);
        lua_getmetatable(L,-1); //This should have returned 1, otherwise we're not dealing with an instance at all
        lua_remove(L,-2);
    }
    else
        luaL_getmetatable(L, classname); // get metatable
    lua_pushvalue(L,-1);
#ifdef LUA_IS_LUAU
    if (g_gcToken==((size_t)-1))
        g_gcToken=lua_newtoken(L,"__gc");
    lua_rawgettoken(L, -1, g_gcToken); // mt.__gc = destructor
    void **destructor=(void **)lua_touserdata(L,-1);
    lua_pop(L,1);
    lua_setmetatable(L, -3);		 // set metatable for table and pop metatable
    void** userdata;
    if (destructor)
        userdata= (void**)lua_newuserdatadtor(L,sizeof(void*),(void (*)(void *))(*destructor)); // create userdata and push it onto the stack
    else
        userdata= (void**)lua_newuserdata(L,sizeof(void*)); // create userdata and push it onto the stack
#else

    lua_setmetatable(L, -3);		 // set metatable for table and pop metatable

	void** userdata = (void**)lua_newuserdata(L,sizeof(void*)); // create userdata and push it onto the stack
#endif
	*userdata = ptr;											// store adress in userdata

    lua_insert(L,-2);
	lua_setmetatable(L, -2);          // set metatable for userdata and pop metatable

    lua_rawsettoken(L, -2, g_userdataToken);   // table.__userdata = userdata

	// stack top is the new instance (table)
}

void g_pushInstance(lua_State* L, const char* classname, void* ptr)
{
    lua_newtable(L);                 // create table to be the object
    g_makeInstance(L,classname,ptr);
}

/* convert a stack index to positive */
#define abs_index(L, i)		((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : \
							lua_gettop(L) + (i) + 1)

int g_isInstanceOf(lua_State* L, const char* classname, int index)
{
	index = abs_index(L, index);

	if (lua_istable(L, index) == 0)	// check if the index is table
		return 0;

	lua_getfield(L, LUA_REGISTRYINDEX, classname); // get metatable of given type
	if (lua_getmetatable(L, index) == 0)		// get metatable of given table. if table doesn't have metatable return false
	{
		lua_pop(L, 1);		// pop metatable of given type
		return 0;
	}

	while (true)
	{
		if (lua_rawequal(L, -1, -2) != 0)
		{
			lua_pop(L, 2);                      // pop both same metatables
			return 1;
		}

		if (lua_getmetatable(L, -1) == 0) //  if the table does not have a metatable
		{
			lua_pop(L, 2);
			break;
		}

		lua_replace(L, -2);
	}

	return 0;
}

/*
  check if the bottom of stack is table
  grab the table on bottom of stack
  make typecheck
  return *table.__userdata
  (optional and not necessary) leave stack as it is
*/
void* g_getInstance(lua_State* L, const char* classname, int index)
{
    index = abs_index(L, index);
    if (g_userdataToken==((size_t)-1))
        g_userdataToken=lua_newtoken(L,"__userdata");

    if (disableTypeChecking_ == true)
    {
        if (!lua_istable(L, index))	// check if the bottom of stack (first paramater, i.e. self) is table
        {
			luaL_typerror(L, index, classname);
			return NULL;
		}

        if (lua_gettoken(L, index, g_userdataToken)!=LUA_TUSERDATA) // get adress
		{
			lua_pop(L, 1);
			luaL_error(L, "index '__userdata' cannot be found");
			return NULL;
		}
		void* ptr = *(void**)lua_touserdata(L, -1);
		lua_pop(L, 1);

		return ptr;
	}
	else
	{
		if (lua_istable(L, index) == 0)	// check if the bottom of stack (first paramater, i.e. self) is table
		{
			luaL_typerror(L, index, classname);
			return NULL;
		}

        lua_rawgetfield(L, LUA_REGISTRYINDEX, classname); // get metatable of given type
		if (lua_getmetatable(L, index) == 0)		// get metatable of given table. if table doesn't have metatable, give type error
		{
			lua_pop(L, 1);		// pop metatable of given type
			luaL_typerror(L, index, classname);
			return NULL;
		}

		while (true)
		{
			if (lua_rawequal(L, -1, -2) != 0)
			{
				lua_pop(L, 2);                      // pop both same metatables

                if (lua_gettoken(L, index, g_userdataToken)!=LUA_TUSERDATA) // get adress
				{
					lua_pop(L, 1);
					luaL_error(L, "index '__userdata' cannot be found");
					return NULL;
				}
				void* ptr = *(void**)lua_touserdata(L, -1);
				lua_pop(L, 1);

				return ptr;
			}

			if (lua_getmetatable(L, -1) == 0) //  if the table does not have a metatable
			{
				lua_pop(L, 2);
				break;
			}

			lua_replace(L, -2);
		}

		luaL_typerror(L, index, classname);
		return NULL;
	}

	return NULL;
}

void* g_getInstanceOfType(lua_State* L, const char* classname, int index, int typeMap)
{
    index = abs_index(L, index);
    if (g_userdataToken==((size_t)-1))
        g_userdataToken=lua_newtoken(L,"__userdata");

    if (!lua_istable(L, index))	// check if the bottom of stack (first paramater, i.e. self) is table
    {
        luaL_typerror(L, index, classname);
        return NULL;
    }

    if (lua_gettoken(L, index, g_userdataToken)!=LUA_TUSERDATA) // get address
    {
        lua_pop(L, 1);
        luaL_error(L, "index '__userdata' cannot be found");
        return NULL;
    }
    GReferenced* ptr = *(GReferenced**)lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (ptr->isOfType(typeMap))
        return ptr;

    return g_getInstance(L,classname,index);
}

void g_setInstance(lua_State* L, int index, void* ptr)
{
    if (g_userdataToken==((size_t)-1))
        g_userdataToken=lua_newtoken(L,"__userdata");
    if (lua_gettoken(L, index, g_userdataToken)!=LUA_TUSERDATA) // get adress
    {
        lua_pop(L, 1);
		luaL_error(L, "index '__userdata' cannot be found");
	}
	*(void**)lua_touserdata(L, -1) = ptr;
	lua_pop(L, 1);
}

int g_error(lua_State* L, const char* msg)
{
    LuaApplicationBase* application = static_cast<LuaApplicationBase*>(luaL_getdata(L));

    application->setError(msg);
#if 0
	lua_PrintFunc printfunc = lua_getprintfunc(L);
	void* data = lua_getprintfuncdata(L);
	printfunc(msg, data);

	application->deinitialize();
	application->initialize();
#endif
	return 0;
}

class Object : public GReferenced
{

};

GProxy::GProxy(GType type)
{
	object_ = NULL;

	if (type == eBase)
	{
		object_ = new Object;
		object_->setProxy(this);
	}
}

GProxy::~GProxy()
{
	if (object_)
		object_->unref();
}

GEventDispatcherProxy::GEventDispatcherProxy(GType type) : GProxy(type)
{
	if (type == eEventDispatcher)
	{
		object_ = new EventDispatcher;
		object_->setProxy(this);
	}
}

GEventDispatcherProxy::~GEventDispatcherProxy()
{
}
