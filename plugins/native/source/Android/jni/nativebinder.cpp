#include "native.h"
#include "gideros.h"
#include <glog.h>
#include <map>
#include <string>
#include "pystring.h"

// some Lua helper functions
#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif
#define NATIVEFUNCCALLED  "__FunctionCalled"
struct TypedValue
{
	int type;
	gnative_Value value;
};

static lua_State *L = NULL;

static void luaL_newweaktable(lua_State *L, const char *mode)
{
	lua_newtable(L);			// create table for instance list
	lua_pushstring(L, mode);
	lua_setfield(L, -2, "__mode");	  // set as weak-value table
	lua_pushvalue(L, -1);             // duplicate table
	lua_setmetatable(L, -2);          // set itself as metatable
}

static void luaL_rawgetptr(lua_State *L, int idx, void *ptr)
{
	idx = abs_index(L, idx);
	lua_pushlightuserdata(L, ptr);
	lua_rawget(L, idx);
}

static void luaL_rawsetptr(lua_State *L, int idx, void *ptr)
{
	idx = abs_index(L, idx);
	lua_pushlightuserdata(L, ptr);
	lua_insert(L, -2);
	lua_rawset(L, idx);
}

static int lua_print(lua_State* L, const char* str)
{
	lua_getglobal(L, "_G");
	lua_getfield(L, -1, "print");
	lua_pushstring(L, str);
	lua_call(L, 1, 0);
	lua_pop(L, 1);
}

static int cindex(lua_State *L);
static int cnewindex(lua_State *L);

void g_createMetaClass(lua_State* L,
                                         const char* classname,
                                         const char* basename,
                                         int (*constructor) (lua_State*),
                                         int (*destructor) (lua_State*),
                                         const luaL_reg* functionlist)
{
        luaL_newmetatable(L, classname); // registry[classname] = {} and new empty metatable is now at top

        lua_pushcfunction(L, &cindex);                   // duplicate metatable
        lua_setfield(L, -2, "__index"); // mt.__index = mt
		
		lua_pushcfunction(L, &cnewindex);
        lua_setfield(L, -2, "__newindex");

        if (destructor)
        {
                lua_pushcfunction(L, destructor);
                lua_setfield(L, -2, "__gc"); // mt.__gc = destructor
        }

        //luaL_register(L, NULL, functionlist); // simply register all functions into the metatable (which is on top)

    if (basename)
        luaL_getmetatable(L, basename);
    else
        luaL_getmetatable(L, "Object");
    lua_setmetatable(L, -2); // mt.mt = basemt

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
                else    // last field
                {
                        lua_pushvalue(L, -2);
                        lua_setfield(L, -2, w);
                        lua_pop(L, 2);
                }
        }
}

static char keyWeak = ' ';

class NativeInstance : public GProxy
{
public:
    NativeInstance(gnative_Object *ob)
    {
		ob_ = ob;
    }
    
    ~NativeInstance()
    {		
		gnative_DeleteObject(ob_);
    }

    gnative_Object* getObject()
    {
    	return ob_;
    }
    
	gnative_Function *match(lua_State *L, int index)
	{
		lua_getmetatable( L , 1 );
		lua_pushstring( L , NATIVEFUNCCALLED );
		lua_rawget( L , -2 );
		if ( lua_type( L , -1 ) == LUA_TNIL )
	    {
     		lua_pushstring( L , "Error no method name" );
		    lua_error( L );
	    }
		const char *methodName = lua_tostring( L , -1 );
   		lua_pop( L , 2 );
		
		gnative_Function **functions = gnative_ObjectGetFunctionsByName(ob_, methodName);

		if(functions != NULL)
		{
			int parameterCount = lua_gettop(L) - index;
			gnative_Function *candidate = NULL;
			int minScore = 100;
			for (;*functions;++functions)
			{
				gnative_Function *function = *functions;
				//if parameter count matches
				if (gnative_FunctionGetParameterCount(function) == parameterCount)
				{	
					if(parameterCount == 0)
						return function;
					int score = 0;
					gnative_Type *parameterTypes = gnative_FunctionGetParameterTypes(function);
					gnative_Class **parameterClasses = gnative_FunctionGetParameterClasses(function);
					
					for (int i = 0; i < parameterCount; ++i)
					{
						switch (lua_type(L, i + index + 1))
						{
							case LUA_TBOOLEAN:
								if (parameterTypes[i] != GNATIVE_TYPE_BOOLEAN)
									score += 1000;
								break;
							case LUA_TLIGHTUSERDATA:
								break;
							case LUA_TNUMBER:
								if (parameterTypes[i] != GNATIVE_TYPE_BYTE &&
									parameterTypes[i] != GNATIVE_TYPE_CHAR &&
									parameterTypes[i] != GNATIVE_TYPE_SHORT &&
									parameterTypes[i] != GNATIVE_TYPE_INT &&
									parameterTypes[i] != GNATIVE_TYPE_LONG &&
									parameterTypes[i] != GNATIVE_TYPE_FLOAT &&
									parameterTypes[i] != GNATIVE_TYPE_DOUBLE)
									score += 1000;
								else
									score += (8 - parameterTypes[i]);
								break;
							case LUA_TSTRING:
								if (parameterTypes[i] != GNATIVE_TYPE_STRING)
									score += 1000;
								break;
							case LUA_TTABLE:
							{
								if (parameterTypes[i] != GNATIVE_TYPE_OBJECT)
									score += 1000;
								else
								{
									GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "NativeInstance", i + index + 1));
									NativeInstance *nat = static_cast<NativeInstance*>(object->proxy());
									gnative_Object *obj = nat->getObject();
									if(strcmp(gnative_ObjectGetName(obj), gnative_ClassGetName(parameterClasses[i])) != 0)
									{
										if (gnative_ClassIsInstance(parameterClasses[i], obj) == false)
											score += 1000;
										else
											score += 1;
									}
								}
								break;
							}
							case LUA_TFUNCTION:
								break;
							case LUA_TUSERDATA:
								break;
							case LUA_TNIL:
								if (parameterTypes[i] != GNATIVE_TYPE_OBJECT && parameterTypes[i] != GNATIVE_TYPE_STRING)
									score += 1000;
								break;
						}
						if(score >= minScore)
						{
							break;
						}
					}
					if(score < minScore)
					{
						candidate = function;
						minScore = score;
					}
				}
			}
			return candidate;
		}
		return NULL;
	}
	
	int callMethod(lua_State *L)
	{
		gnative_Function *function = match(L, 1);

		if(function != NULL)
		{
			int parameterCount = lua_gettop(L) - 1;

			gnative_Type *parameterTypes = gnative_FunctionGetParameterTypes(function);

			std::vector<gnative_Value> parameters;

			for (int i = 0; i < parameterCount; ++i)
			{
				gnative_Value v;

				switch (lua_type(L, i + 2))
				{
					case LUA_TBOOLEAN:
						v.z = lua_toboolean(L, i + 2);
						break;
					case LUA_TLIGHTUSERDATA:
						break;
					case LUA_TNUMBER:
						if (parameterTypes[i] == GNATIVE_TYPE_BYTE)
							v.b = lua_tonumber(L, i + 2);
						else if (parameterTypes[i] == GNATIVE_TYPE_CHAR)
							v.c = lua_tonumber(L, i + 2);
						else if (parameterTypes[i] == GNATIVE_TYPE_SHORT)
							v.s = lua_tonumber(L, i + 2);
						else if (parameterTypes[i] == GNATIVE_TYPE_INT)
							v.i = lua_tonumber(L, i + 2);
						else if (parameterTypes[i] == GNATIVE_TYPE_LONG)
							v.l = lua_tonumber(L, i + 2);
						else if (parameterTypes[i] == GNATIVE_TYPE_FLOAT)
							v.f = lua_tonumber(L, i + 2);
						else if (parameterTypes[i] == GNATIVE_TYPE_DOUBLE)
							v.d = lua_tonumber(L, i + 2);
						break;
					case LUA_TSTRING:
						v.str = lua_tostring(L, i + 2);
						break;
					case LUA_TTABLE:
					{
						GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "NativeInstance", i + 2));
						NativeInstance *nat = static_cast<NativeInstance*>(object->proxy());
						v.obj = nat->getObject();
						break;
					}
					case LUA_TFUNCTION:
						break;
					case LUA_TUSERDATA:
						break;
					case LUA_TNIL:
						v.obj = NULL;
						break;
				}

				parameters.push_back(v);
			}
			gnative_Value result = gnative_FunctionCall(function, ob_, parameters.empty() ? NULL : &parameters[0]);
			int nresult;
			switch (gnative_FunctionGetReturnType(function))
			{
				case GNATIVE_TYPE_VOID:
					nresult = 0;
					break;
				case GNATIVE_TYPE_BOOLEAN:
					lua_pushboolean(L, result.z);
					nresult = 1;
					break;
				case GNATIVE_TYPE_BYTE:
					lua_pushnumber(L, result.b);
					nresult = 1;
					break;
				case GNATIVE_TYPE_CHAR:
					lua_pushnumber(L, result.c);
					nresult = 1;
					break;
				case GNATIVE_TYPE_SHORT:
					lua_pushnumber(L, result.s);
					nresult = 1;
					break;
				case GNATIVE_TYPE_INT:
					lua_pushnumber(L, result.i);
					nresult = 1;
					break;
				case GNATIVE_TYPE_LONG:
					lua_pushnumber(L, result.l);
					nresult = 1;
					break;
				case GNATIVE_TYPE_FLOAT:
					lua_pushnumber(L, result.f);
					nresult = 1;
					break;
				case GNATIVE_TYPE_DOUBLE:
					lua_pushnumber(L, result.d);
					nresult = 1;
					break;
				case GNATIVE_TYPE_STRING:
					lua_pushstring(L, result.str);
					free((char*)result.str);
					nresult = 1;
					break;
				case GNATIVE_TYPE_OBJECT:
					nresult = 1;
					if(result.obj == NULL)
					{
						lua_pushnil(L);
					}
					else
					{
						NativeInstance *natIns = new NativeInstance(result.obj);
		    			g_pushInstance(L, "NativeInstance", natIns->object());
    	
						luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
						lua_pushvalue(L, -2);
						luaL_rawsetptr(L, -2, natIns);
						lua_pop(L, 1);
		    		}
					break;
			}

			return nresult;
		}
		//trying out the property
		else 
		{
			lua_pushstring( L , "No Method or Field found for name" );
		    lua_error( L );
		}
		return 0;
	}
private:
    gnative_Object *ob_;
};

class NativeProxy : public GProxy
{
public:
    NativeProxy(const char *className)
    {
		ob_ = gnative_CreateProxy(className, (long)this);
    }
    
    ~NativeProxy()
    {		
		
    }

    void onInvoke(const char *methodName, int length, gnative_Value *values, int types[])
    {
    	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
		luaL_rawgetptr(L, -1, this);
		
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 2);
			return;
		}

		//checking if there is such method
		lua_getfield(L, -1, methodName);
		if (lua_type(L, -1) == LUA_TFUNCTION)
		{
			for(int i = 0; i < length; i++)
			{
				switch(types[i])
				{
					case GNATIVE_TYPE_BOOLEAN:
						lua_pushboolean(L, values->z);
						break;
					case GNATIVE_TYPE_BYTE:
						lua_pushnumber(L, values->b);
						break;
					case GNATIVE_TYPE_CHAR:
						lua_pushnumber(L, values->c);
						break;
					case GNATIVE_TYPE_SHORT:
						lua_pushnumber(L, values->s);
						break;
					case GNATIVE_TYPE_INT:
						lua_pushnumber(L, values->i);
						break;
					case GNATIVE_TYPE_LONG:
						lua_pushnumber(L, values->l);
						break;
					case GNATIVE_TYPE_FLOAT:
						lua_pushnumber(L, values->f);
						break;
					case GNATIVE_TYPE_DOUBLE:
						lua_pushnumber(L, values->d);
						break;
					case GNATIVE_TYPE_STRING:
						lua_pushstring(L, values->str);
						free((char*)values->str);
						break;
					case GNATIVE_TYPE_OBJECT:
						if(values->obj != NULL)
						{
							NativeInstance *natIns = new NativeInstance(values->obj);
			    			g_pushInstance(L, "NativeInstance", natIns->object());
    	
							luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
							lua_pushvalue(L, -2);
							luaL_rawsetptr(L, -2, natIns);
							lua_pop(L, 1);
			    		}
			    		else
			    		{
			    			lua_pushnil(L);
			    		}
						break;
				}
				values++;
			}
			lua_call(L, length, 0);
		}
    }
	
private:
    gnative_Object *ob_;
};


class NativeClass : public GProxy
{
public:
    NativeClass(const char* className)
    {
		cls_ = gnative_ClassCreate(className);
    }
    
    ~NativeClass()
    {
		
    }

    gnative_Class *getClass(){
    	return cls_;
    }

    gnative_Constructor *matchConstructor(lua_State *L, int index)
	{
		
		gnative_Constructor **functions = gnative_ClassGetConstructors(cls_);

		if(functions != NULL)
		{
			int parameterCount = lua_gettop(L) - index;
			gnative_Constructor *candidate = NULL;
			int minScore = 100;
			for (;*functions;++functions)
			{
				gnative_Constructor *function = *functions;
				//if parameter count matches
				if (gnative_ConstructorGetParameterCount(function) == parameterCount)
				{	
					if(parameterCount == 0)
						return function;
					int score = 0;
					gnative_Type *parameterTypes = gnative_ConstructorGetParameterTypes(function);
					gnative_Class **parameterClasses = gnative_ConstructorGetParameterClasses(function);
					
					for (int i = 0; i < parameterCount; ++i)
					{
						switch (lua_type(L, i + index + 1))
						{
							case LUA_TBOOLEAN:
								if (parameterTypes[i] != GNATIVE_TYPE_BOOLEAN)
									score += 1000;
								break;
							case LUA_TLIGHTUSERDATA:
								break;
							case LUA_TNUMBER:
								if (parameterTypes[i] != GNATIVE_TYPE_BYTE &&
									parameterTypes[i] != GNATIVE_TYPE_CHAR &&
									parameterTypes[i] != GNATIVE_TYPE_SHORT &&
									parameterTypes[i] != GNATIVE_TYPE_INT &&
									parameterTypes[i] != GNATIVE_TYPE_LONG &&
									parameterTypes[i] != GNATIVE_TYPE_FLOAT &&
									parameterTypes[i] != GNATIVE_TYPE_DOUBLE)
									score += 1000;
								else
									score += (8 - parameterTypes[i]);
								break;
							case LUA_TSTRING:
								if (parameterTypes[i] != GNATIVE_TYPE_STRING)
									score += 1000;
								break;
							case LUA_TTABLE:
							{
								if (parameterTypes[i] != GNATIVE_TYPE_OBJECT)
									score += 1000;
								else
								{
									GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "NativeInstance", i + index + 1));
									NativeInstance *nat = static_cast<NativeInstance*>(object->proxy());
									gnative_Object *obj = nat->getObject();
									if(strcmp(gnative_ObjectGetName(obj), gnative_ClassGetName(parameterClasses[i])) != 0)
									{
										if (gnative_ClassIsInstance(parameterClasses[i], obj) == false)
											score += 1000;
										else
											score += 1;
									}
								}
								break;
							}
							case LUA_TFUNCTION:
								break;
							case LUA_TUSERDATA:
								break;
							case LUA_TNIL:
								if (parameterTypes[i] != GNATIVE_TYPE_OBJECT && parameterTypes[i] != GNATIVE_TYPE_STRING)
									score += 1000;
								break;
						}
						if(score >= minScore)
						{
							break;
						}
					}
					if(score < minScore)
					{
						candidate = function;
						minScore = score;
					}
				}
			}
			return candidate;
		}
		return NULL;
	}
	
	gnative_Object* createInstance(lua_State *L)
	{
		gnative_Constructor *constructor = matchConstructor(L, 0);
		if(constructor != NULL)
		{
			int parameterCount = lua_gettop(L);

			gnative_Type *parameterTypes = gnative_ConstructorGetParameterTypes(constructor);

			std::vector<gnative_Value> parameters;

			for (int i = 0; i < parameterCount; ++i)
			{
				gnative_Value v;

				switch (lua_type(L, i + 1))
				{
				case LUA_TBOOLEAN:
					v.z = lua_toboolean(L, i + 1);
					break;
				case LUA_TLIGHTUSERDATA:
					break;
				case LUA_TNUMBER:
					if (parameterTypes[i] == GNATIVE_TYPE_BYTE)
						v.b = lua_tonumber(L, i + 1);
					else if (parameterTypes[i] == GNATIVE_TYPE_CHAR)
						v.c = lua_tonumber(L, i + 1);
					else if (parameterTypes[i] == GNATIVE_TYPE_SHORT)
						v.s = lua_tonumber(L, i + 1);
					else if (parameterTypes[i] == GNATIVE_TYPE_INT)
						v.i = lua_tonumber(L, i + 1);
					else if (parameterTypes[i] == GNATIVE_TYPE_LONG)
						v.l = lua_tonumber(L, i + 1);
					else if (parameterTypes[i] == GNATIVE_TYPE_FLOAT)
						v.f = lua_tonumber(L, i + 1);
					else if (parameterTypes[i] == GNATIVE_TYPE_DOUBLE)
						v.d = lua_tonumber(L, i + 1);
					break;
				case LUA_TSTRING:
					v.str = lua_tostring(L, i + 1);
					break;
				case LUA_TTABLE:
				{
					GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "NativeInstance", i + 1));
					NativeInstance *nat = static_cast<NativeInstance*>(object->proxy());
					v.obj = nat->getObject();
					break;
				}
				case LUA_TFUNCTION:
					break;
				case LUA_TUSERDATA:
					break;
				case LUA_TNIL:
					v.obj = NULL;
					break;
				}

				parameters.push_back(v);
			}
			return gnative_CreateObject(constructor, parameters.empty() ? NULL : &parameters[0]);
		}
	}

	gnative_Object* createArray(lua_State *L)
	{
		std::vector<gnative_Object*> elements;
		lua_pushnil(L);  // first key
		int length = 0;
		while (lua_next(L, 1)) {		
			lua_pushvalue(L, lua_upvalueindex(1)); //get object
       		lua_getfield(L, -1, "new"); //get constructor
	        lua_remove(L, -2); //remove object
	        lua_pushvalue(L, -2); //push table value as param
	        lua_call(L, 1, 1);//create instance

			//get instance
			GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "NativeInstance", -1));
			NativeInstance *nat = static_cast<NativeInstance*>(object->proxy());
			gnative_Object *obj = nat->getObject();
			elements.push_back(obj);
	        
			lua_pop(L, 1);//remove object

			// removes 'value'; keeps 'key' for next iteration
			lua_pop(L, 1);
			length++;
		}
		//return NULL;
		return gnative_CreateArray(getClass(), elements.empty() ? NULL : &elements[0], length);
	}

	gnative_Function *match(lua_State *L, int index)
	{
		lua_getmetatable( L , lua_upvalueindex(1) );
		lua_pushstring( L , NATIVEFUNCCALLED );
		lua_rawget( L , -2 );
		if ( lua_type( L , -1 ) == LUA_TNIL )
	    {
     		lua_pushstring( L , "Error no method name" );
		    lua_error( L );
	    }
		const char *methodName = lua_tostring( L , -1 );
   		lua_pop( L , 2 );
		
		gnative_Function **functions = gnative_ClassGetStaticFunctionsByName(cls_, methodName);

		if(functions != NULL)
		{
			int parameterCount = lua_gettop(L) - index;
			gnative_Function *candidate = NULL;
			int minScore = 100;
			for (;*functions;++functions)
			{
				gnative_Function *function = *functions;
				//if parameter count matches
				if (gnative_FunctionGetParameterCount(function) == parameterCount)
				{	
					if(parameterCount == 0)
						return function;

					int score = 0;
					gnative_Type *parameterTypes = gnative_FunctionGetParameterTypes(function);
					gnative_Class **parameterClasses = gnative_FunctionGetParameterClasses(function);
					for (int i = 0; i < parameterCount; ++i)
					{
						switch (lua_type(L, i + index + 1))
						{
							case LUA_TBOOLEAN:
								if (parameterTypes[i] != GNATIVE_TYPE_BOOLEAN)
									score += 1000;
								break;
							case LUA_TLIGHTUSERDATA:
								break;
							case LUA_TNUMBER:
								if (parameterTypes[i] != GNATIVE_TYPE_BYTE &&
									parameterTypes[i] != GNATIVE_TYPE_CHAR &&
									parameterTypes[i] != GNATIVE_TYPE_SHORT &&
									parameterTypes[i] != GNATIVE_TYPE_INT &&
									parameterTypes[i] != GNATIVE_TYPE_LONG &&
									parameterTypes[i] != GNATIVE_TYPE_FLOAT &&
									parameterTypes[i] != GNATIVE_TYPE_DOUBLE)
									score += 1000;
								else
									score += (8 - parameterTypes[i]);
								break;
							case LUA_TSTRING:
								if (parameterTypes[i] != GNATIVE_TYPE_STRING)
									score += 1000;
								break;
							case LUA_TTABLE:
							{
								if (parameterTypes[i] != GNATIVE_TYPE_OBJECT)
									score += 1000;
								else
								{
									GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "NativeInstance", i + index + 1));
									NativeInstance *nat = static_cast<NativeInstance*>(object->proxy());
									gnative_Object *obj = nat->getObject();
									if(strcmp(gnative_ObjectGetName(obj), gnative_ClassGetName(parameterClasses[i])) != 0)
									{
										if (gnative_ClassIsInstance(parameterClasses[i], obj) == false)
											score += 1000;
										else
											score += 1;
									}
								}
								break;
							}
							case LUA_TFUNCTION:
								break;
							case LUA_TUSERDATA:
								break;
							case LUA_TNIL:
								if (parameterTypes[i] != GNATIVE_TYPE_OBJECT && parameterTypes[i] != GNATIVE_TYPE_STRING)
									score += 1000;
								break;
						}
						
						if(score >= minScore)
						{
							break;
						}
					}
					if(score < minScore)
					{
						candidate = function;
						minScore = score;
					}
				}
			}
			return candidate;
		}
		return NULL;
	}

	int callStaticMethod(lua_State *L)
	{
		gnative_Function *function = match(L, 0);

		if(function != NULL)
		{
			int parameterCount = lua_gettop(L);

			gnative_Type *parameterTypes = gnative_FunctionGetParameterTypes(function);

			std::vector<gnative_Value> parameters;

			for (int i = 0; i < parameterCount; ++i)
			{
				gnative_Value v;

				switch (lua_type(L, i + 1))
				{
				case LUA_TBOOLEAN:
					v.z = lua_toboolean(L, i + 1);
					break;
				case LUA_TLIGHTUSERDATA:
					break;
				case LUA_TNUMBER:
					if (parameterTypes[i] == GNATIVE_TYPE_BYTE)
						v.b = lua_tonumber(L, i + 1);
					else if (parameterTypes[i] == GNATIVE_TYPE_CHAR)
						v.c = lua_tonumber(L, i + 1);
					else if (parameterTypes[i] == GNATIVE_TYPE_SHORT)
						v.s = lua_tonumber(L, i + 1);
					else if (parameterTypes[i] == GNATIVE_TYPE_INT)
						v.i = lua_tonumber(L, i + 1);
					else if (parameterTypes[i] == GNATIVE_TYPE_LONG)
						v.l = lua_tonumber(L, i + 1);
					else if (parameterTypes[i] == GNATIVE_TYPE_FLOAT)
						v.f = lua_tonumber(L, i + 1);
					else if (parameterTypes[i] == GNATIVE_TYPE_DOUBLE)
						v.d = lua_tonumber(L, i + 1);
					break;
				case LUA_TSTRING:
					v.str = lua_tostring(L, i + 1);
					break;
				case LUA_TTABLE:
				{
					GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "NativeInstance", i + 1));
					NativeInstance *nat = static_cast<NativeInstance*>(object->proxy());
					v.obj = nat->getObject();
					break;
				}
				case LUA_TFUNCTION:
					break;
				case LUA_TUSERDATA:
					break;
				case LUA_TNIL:
					v.obj = NULL;
					break;
				}
					parameters.push_back(v);
			}
			gnative_Value result = gnative_FunctionStaticCall(function, parameters.empty() ? NULL : &parameters[0]);

			int nresult;
			switch (gnative_FunctionGetReturnType(function))
			{
				case GNATIVE_TYPE_VOID:
					nresult = 0;
					break;
				case GNATIVE_TYPE_BOOLEAN:
					lua_pushboolean(L, result.z);
					nresult = 1;
					break;
				case GNATIVE_TYPE_BYTE:
					lua_pushnumber(L, result.b);
					nresult = 1;
					break;
				case GNATIVE_TYPE_CHAR:
					lua_pushnumber(L, result.c);
					nresult = 1;
					break;
				case GNATIVE_TYPE_SHORT:
					lua_pushnumber(L, result.s);
					nresult = 1;
					break;
				case GNATIVE_TYPE_INT:
					lua_pushnumber(L, result.i);
					nresult = 1;
					break;
				case GNATIVE_TYPE_LONG:
					lua_pushnumber(L, result.l);
					nresult = 1;
					break;
				case GNATIVE_TYPE_FLOAT:
					lua_pushnumber(L, result.f);
					nresult = 1;
					break;
				case GNATIVE_TYPE_DOUBLE:
					lua_pushnumber(L, result.d);
					nresult = 1;
					break;
				case GNATIVE_TYPE_STRING:
					lua_pushstring(L, result.str);
					free((char*)result.str);
					nresult = 1;
					break;
				case GNATIVE_TYPE_OBJECT:
					nresult = 1;
					if(result.obj == NULL)
					{
						lua_pushnil(L);
					}
					else
					{
				    	NativeInstance *natIns = new NativeInstance(result.obj);

				    	g_pushInstance(L, "NativeInstance", natIns->object());
    	
						luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
						lua_pushvalue(L, -2);
						luaL_rawsetptr(L, -2, natIns);
						lua_pop(L, 1);
				    }

					break;
			}
	
			return nresult;
		}
		else 
		{
			lua_pushstring( L , "No Method or Field found for name" );
		    lua_error( L );
		}
		return 0;
	}
private:
    gnative_Class *cls_;
};

static NativeClass *getNativeClassInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "NativeClass", index));
	NativeClass *nat = static_cast<NativeClass*>(object->proxy());
    
	return nat;
}

static NativeInstance *getNativeInstanceInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "NativeInstance", index));
	NativeInstance *nat = static_cast<NativeInstance*>(object->proxy());
    
	return nat;
}

static int getClass(lua_State *L)
{
	
	const char *className = luaL_checkstring(L, 1);

    NativeClass *natCls = new NativeClass(className);

    g_pushInstance(L, "NativeClass", natCls->object());
    
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, natCls);
	lua_pop(L, 1);
    
    return 1;
}

static int createProxy(lua_State *L)
{
	const char *className = luaL_checkstring(L, 1);
	NativeProxy *nat = new NativeProxy(className);

    g_pushInstance(L, "NativeProxy", nat->object());
    
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, nat);
	lua_pop(L, 1);

	//copy the event table

	lua_pushnil(L);
    while(lua_next(L, 2) != 0) {
    	lua_pushvalue(L, -2);
    	lua_insert(L, -2);
    	lua_settable(L, -4);
    }
    
    return 1;
}

static int getPath(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	lua_pushstring(L, g_pathForFile(path));
	
    return 1;
}

static int newArray(lua_State *L)
{
	
	const char *type = luaL_checkstring(L, 1);

	std::vector<gnative_Value> elements;
	lua_pushnil(L);  // first key
	int length = 0;
	while (lua_next(L, 2) != 0) {		
		gnative_Value v;
		if(strcmp("bool", type) == 0)
		{
			v.z = lua_toboolean(L, -1);
		}
		else if(strcmp("byte", type) == 0)
		{
			v.b = lua_tonumber(L, -1);
		}
		else if(strcmp("char", type) == 0)
		{
			v.c = lua_tonumber(L, -1);
		}
		else if(strcmp("short", type) == 0)
		{
			v.s = lua_tonumber(L, -1);
		}
		else if(strcmp("int", type) == 0)
		{
			v.i = lua_tonumber(L, -1);
		}
		else if(strcmp("long", type) == 0)
		{
			v.l = lua_tonumber(L, -1);
		}
		else if(strcmp("float", type) == 0)
		{
			v.f = lua_tonumber(L, -1);
		}
		else if(strcmp("double", type) == 0)
		{
			v.d = lua_tonumber(L, -1);
		}
		else if(strcmp("string", type) == 0)
		{
			v.str = luaL_checkstring(L, -1);
		}
		// removes 'value'; keeps 'key' for next iteration*/
		lua_pop(L, 1);
		length++;
		elements.push_back(v);
	}

     gnative_Object *ob = gnative_CreatePrimitiveArray(type, elements.empty() ? NULL : &elements[0], length);

	if(ob)
	{
    	NativeInstance *natIns = new NativeInstance(ob);

    	g_pushInstance(L, "NativeInstance", natIns->object());
    	
		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
		lua_pushvalue(L, -2);
		luaL_rawsetptr(L, -2, natIns);
		lua_pop(L, 1);
    
    	return 1;
    }
    
    return 1;
//    return 0;
}

static int nativeClass_destruct(lua_State* L)
{
	void *ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	NativeClass *nat = static_cast<NativeClass*>(object->proxy());
	
	nat->unref();
	
	return 0;
}

static int nativeInstance_destruct(lua_State* L)
{
	void *ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	NativeInstance *nat = static_cast<NativeInstance*>(object->proxy());
	
	nat->unref();
	
	return 0;
}

static int nativeProxy_destruct(lua_State* L)
{
	void *ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	NativeProxy *nat = static_cast<NativeProxy*>(object->proxy());
	
	nat->unref();
	
	return 0;
}

static int createInstance(lua_State *L)
{
    NativeClass *nat = getNativeClassInstance(L, lua_upvalueindex(1));
    
    gnative_Object *ob = nat->createInstance(L);

	if(ob)
	{
    	NativeInstance *natIns = new NativeInstance(ob);

    	g_pushInstance(L, "NativeInstance", natIns->object());
    	
		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
		lua_pushvalue(L, -2);
		luaL_rawsetptr(L, -2, natIns);
		lua_pop(L, 1);
    
    	return 1;
    }
    return 0;
}

static int createArray(lua_State *L)
{
	
	NativeClass *nat = getNativeClassInstance(L, lua_upvalueindex(1));

    gnative_Object *ob = nat->createArray(L);

	if(ob)
	{
    	NativeInstance *natIns = new NativeInstance(ob);

    	g_pushInstance(L, "NativeInstance", natIns->object());
    	
		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
		lua_pushvalue(L, -2);
		luaL_rawsetptr(L, -2, natIns);
		lua_pop(L, 1);
    
    	return 1;
    }
    return 0;
}

static int callStaticMethod(lua_State *L)
{
	NativeClass *nat = getNativeClassInstance(L, lua_upvalueindex(1));
	return nat->callStaticMethod(L);
}

static int callMethod(lua_State *L)
{
	NativeInstance *nat = getNativeInstanceInstance(L, 1);
	return nat->callMethod(L);
}

void gnative_InvokeMethod(const char *methodName, int length, gnative_Value *values, int types[], long data)
{
	((NativeProxy*)data)->onInvoke(methodName, length, values, types);
}

static const char* getLuaType(lua_State *L, int i){
	switch (lua_type(L, i))
	{
		case LUA_TBOOLEAN:
			return "bool";
			break;
		case LUA_TLIGHTUSERDATA:
			return "lightuserdata";
			break;
		case LUA_TNUMBER:
			return "number";
			break;
		case LUA_TSTRING:
			return "string";
			break;
		case LUA_TTABLE:
			return "table";
		case LUA_TFUNCTION:
			return "function";
			break;
		case LUA_TUSERDATA:
			return "userdata";
			break;
		case LUA_TNIL:
			return "nil";
			break;
	}
	return "none";
}

static void printStack(lua_State *L){
	int c = lua_gettop(L);
	lua_print(L, "--------");
	for(int i = c; i > 0; i--)
		lua_print(L, getLuaType(L, i));
	lua_print(L, "--------");
}

static int cnewindex(lua_State *L)
{
	const char *key = luaL_checkstring(L, 2);
	if(strcmp(key, "__userdata") != 0)
	{
		if(g_isInstanceOf(L, "NativeClass", 1))
		{
			NativeClass *nat = getNativeClassInstance(L, 1);
			gnative_Field **fields = gnative_ClassGetFieldsByName(nat->getClass(), key);
			gnative_Field *field = *fields;
			gnative_Value v;
	
			if(field != NULL)
			{
				switch (gnative_FieldGetReturnType(field))
				{
					case GNATIVE_TYPE_VOID:
						break;
					case GNATIVE_TYPE_BOOLEAN:
						v.z = lua_toboolean(L, 3);
						break;
					case GNATIVE_TYPE_BYTE:
						v.b = lua_tonumber(L, 3);
						break;
					case GNATIVE_TYPE_CHAR:
						v.c = lua_tonumber(L, 3);
						break;
					case GNATIVE_TYPE_SHORT:
						v.s = lua_tonumber(L, 3);
						break;
					case GNATIVE_TYPE_INT:
						v.i = lua_tonumber(L, 3);
						break;
					case GNATIVE_TYPE_LONG:
						v.l = lua_tonumber(L, 3);
						break;
					case GNATIVE_TYPE_FLOAT:
						v.f = lua_tonumber(L, 3);
						break;
					case GNATIVE_TYPE_DOUBLE:
						v.d = lua_tonumber(L, 3);
						break;
					case GNATIVE_TYPE_STRING:
						v.str = lua_tostring(L, 3);
						break;
					case GNATIVE_TYPE_OBJECT:
						GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "NativeInstance", 3));
						NativeInstance *nat = static_cast<NativeInstance*>(object->proxy());
						v.obj = nat->getObject();
						break;
				}
				
				gnative_FieldSetStaticValue(field, &v);
				return 0;
			}
		}
		else if(g_isInstanceOf(L, "NativeInstance", 1))
		{
			NativeInstance *nat = getNativeInstanceInstance(L, 1);
			gnative_Field **fields = gnative_ObjectGetFieldsByName(nat->getObject(), key);
			gnative_Field *field = *fields;
			gnative_Value v;
			
			if(field != NULL)
			{
				switch (gnative_FieldGetReturnType(field))
				{
					case GNATIVE_TYPE_VOID:
						break;
					case GNATIVE_TYPE_BOOLEAN:
						v.z = lua_toboolean(L, 3);
						break;
					case GNATIVE_TYPE_BYTE:
						v.b = lua_tonumber(L, 3);
						break;
					case GNATIVE_TYPE_CHAR:
						v.c = lua_tonumber(L, 3);
						break;
					case GNATIVE_TYPE_SHORT:
						v.s = lua_tonumber(L, 3);
						break;
					case GNATIVE_TYPE_INT:
						v.i = lua_tonumber(L, 3);
						break;
					case GNATIVE_TYPE_LONG:
						v.l = lua_tonumber(L, 3);
						break;
					case GNATIVE_TYPE_FLOAT:
						v.f = lua_tonumber(L, 3);
						break;
					case GNATIVE_TYPE_DOUBLE:
						v.d = lua_tonumber(L, 3);
						break;
					case GNATIVE_TYPE_STRING:
						v.str = lua_tostring(L, 3);
						break;
					case GNATIVE_TYPE_OBJECT:
						GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "NativeInstance", 3));
						NativeInstance *nat = static_cast<NativeInstance*>(object->proxy());
						v.obj = nat->getObject();
						break;
				}
				gnative_FieldSetValue(field, nat->getObject(), &v);
				return 0;
			}
		}
	}
	lua_rawset(L, 1);
	return 0;
}

static int cindex(lua_State *L)
{
	const char *key = luaL_checkstring(L, 2);
	lua_getmetatable( L , 1 );
	lua_pushstring( L , NATIVEFUNCCALLED );
   	lua_pushstring( L , key );
	lua_rawset( L , -3 );
	lua_pop( L , 1 );

	if(g_isInstanceOf(L, "NativeClass", 1))
	{
		if(strcmp(key, "new") == 0)
		{
			lua_pushvalue(L, 1);
			lua_pushcclosure(L, &createInstance, 1);
			return 1;
		}
		else if(strcmp(key, "newArray") == 0)
		{
			lua_pushvalue(L, 1);
			lua_pushcclosure(L, &createArray, 1);
			return 1;
		}
		else
		{
			NativeClass *nat = getNativeClassInstance(L, 1);
			gnative_Function **functions = gnative_ClassGetStaticFunctionsByName(nat->getClass(), key);
			if(functions != NULL)
			{
				lua_pushvalue(L, 1);
				lua_pushcclosure(L, &callStaticMethod, 1);
				return 1;
			}
			//trying out the property
			else 
			{
				gnative_Field **fields = gnative_ClassGetFieldsByName(nat->getClass(), key);
				gnative_Field *field = *fields;

				if(field != NULL)
				{
					gnative_Value result = gnative_FieldGetStaticValue(field);

					int nresult;
					switch (gnative_FieldGetReturnType(field))
					{
						case GNATIVE_TYPE_VOID:
							nresult = 0;
							break;
						case GNATIVE_TYPE_BOOLEAN:
							lua_pushboolean(L, result.z);
							nresult = 1;
							break;
						case GNATIVE_TYPE_BYTE:
							lua_pushnumber(L, result.b);
							nresult = 1;
							break;
						case GNATIVE_TYPE_CHAR:
							lua_pushnumber(L, result.c);
							nresult = 1;
							break;
						case GNATIVE_TYPE_SHORT:
							lua_pushnumber(L, result.s);
							nresult = 1;
							break;
						case GNATIVE_TYPE_INT:
							lua_pushnumber(L, result.i);
							nresult = 1;
							break;
						case GNATIVE_TYPE_LONG:
							lua_pushnumber(L, result.l);
							nresult = 1;
							break;
						case GNATIVE_TYPE_FLOAT:
							lua_pushnumber(L, result.f);
							nresult = 1;
							break;
						case GNATIVE_TYPE_DOUBLE:
							lua_pushnumber(L, result.d);
							nresult = 1;
							break;
						case GNATIVE_TYPE_STRING:
							lua_pushstring(L, result.str);
							free((char*)result.str);
							nresult = 1;
							break;
						case GNATIVE_TYPE_OBJECT:
							nresult = 1;
							if(result.obj == NULL)
							{
								lua_pushnil(L);
							}
							else
							{
						    	NativeInstance *natIns = new NativeInstance(result.obj);
	
						    	g_pushInstance(L, "NativeInstance", natIns->object());
    	
								luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
								lua_pushvalue(L, -2);
								luaL_rawsetptr(L, -2, natIns);
								lua_pop(L, 1);
						    }
	
							break;
					}
		
					return nresult;
				}
				else
				{

					lua_pushstring( L , "No Method or Field found for name" );
				    lua_error( L );
				}
			}
		}
	}
	else if(g_isInstanceOf(L, "NativeInstance", 1))
	{
		NativeInstance *nat = getNativeInstanceInstance(L, 1);
		gnative_Function **functions = gnative_ObjectGetFunctionsByName(nat->getObject(), key);

		if(functions != NULL)
		{
			lua_pushcfunction( L , &callMethod );
			return 1;
		}
		//trying out the property
		else 
		{
			gnative_Field **fields = gnative_ObjectGetFieldsByName(nat->getObject(), key);
			gnative_Field *field = *fields;
			if(field != NULL)
			{
				gnative_Value result = gnative_FieldGetValue(field, nat->getObject());

				int nresult;
				switch (gnative_FieldGetReturnType(field))
				{
					case GNATIVE_TYPE_VOID:
						nresult = 0;
						break;
					case GNATIVE_TYPE_BOOLEAN:
						lua_pushboolean(L, result.z);
						nresult = 1;
						break;
					case GNATIVE_TYPE_BYTE:
						lua_pushnumber(L, result.b);
						nresult = 1;
						break;
					case GNATIVE_TYPE_CHAR:
						lua_pushnumber(L, result.c);
						nresult = 1;
						break;
					case GNATIVE_TYPE_SHORT:
						lua_pushnumber(L, result.s);
						nresult = 1;
						break;
					case GNATIVE_TYPE_INT:
						lua_pushnumber(L, result.i);
						nresult = 1;
						break;
					case GNATIVE_TYPE_LONG:
						lua_pushnumber(L, result.l);
						nresult = 1;
						break;
					case GNATIVE_TYPE_FLOAT:
						lua_pushnumber(L, result.f);
						nresult = 1;
						break;
					case GNATIVE_TYPE_DOUBLE:
						lua_pushnumber(L, result.d);
						nresult = 1;
						break;
					case GNATIVE_TYPE_STRING:
						lua_pushstring(L, result.str);
						free((char*)result.str);
						nresult = 1;
						break;
					case GNATIVE_TYPE_OBJECT:
						nresult = 1;
						if(result.obj == NULL)
						{
							lua_pushnil(L);
						}
						else
						{
					    	NativeInstance *natIns = new NativeInstance(result.obj);

					    	g_pushInstance(L, "NativeInstance", natIns->object());
    	
							luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
							lua_pushvalue(L, -2);
							luaL_rawsetptr(L, -2, natIns);
							lua_pop(L, 1);
					    }
	
						break;
				}
		
				return nresult;
			}
			else
			{
				lua_pushstring( L , "No Method or Field found" );
			    lua_error( L );
			}
		}
	}
	return 0;
}
static int loader(lua_State *L)
{

	const luaL_Reg nativeFunctionlist[] = {
		{"getClass", getClass},
		{"createProxy", createProxy},
		{"getPath", getPath},
		{"newArray", newArray},
		{NULL, NULL},
	};
    
    g_createMetaClass(L, "NativeClass", NULL, NULL, nativeClass_destruct, NULL);
    g_createMetaClass(L, "NativeInstance", NULL, NULL, nativeInstance_destruct, NULL);
    g_createMetaClass(L, "NativeProxy", NULL, NULL, nativeProxy_destruct, NULL);
    
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);

	luaL_register(L, "native", nativeFunctionlist);

	return 1;
}
    
static void g_initializePlugin(lua_State *L)
{
	::L = L;

	gnative_Init();
	
    lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "native");
	
	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
	::L = NULL;
	// I'm not putting gnative_Cleanup() here yet.
	//And I´m putting because I need to clear the rootview on every player restart
	gnative_Cleanup();
}

REGISTER_PLUGIN("Native", "1.0")
