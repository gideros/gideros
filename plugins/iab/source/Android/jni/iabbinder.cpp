#include "iab.h"
#include "gideros.h"
#include <map>
#include <string>
#include <vector>
#include <glog.h>

// some Lua helper functions
#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif

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

static std::vector<std::string> tableToVector(lua_State *L, int index)
{
    luaL_checktype(L, index, LUA_TTABLE);

    std::vector<std::string> result;
    int n = lua_objlen(L, index);
    for (int i = 1; i <= n; ++i)
    {
        lua_rawgeti(L, index, i);
        result.push_back(luaL_checkstring(L, -1));
        lua_pop(L, 1);
    }

    return result;
}

static std::map<std::string, std::string> tableToMap(lua_State *L, int index)
{
    luaL_checktype(L, index, LUA_TTABLE);
    
    std::map<std::string, std::string> result;
    
    int t = abs_index(L, index);
    
	lua_pushnil(L);
	while (lua_next(L, t) != 0)
	{
		lua_pushvalue(L, -2);
        std::string key = luaL_checkstring(L, -1);
		lua_pop(L, 1);
		
        std::string value = luaL_checkstring(L, -1);
		
		result[key] = value;
		
		lua_pop(L, 1);
	}
    
    return result;
}

static const char *AVAILABLE = "available";
static const char *NOT_AVAILABLE = "notAvailable";
static const char *PURCHASE_COMPLETE = "purchaseComplete";
static const char *PURCHASE_ERROR = "purchaseError";
static const char *PRODUCTS_COMPLETE = "productsComplete";
static const char *PRODUCTS_ERROR = "productsError";
static const char *RESTORE_COMPLETE = "restoreComplete";
static const char *RESTORE_ERROR = "restoreError";

static char keyWeak = ' ';

class IAB : public GEventDispatcherProxy
{
public:
    IAB(lua_State *L, const char *iab) : L(L)
    {
		iab_ = strdup(iab);
		giab_initialize(iab);
		giab_addCallback(callback_s, this);		
    }
    
    ~IAB()
    {
		giab_destroy(iab_);
		giab_removeCallback(callback_s, this);
    }
	
	void setup(giab_Parameter *params)
	{
		giab_setup(iab_, params);
	}

	void check()
	{
		giab_check(iab_);
	}
	
	void setProducts(giab_DoubleEvent *params)
	{
		giab_setProducts(iab_, params);
	}
	
	void setConsumables(const char * const *params)
	{
		giab_setConsumables(iab_, params);
	}
	
	void request()
	{
		giab_request(iab_);
	}

	void purchase(const char *productId)
	{
		giab_purchase(iab_, productId);
	}

	void restore()
	{
		giab_restore(iab_);
	}

private:
	static void callback_s(int type, void *event, void *udata)
	{
		static_cast<IAB*>(udata)->callback(type, event);
	}
	
	void callback(int type, void *event)
	{
        dispatchEvent(type, event);
	}
	
	void dispatchEvent(int type, void *event)
	{
		int shouldDispatch = 0;
		if (type == GIAB_PURCHASE_ERROR_EVENT || type == GIAB_PRODUCTS_ERROR_EVENT || type == GIAB_RESTORE_ERROR_EVENT)
        {
			giab_DoubleEvent *event2 = (giab_DoubleEvent*)event;
			if(strcmp(event2->iap, iab_) == 0)
			{
				shouldDispatch = 1;
			}
		}
		else if(type == GIAB_AVAILABLE_EVENT || type == GIAB_NOT_AVAILABLE_EVENT || type == GIAB_RESTORE_COMPLETE_EVENT)
		{
			giab_SimpleEvent *event2 = (giab_SimpleEvent*)event;
			if(strcmp(event2->iap, iab_) == 0)
			{
				shouldDispatch = 1;
			}
		}
		else if(type == GIAB_PURCHASE_COMPLETE_EVENT)
		{
			giab_Purchase *event2 = (giab_Purchase*)event;
			if(strcmp(event2->iap, iab_) == 0)
			{
				shouldDispatch = 1;
			}
		}
		else if(type == GIAB_PRODUCTS_COMPLETE_EVENT)
		{
			giab_Products *event2 = (giab_Products*)event;
			if(strcmp(event2->iap, iab_) == 0)
			{
				shouldDispatch = 1;
			}
		}
		if(shouldDispatch)
		{
			luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
			luaL_rawgetptr(L, -1, this);
			
			if (lua_isnil(L, -1))
			{
				lua_pop(L, 2);
				return;
			}
			
			lua_getfield(L, -1, "dispatchEvent");
			
			lua_pushvalue(L, -2);
			
			lua_getglobal(L, "Event");
			lua_getfield(L, -1, "new");
			lua_remove(L, -2);
			
			switch (type)
			{
				case GIAB_AVAILABLE_EVENT:
					lua_pushstring(L, AVAILABLE);
					break;
				case GIAB_NOT_AVAILABLE_EVENT:
					lua_pushstring(L, NOT_AVAILABLE);
					break;
				case GIAB_PURCHASE_COMPLETE_EVENT:
					lua_pushstring(L, PURCHASE_COMPLETE);
					break;
				case GIAB_PURCHASE_ERROR_EVENT:
					lua_pushstring(L, PURCHASE_ERROR);
					break;
				case GIAB_PRODUCTS_COMPLETE_EVENT:
					lua_pushstring(L, PRODUCTS_COMPLETE);
					break;
				case GIAB_PRODUCTS_ERROR_EVENT:
					lua_pushstring(L, PRODUCTS_ERROR);
					break;
				case GIAB_RESTORE_COMPLETE_EVENT:
					lua_pushstring(L, RESTORE_COMPLETE);
					break;
				case GIAB_RESTORE_ERROR_EVENT:
					lua_pushstring(L, RESTORE_ERROR);
					break;
			}
	
			lua_call(L, 1, 1);
			
			if (type == GIAB_PURCHASE_ERROR_EVENT || type == GIAB_PRODUCTS_ERROR_EVENT || type == GIAB_RESTORE_ERROR_EVENT)
			{
				giab_DoubleEvent *event2 = (giab_DoubleEvent*)event;
				lua_pushstring(L, event2->value);
				lua_setfield(L, -2, "error");
			}
			else if(type == GIAB_PURCHASE_COMPLETE_EVENT)
			{
				giab_Purchase *event2 = (giab_Purchase*)event;
				lua_pushstring(L, event2->productId);
				lua_setfield(L, -2, "productId");
				lua_pushstring(L, event2->receiptId);
				lua_setfield(L, -2, "receiptId");
			}
			else if(type == GIAB_PRODUCTS_COMPLETE_EVENT)
			{
				giab_Products *event2 = (giab_Products*)event;
				lua_newtable(L);
				for (int i = 0; i < event2->count; ++i)
				{					
					lua_newtable(L);
					
					lua_pushstring(L, event2->products[i].productId);
					lua_setfield(L, -2, "productId");
		
					lua_pushstring(L, event2->products[i].title);
					lua_setfield(L, -2, "title");
					
					lua_pushstring(L, event2->products[i].description);
					lua_setfield(L, -2, "description");
					
					lua_pushstring(L, event2->products[i].price);
					lua_setfield(L, -2, "price");
					
					lua_rawseti(L, -2, i + 1);
				}
				lua_setfield(L, -2, "products");
			}
	
			lua_call(L, 2, 0);
			
			lua_pop(L, 2);
		}
	}

private:
    lua_State *L;
    const char* iab_;
};

static int destruct(lua_State* L)
{
	void *ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	IAB *iab = static_cast<IAB*>(object->proxy());
	
	iab->unref();
	
	return 0;
}

static IAB *getInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "IAB", index));
	IAB *iab = static_cast<IAB*>(object->proxy());
	return iab;
}

static int detectStores(lua_State* L)
{
	int i = 1;
	std::vector<giab_SimpleParam> params2;
	while(!lua_isnoneornil(L, i))
	{
		const char *p = luaL_checkstring(L, i);
		giab_SimpleParam param = {p};
		params2.push_back(param);
		i++;
	}
	giab_SimpleParam param = {""};
	params2.push_back(param);
	
	giab_SimpleParam* params = giab_detectStores(&params2[0]);
	lua_newtable(L);
    if (params)
    {
        int i = 0;
		while (!params->value.empty())
		{
            //set key
            lua_pushinteger(L, i+1);
            lua_pushstring(L, params->value.c_str());
            //back to table
            lua_settable(L, -3);
			
			params++;
			i++;
        }
    }
    lua_pushvalue(L, -1);
	return 1;
}

static int init(lua_State* L)
{
	const char *iabp = luaL_checkstring(L, 1);
	IAB *iab = new IAB(L, iabp);
	g_pushInstance(L, "IAB", iab->object());
    
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, iab);
	lua_pop(L, 1);
    
	lua_pushvalue(L, -1);
	return 1;
}

static int setup(lua_State *L)
{
    IAB *iab = getInstance(L, 1);
	int i = 2;
	std::vector<giab_Parameter> params2;
	while(!lua_isnoneornil(L, i))
	{
		size_t size;
		const char *p = luaL_checklstring(L, i, &size);
		giab_Parameter param = {p, size};
		params2.push_back(param);
		i++;
	}
	giab_Parameter param = {NULL, 0};
	params2.push_back(param);
	iab->setup(&params2[0]);
    return 0;
}

static int setProducts(lua_State* L)
{
	IAB *iab = getInstance(L, 1);
	std::map<std::string, std::string> params = tableToMap(L, 2);

	std::vector<giab_DoubleEvent> params2;
        
	std::map<std::string, std::string>::iterator iter, e = params.end();
	for (iter = params.begin(); iter != e; ++iter)
	{
		giab_DoubleEvent param = {iter->first.c_str(), iter->second.c_str()};
		params2.push_back(param);
	}
	giab_DoubleEvent param = {NULL, NULL};
	params2.push_back(param);
	
	iab->setProducts(&params2[0]);
	return 0;
}

static int setConsumables(lua_State* L)
{
	IAB *iab = getInstance(L, 1);
	std::vector<std::string> param = tableToVector(L, 2);

	std::vector<const char*> param2;
	for (size_t i = 0; i < param.size(); ++i)
		param2.push_back(param[i].c_str());
	param2.push_back(NULL);
	
	iab->setConsumables(&param2[0]);
	return 0;
}

static int request(lua_State* L)
{
	IAB *iab = getInstance(L, 1);
	iab->request();
	return 0;
}

static int check(lua_State *L)
{
    IAB *iab = getInstance(L, 1);

    iab->check();
    
    return 0;
}

static int purchase(lua_State *L)
{
    IAB *iab = getInstance(L, 1);
	const char *productId = luaL_checkstring(L, 2);
	iab->purchase(productId);
    return 0;
}

static int restore(lua_State *L)
{
    IAB *iab = getInstance(L, 1);
    iab->restore();
    return 0;
}

static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
        {"detectStores", detectStores},
        {"new", init},
        {"setUp", setup},
        {"setProducts", setProducts},
        {"setConsumables", setConsumables},
        {"isAvailable", check},
        {"requestProducts", request},
        {"purchase", purchase},
        {"restore", restore},
		{NULL, NULL},
	};
    
    g_createClass(L, "IAB", "EventDispatcher", NULL, destruct, functionlist);
    
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	
	lua_getglobal(L, "Event");
	lua_pushstring(L, AVAILABLE);
	lua_setfield(L, -2, "AVAILABLE");
	lua_pushstring(L, NOT_AVAILABLE);
	lua_setfield(L, -2, "NOT_AVAILABLE");
	lua_pushstring(L, PURCHASE_COMPLETE);
	lua_setfield(L, -2, "PURCHASE_COMPLETE");
	lua_pushstring(L, PURCHASE_ERROR);
	lua_setfield(L, -2, "PURCHASE_ERROR");
	lua_pushstring(L, PRODUCTS_COMPLETE);
	lua_setfield(L, -2, "PRODUCTS_COMPLETE");
	lua_pushstring(L, PRODUCTS_ERROR);
	lua_setfield(L, -2, "PRODUCTS_ERROR");
	lua_pushstring(L, RESTORE_COMPLETE);
	lua_setfield(L, -2, "RESTORE_COMPLETE");
	lua_pushstring(L, RESTORE_ERROR);
	lua_setfield(L, -2, "RESTORE_ERROR");
	lua_pop(L, 1);
    
    return 0;
}
    
static void g_initializePlugin(lua_State *L)
{
    lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "iab");
	
	lua_pop(L, 2);
	giab_init();
}

static void g_deinitializePlugin(lua_State *L)
{
    giab_cleanup();
}

REGISTER_PLUGIN("IAB", "1.0")
