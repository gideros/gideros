#include "native.h"
#include <map>
#include <string>
#include "pystring.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <stdlib.h>
#include <memory.h>
#include <string.h>

static char keyClasses = ' ';
static char keyObject = ' ';
static char keyObjects = ' ';
static char keyMetatableWithGC = ' ';

static lua_State *L = NULL;

static void luaL_newweaktable(lua_State *L, const char *mode)
{
    lua_newtable(L);
    lua_pushstring(L, mode);
    lua_setfield(L, -2, "__mode");
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -2);
}

static int getScore(lua_State *L, gnative_Function *function)
{
	int parameterCount1 = lua_gettop(L);
	
	int parameterCount2 = gnative_FunctionGetParameterCount(function);
	if (!gnative_FunctionIsStatic(function))
		parameterCount2++;
	
	if (parameterCount1 != parameterCount2)
		return 100;

	return 1;
}

static gnative_Function *match(lua_State *L, gnative_Function **functions)
{
	if (functions == NULL)
		return NULL;
	
	gnative_Function *result = NULL;
	int minScore = 100;
	for (;*functions;++functions)
	{
		gnative_Function *function = *functions;
		int score = getScore(L, function);
		if (score < minScore)
		{
			result = function;
			minScore = score;
		}
	}
	
	return result;
}

static gnative_Value booleanToValue(int z, gnative_Type type)
{
	gnative_Value v;
	
	switch (type)
	{
	case GNATIVE_TYPE_BOOLEAN:
		v.z = z;
		break;
	case GNATIVE_TYPE_CHAR:
		v.c = z;
		break;
	case GNATIVE_TYPE_SHORT:
		v.s = z;
		break;
	case GNATIVE_TYPE_INT:
		v.i = z;
		break;
	case GNATIVE_TYPE_LONG:
		v.l = z;
		break;
	default:
		memset(&v, sizeof(v), 0);
		break;
	}

	return v;
}

static gnative_Value numberToValue(lua_Number n, gnative_Type type)
{
	gnative_Value v;
	
	switch (type)
	{
	case GNATIVE_TYPE_BOOLEAN:
		v.z = 1;
		break;
	case GNATIVE_TYPE_CHAR:
		v.c = n;
		break;
	case GNATIVE_TYPE_SHORT:
		v.s = n;
		break;
	case GNATIVE_TYPE_INT:
		v.i = n;
		break;
	case GNATIVE_TYPE_LONG:
		v.l = n;
		break;
	case GNATIVE_TYPE_FLOAT:
		v.f = n;
		break;
	case GNATIVE_TYPE_DOUBLE:
		v.d = n;
		break;
	case GNATIVE_TYPE_LONG_DOUBLE:
		v.ld = n;
		break;
	case GNATIVE_TYPE_JAVA_CHAR:
		v.jc = n;
		break;
	default:
		memset(&v, sizeof(v), 0);
		break;
	}

	return v;
}


static gnative_Value stringToValue(const char *str, gnative_Type type)
{
	gnative_Value v;
	
	switch (type)
	{
	case GNATIVE_TYPE_STRING:
		v.str = str;
		break;
	case GNATIVE_TYPE_COMPLEX:
		v.ptr = (void*)str;
		break;
	default:
		memset(&v, sizeof(v), 0);
		break;
	}
	
	return v;
}


static int getClass(lua_State *L);

static int pushObject(lua_State *L, gnative_Object *obj)
{
	if (obj == NULL)
		return 0;
	
	lua_pushlightuserdata(L, &keyObjects);
	lua_gettable(L, LUA_REGISTRYINDEX);

	lua_pushlightuserdata(L, obj);
	lua_gettable(L, -2);
	
	if (!lua_isnil(L, -1))
	{
		gnative_ObjectRelease(obj);
		lua_remove(L, -2);
		return 1;
	}
	
	lua_pop(L, 2);
	
	lua_newtable(L);

	lua_pushcfunction(L, getClass);
	lua_pushstring(L, gnative_ClassGetName(gnative_ObjectGetClass(obj)));
	lua_call(L, 1, 1);
	lua_setmetatable(L, -2);

	void **userdata = (void**)lua_newuserdata(L, sizeof(void*));
	*userdata = obj;
	lua_pushlightuserdata(L, &keyMetatableWithGC);
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "__userdata");

	lua_pushlightuserdata(L, &keyObjects);
	lua_gettable(L, LUA_REGISTRYINDEX);
	
	lua_pushlightuserdata(L, obj);
	lua_pushvalue(L, -3);
	lua_settable(L, -3);
	lua_pop(L, 1);

	return 1;
}

static int pushValue(lua_State *L, gnative_Value v, gnative_Type t, bool del)
{
	switch (t)
	{
	case GNATIVE_TYPE_VOID:
		return 0;
	case GNATIVE_TYPE_BOOLEAN:
		lua_pushboolean(L, v.z);
		return 1;
	case GNATIVE_TYPE_CHAR:
		lua_pushinteger(L, v.c);
		return 1;
	case GNATIVE_TYPE_SHORT:
		lua_pushinteger(L, v.s);
		return 1;
	case GNATIVE_TYPE_INT:
		lua_pushinteger(L, v.i);
		return 1;
	case GNATIVE_TYPE_LONG:
		lua_pushnumber(L, v.l);
		return 1;
	case GNATIVE_TYPE_FLOAT:
		lua_pushnumber(L, v.f);
		return 1;
	case GNATIVE_TYPE_DOUBLE:
		lua_pushnumber(L, v.d);
		return 1;
	case GNATIVE_TYPE_LONG_DOUBLE:
		lua_pushnumber(L, v.ld);
		return 1;
	case GNATIVE_TYPE_STRING:
		lua_pushstring(L, v.str);
		if (del)
			free((char*)v.str);
		return 1;
	case GNATIVE_TYPE_OBJECT:
		if (!del)
			gnative_ObjectRetain(v.obj);
		return pushObject(L, v.obj);
	case GNATIVE_TYPE_COMPLEX:
		return 0;
	case GNATIVE_TYPE_JAVA_CHAR:
		lua_pushinteger(L, v.jc);
		return 1;
	}
	
	return 0;
}

gnative_Object *toObject(lua_State *L, int index)
{
	lua_pushstring(L, "__userdata");
	lua_rawget(L, index);
	void* ptr = *(void**)lua_touserdata(L, -1);
	gnative_Object *obj = (gnative_Object*)ptr;
	lua_pop(L, 1);
	
	return obj;
}

static int function(lua_State *L)
{
	gnative_Class *cls = (gnative_Class*)lua_touserdata(L, lua_upvalueindex(1));
	const char *name = lua_tostring(L, lua_upvalueindex(2));

	gnative_Function **functions = gnative_ClassGetFunctionsByName(cls, name);
	
	gnative_Function *function = match(L, functions);

	int parameterCount = lua_gettop(L);

	gnative_Type *parameterTypes = gnative_FunctionGetParameterTypes(function);

	gnative_Object *self;
	int delta;
	if (gnative_FunctionIsStatic(function))
	{
		self = NULL;
		delta = 0;
	}
	else
	{
		self = toObject(L, 1);
		delta = 1;
	}

	std::vector<gnative_Value> parameters;
	
	for (int i = delta; i < parameterCount; ++i)
	{
		gnative_Type t = parameterTypes[i - delta];
		gnative_Value v;
		switch (lua_type(L, i + 1))
		{
		case LUA_TBOOLEAN:
			v = booleanToValue(lua_toboolean(L, i + 1), t);
			break;
		case LUA_TLIGHTUSERDATA:
			break;
		case LUA_TNUMBER:
			v = numberToValue(lua_tonumber(L, i + 1), t);
			break;
		case LUA_TSTRING:
			v = stringToValue(lua_tostring(L, i + 1), t);
			break;
		case LUA_TTABLE:
			v.obj = toObject(L, i + 1);
			break;
		case LUA_TNIL:
			v.obj = NULL;
			break;
		}
		parameters.push_back(v);
	}
	
	gnative_Value v = gnative_FunctionCall(function, self, parameters.empty() ? NULL : &parameters[0]);

	gnative_Type t = gnative_FunctionGetReturnType(function);
	
	return pushValue(L, v, t, true);
}


static bool getField(lua_State *L, gnative_Class *cls, const char *name)
{
	gnative_Function **functions;

	functions = gnative_ClassGetFunctionsByName(cls, name);
	if (functions != NULL)
	{
		lua_pushlightuserdata(L, cls);
		lua_pushstring(L, name);
		lua_pushcclosure(L, function, 2);
		return true;
	}
	
	lua_pushnil(L);
	
	return false;
}

static int __index(lua_State *L)
{
	lua_pushstring(L, "__classdata");
	lua_rawget(L, 1);

	if (!lua_isnil(L, -1))
	{
		gnative_Class *cls = (gnative_Class*)lua_touserdata(L, -1);
	
		lua_pop(L, 1);

		const char *name = lua_tostring(L, 2);

		if (name == NULL)
			return 0;
	
		bool cache = getField(L, cls, name);
	
		if (!lua_isnil(L, -1))
		{
			if (cache)
			{
				lua_pushvalue(L, 1);
				lua_pushvalue(L, 2);
				lua_pushvalue(L, -3);
				lua_settable(L, -3);
				lua_pop(L, 1);
			}
		
			return 1;
		}
	
		lua_pop(L, 1);
	}
		
	lua_getmetatable(L, 1);
	
	if (lua_isnil(L, -1))
	{
		lua_pop(L, 1);
		return 0;
	}
	
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);

	lua_remove(L, -2);

	return 1;
}

static int __new(lua_State *L)
{
	lua_pushstring(L, "__classdata");
	lua_rawget(L, lua_upvalueindex(1));
	gnative_Class *cls = (gnative_Class*)lua_touserdata(L, -1);
	lua_pop(L, 1);
	
	gnative_Object *obj = gnative_CreateObject(cls, NULL, NULL);
	
	return pushObject(L, obj);
}

static int __gc(lua_State *L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	gnative_Object *obj = (gnative_Object*)ptr;
	gnative_ObjectRelease(obj);
	return 0;
}

static void s_implementation(gnative_Object *obj, gnative_Function *function, gnative_Value *parameters)
{
	if (L == NULL)
		return;

	gnative_Class *cls = gnative_ObjectGetClass(obj);
	
	lua_pushlightuserdata(L, &keyClasses);
	lua_gettable(L, LUA_REGISTRYINDEX);
	
	lua_pushlightuserdata(L, cls);
	lua_gettable(L, -2);
	
	lua_remove(L, -2);

	lua_pushstring(L, "__functions");
	lua_rawget(L, -2);

	lua_pushlightuserdata(L, function);
	lua_gettable(L, -2);
	
	gnative_Value v;
	v.obj = obj;
	pushValue(L, v, GNATIVE_TYPE_OBJECT, false);

	int parameterCount = gnative_FunctionGetParameterCount(function);
	
	gnative_Type *parameterTypes = gnative_FunctionGetParameterTypes(function);

	for (int i = 0; i < parameterCount; ++i)
		pushValue(L, parameters[i], parameterTypes[i], false);

	lua_call(L, 1 + parameterCount, 0);
	
	lua_pop(L, 2);
}

static int addFunction(lua_State *L)
{
	lua_pushstring(L, "__classdata");
	lua_rawget(L, lua_upvalueindex(1));
	gnative_Class *cls = (gnative_Class*)lua_touserdata(L, -1);
	lua_pop(L, 1);

	const char *name = luaL_checkstring(L, 1);
	const char *types = luaL_checkstring(L, 3);
	
	// TODO: check index 2 is a function

	gnative_Function *function = gnative_ClassAddFunction(cls, name, types, s_implementation);

	lua_pushstring(L, "__functions");
	lua_rawget(L, lua_upvalueindex(1));
	
	if (lua_isnil(L, -1))
	{
		lua_pop(L, 1);
		lua_newtable(L);
		lua_pushvalue(L, -1);
		lua_setfield(L, lua_upvalueindex(1), "__functions");
	}
	
	lua_pushlightuserdata(L, function);
	lua_pushvalue(L, 2);
	lua_settable(L, -3);
	lua_pop(L, 1);

	return 0;
}

static int getClass(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	gnative_Class *cls = gnative_GetClass(name);
	if (cls == NULL)
		return 0;
	
	lua_pushlightuserdata(L, &keyClasses);
	lua_gettable(L, LUA_REGISTRYINDEX);
	
	lua_pushlightuserdata(L, cls);
	lua_gettable(L, -2);
		
	lua_remove(L, -2);
	
	if (!lua_isnil(L, -1))
		return 1;
	
	lua_pop(L, 1);
	
	lua_newtable(L);
	
	lua_pushlightuserdata(L, cls);
	lua_setfield(L, -2, "__classdata");
	
	lua_pushcfunction(L, __index);
	lua_setfield(L, -2, "__index");
	
	lua_pushvalue(L, -1);
	lua_pushcclosure(L, __new, 1);
	lua_setfield(L, -2, "new");

	lua_pushvalue(L, -1);
	lua_pushcclosure(L, addFunction, 1);
	lua_setfield(L, -2, "addFunction");
	
	gnative_Class *superclass = gnative_ClassGetSuperclass(cls);
	
	if (superclass != NULL)
	{
		lua_pushcfunction(L, getClass);
		lua_pushstring(L, gnative_ClassGetName(superclass));
		lua_call(L, 1, 1);
		lua_setmetatable(L, -2);
	}
	else
	{
		lua_pushlightuserdata(L, &keyObject);
		lua_gettable(L, LUA_REGISTRYINDEX);
		lua_setmetatable(L, -2);
	}
	
	lua_pushlightuserdata(L, &keyClasses);
	lua_gettable(L, LUA_REGISTRYINDEX);
	
	lua_pushlightuserdata(L, cls);
	lua_pushvalue(L, -3);
	lua_settable(L, -3);
	lua_pop(L, 1);

	return 1;
}

static int createClass(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	
	// TODO: error checking
	lua_pushstring(L, "__classdata");
	lua_rawget(L, 2);
	gnative_Class *superclass = (gnative_Class*)lua_touserdata(L, -1);
	lua_pop(L, 1);

	gnative_Class *cls = gnative_CreateClass(name, superclass);
	
	if (cls == NULL)
	{
		// TODO: give error
		return 0;
	}
	
	lua_pushcfunction(L, getClass);
	lua_pushstring(L, gnative_ClassGetName(cls));
	lua_call(L, 1, 1);

	return 1;
}


extern "C" {

int loader(lua_State *L)
{
	::L = L;

	lua_pushlightuserdata(L, &keyClasses);
	lua_newtable(L);
	lua_settable(L, LUA_REGISTRYINDEX);
	
	lua_pushlightuserdata(L, &keyObject);
	lua_newtable(L);
	lua_pushcfunction(L, __index);
	lua_setfield(L, -2, "__index");
	lua_settable(L, LUA_REGISTRYINDEX);

	lua_pushlightuserdata(L, &keyObjects);
	luaL_newweaktable(L, "v");
	lua_settable(L, LUA_REGISTRYINDEX);
	
	lua_pushlightuserdata(L, &keyMetatableWithGC);
	lua_newtable(L);
	lua_pushcfunction(L, __gc);
	lua_setfield(L, -2, "__gc");
	lua_settable(L, LUA_REGISTRYINDEX);

	lua_newtable(L);
	lua_pushcfunction(L, getClass);
	lua_setfield(L, -2, "getClass");

	lua_pushcfunction(L, createClass);
	lua_setfield(L, -2, "createClass");
	
	lua_pushvalue(L, -1);
	lua_setglobal(L, "native");

	return 1;
}

}

