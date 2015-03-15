#include "googlelvl.h"
#include "gideros.h"
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

static const char *LICENSE_ALLOWED = "licenseAllowed";
static const char *LICENSE_DENIED = "licenseDenied";
static const char *LICENSE_RETRY = "licenseRetry";
static const char *ERROR = "error";
static const char *DOWNLOAD_REQUIRED = "downloadRequried";
static const char *DOWNLOAD_NOT_REQUIRED = "downloadNotReqruired";
static const char *DOWNLOAD_STATE = "downloadState";
static const char *DOWNLOAD_PROGRESS = "downloadProgress";

static char keyWeak = ' ';

class GoogleLVL : public GEventDispatcherProxy
{
public:
    GoogleLVL(lua_State *L) : L(L)
    {
        ggooglelvl_init();
		ggooglelvl_addCallback(callback_s, this);		
    }
    
    ~GoogleLVL()
    {
		ggooglelvl_removeCallback(callback_s, this);
		ggooglelvl_cleanup();
    }
	
	void setKey(const char *key)
	{
		ggooglelvl_setKey(key);
	}

	void checkLicense()
	{
		ggooglelvl_checkLicense();
	}
	
	void checkExpansion()
	{
		ggooglelvl_checkExpansion();
	}
	
	void cellularDownload(int use)
	{
		ggooglelvl_cellularDownload(use);
	}

private:
	static void callback_s(int type, void *event, void *udata)
	{
		static_cast<GoogleLVL*>(udata)->callback(type, event);
	}
	
	void callback(int type, void *event)
	{
        dispatchEvent(type, event);
	}
	
	void dispatchEvent(int type, void *event)
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
            case GOOGLELVL_ALLOW_EVENT:
                lua_pushstring(L, LICENSE_ALLOWED);
                break;
            case GOOGLELVL_DISALLOW_EVENT:
                lua_pushstring(L, LICENSE_DENIED);
                break;
            case GOOGLELVL_RETRY_EVENT:
                lua_pushstring(L, LICENSE_RETRY);
                break;
			case GOOGLELVL_DOWNLOAD_REQUIRED_EVENT:
                lua_pushstring(L, DOWNLOAD_REQUIRED);
                break;
			case GOOGLELVL_DOWNLOAD_NOT_REQUIRED_EVENT:
                lua_pushstring(L, DOWNLOAD_NOT_REQUIRED);
                break;
			case GOOGLELVL_DOWNLOAD_STATE_EVENT:
                lua_pushstring(L, DOWNLOAD_STATE);
                break;
			case GOOGLELVL_DOWNLOAD_PROGRESS_EVENT:
                lua_pushstring(L, DOWNLOAD_PROGRESS);
                break;
			case GOOGLELVL_ERROR_EVENT:
                lua_pushstring(L, ERROR);
                break;
        }

        lua_call(L, 1, 1);
		
		if (type == GOOGLELVL_ERROR_EVENT)
        {
            ggooglelvl_SimpleEvent *event2 = (ggooglelvl_SimpleEvent*)event;
            
			lua_pushstring(L, event2->error);
			lua_setfield(L, -2, "error");
        }
		else if (type == GOOGLELVL_DOWNLOAD_PROGRESS_EVENT)
        {
            ggooglelvl_ProgressEvent *event2 = (ggooglelvl_ProgressEvent*)event;
            
			lua_pushnumber(L, event2->speed);
			lua_setfield(L, -2, "speed");
			lua_pushnumber(L, event2->time);
			lua_setfield(L, -2, "timeLeft");
			lua_pushnumber(L, event2->progress);
			lua_setfield(L, -2, "progress");
			lua_pushnumber(L, event2->total);
			lua_setfield(L, -2, "total");
        }
		else if (type == GOOGLELVL_DOWNLOAD_STATE_EVENT)
        {
            ggooglelvl_StateEvent *event2 = (ggooglelvl_StateEvent*)event;
            
			lua_pushstring(L, event2->state);
			lua_setfield(L, -2, "state");
			lua_pushstring(L, event2->message);
			lua_setfield(L, -2, "message");
        }

		lua_call(L, 2, 0);
		
		lua_pop(L, 2);
	}

private:
    lua_State *L;
    bool initialized_;
};

static int destruct(lua_State* L)
{
	void *ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	GoogleLVL *googlelvl = static_cast<GoogleLVL*>(object->proxy());
	
	googlelvl->unref();
	
	return 0;
}

static GoogleLVL *getInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "GoogleLVL", index));
	GoogleLVL *googlelvl = static_cast<GoogleLVL*>(object->proxy());
    
	return googlelvl;
}

static int setKey(lua_State *L)
{
    GoogleLVL *googlelvl = getInstance(L, 1);
    
    const char *key = luaL_checkstring(L, 2);
    
    googlelvl->setKey(key);
    
    return 0;
}

static int checkLicense(lua_State *L)
{
    GoogleLVL *googlelvl = getInstance(L, 1);
    
    googlelvl->checkLicense();
    
    return 0;
}

static int checkExpansion(lua_State *L)
{
    GoogleLVL *googlelvl = getInstance(L, 1);
    googlelvl->checkExpansion();
    return 0;
}

static int cellularDownload(lua_State *L)
{
    GoogleLVL *googlelvl = getInstance(L, 1);
    googlelvl->cellularDownload(lua_toboolean(L, 2));
    return 0;
}

static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
        {"setKey", setKey},
        {"checkLicense", checkLicense},
        {"checkExpansion", checkExpansion},
        {"cellularDownload", cellularDownload},
		{NULL, NULL},
	};
    
    g_createClass(L, "GoogleLicensing", "EventDispatcher", NULL, destruct, functionlist);
    
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    
	lua_getglobal(L, "Event");
	lua_pushstring(L, LICENSE_ALLOWED);
	lua_setfield(L, -2, "LICENSE_ALLOWED");
	lua_pushstring(L, LICENSE_DENIED);
	lua_setfield(L, -2, "LICENSE_DENIED");
	lua_pushstring(L, LICENSE_RETRY);
	lua_setfield(L, -2, "LICENSE_RETRY");
	lua_pushstring(L, DOWNLOAD_REQUIRED);
	lua_setfield(L, -2, "DOWNLOAD_REQUIRED");
	lua_pushstring(L, DOWNLOAD_NOT_REQUIRED);
	lua_setfield(L, -2, "DOWNLOAD_NOT_REQUIRED");
	lua_pushstring(L, DOWNLOAD_STATE);
	lua_setfield(L, -2, "DOWNLOAD_STATE");
	lua_pushstring(L, DOWNLOAD_PROGRESS);
	lua_setfield(L, -2, "DOWNLOAD_PROGRESS");
	lua_pushstring(L, ERROR);
	lua_setfield(L, -2, "ERROR");
	lua_pop(L, 1);
	
    GoogleLVL *googlelvl = new GoogleLVL(L);
	g_pushInstance(L, "GoogleLicensing", googlelvl->object());
    
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, googlelvl);
	lua_pop(L, 1);
    
	lua_pushvalue(L, -1);
	lua_setglobal(L, "googlelicensing");
    
    return 1;
}
    
static void g_initializePlugin(lua_State *L)
{
    lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "googlelicensing");
	
	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
    
}

REGISTER_PLUGIN("Google Licensing", "1.0")
