#include "gfacebook.h"
#include "gideros.h"
#include <map>
#include <string>
#include <vector>


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

static const char *LOGIN_COMPLETE = "loginComplete";
static const char *LOGIN_ERROR = "loginError";
static const char *LOGIN_CANCEL = "loginCancel";
static const char *LOGOUT_COMPLETE = "logoutComplete";
static const char *DIALOG_COMPLETE = "dialogComplete";
static const char *DIALOG_ERROR = "dialogError";
static const char *DIALOG_CANCEL = "dialogCancel";
static const char *REQUEST_COMPLETE = "requestComplete";
static const char *REQUEST_ERROR = "requestError";

static char keyWeak = ' ';

static lua_State *L = NULL;

class GFacebook : public GEventDispatcherProxy
{
public:
    GFacebook()
    {
        gfacebook_init();
        gfacebook_addCallback(callback_s, this);
        
        initialized_ = false;
    }
    
    ~GFacebook()
    {
        gfacebook_removeCallback(callback_s, this);
        gfacebook_cleanup();
    }

    void setAppId(lua_State *L, const char *appId)
    {
        if (initialized_)
            luaL_error(L, "Facebook App ID is already set.");
        
        gfacebook_setAppId(appId);
        initialized_ = true;
    }

    void checkInit(lua_State *L)
    {
        if (!initialized_)
            luaL_error(L, "Facebook App ID is not set.");
    }

    void authorize(lua_State *L, const char * const *permissions)
    {
        checkInit(L);

        gfacebook_authorize(permissions);
    }

    void logout(lua_State *L)
    {
        checkInit(L);

        gfacebook_logout();
    }

    int isSessionValid(lua_State *L)
    {
        checkInit(L);

        return gfacebook_isSessionValid();
    }
    
    void dialog(lua_State *L, const char *action, gfacebook_Parameter *params)
    {
        checkInit(L);

        gfacebook_dialog(action, params);
    }

    void graphRequest(lua_State *L, const char *graphPath, gfacebook_Parameter *params, const char *httpMethod)
    {
        checkInit(L);

        gfacebook_graphRequest(graphPath, params, httpMethod);
    }
    
    void setAccessToken(lua_State *L, const char *accessToken)
    {
        checkInit(L);

        gfacebook_setAccessToken(accessToken);
    }
    
    const char *getAccessToken(lua_State *L)
    {
        checkInit(L);

        return gfacebook_getAccessToken();
    }
    
    void setExpirationDate(lua_State *L, time_t time)
    {
        checkInit(L);

        gfacebook_setExpirationDate(time);
    }
    
    time_t getExpirationDate(lua_State *L)
    {
        checkInit(L);

        return gfacebook_getExpirationDate();
    }
    
    void extendAccessToken(lua_State *L)
    {
        checkInit(L);

        gfacebook_extendAccessToken();
    }
    
    void extendAccessTokenIfNeeded(lua_State *L)
    {
        checkInit(L);

        gfacebook_extendAccessTokenIfNeeded();
    }
    
    int shouldExtendAccessToken(lua_State *L)
    {
        checkInit(L);

        return gfacebook_shouldExtendAccessToken();
    }
    
private:
    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<GFacebook*>(udata)->callback(type, event);
    }
    
    void callback(int type, void *event)
    {
        dispatchEvent(type, event);
    }

	void dispatchEvent(int type, void *event)
	{
        if (L == NULL)
            return;
        
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
            case GFACEBOOK_LOGIN_COMPLETE_EVENT:
                lua_pushstring(L, LOGIN_COMPLETE);
                break;
            case GFACEBOOK_LOGIN_ERROR_EVENT:
                lua_pushstring(L, LOGIN_ERROR);
                break;
            case GFACEBOOK_LOGIN_CANCEL_EVENT:
                lua_pushstring(L, LOGIN_CANCEL);
                break;
            case GFACEBOOK_LOGOUT_COMPLETE_EVENT:
                lua_pushstring(L, LOGOUT_COMPLETE);
                break;
            case GFACEBOOK_DIALOG_COMPLETE_EVENT:
                lua_pushstring(L, DIALOG_COMPLETE);
                break;
            case GFACEBOOK_DIALOG_ERROR_EVENT:
                lua_pushstring(L, DIALOG_ERROR);
                break;
            case GFACEBOOK_DIALOG_CANCEL_EVENT:
                lua_pushstring(L, DIALOG_CANCEL);
                break;
            case GFACEBOOK_REQUEST_COMPLETE_EVENT:
                lua_pushstring(L, REQUEST_COMPLETE);
                break;
            case GFACEBOOK_REQUEST_ERROR_EVENT:
                lua_pushstring(L, REQUEST_ERROR);
                break;
        }
        lua_call(L, 1, 1);

        if (type == GFACEBOOK_DIALOG_ERROR_EVENT)
        {
            gfacebook_DialogErrorEvent *event2 = (gfacebook_DialogErrorEvent*)event;
            
			lua_pushinteger(L, event2->errorCode);
			lua_setfield(L, -2, "errorCode");
			
			lua_pushstring(L, event2->errorDescription);
			lua_setfield(L, -2, "errorDescription");
        }
        else if (type == GFACEBOOK_REQUEST_COMPLETE_EVENT)
        {
            gfacebook_RequestCompleteEvent *event2 = (gfacebook_RequestCompleteEvent*)event;
            
            lua_pushlstring(L, event2->response, event2->responseLength);
			lua_setfield(L, -2, "response");
        }
        else if (type == GFACEBOOK_REQUEST_ERROR_EVENT)
        {
            gfacebook_RequestErrorEvent *event2 = (gfacebook_RequestErrorEvent*)event;
            
			lua_pushinteger(L, event2->errorCode);
			lua_setfield(L, -2, "errorCode");
			
			lua_pushstring(L, event2->errorDescription);
			lua_setfield(L, -2, "errorDescription");
        }

		lua_call(L, 2, 0);
		
		lua_pop(L, 2);
	}

private:
    bool initialized_;
};

static int destruct(lua_State* L)
{
	void *ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	GFacebook *facebook = static_cast<GFacebook*>(object->proxy());
	
	facebook->unref();
	
	return 0;
}

static GFacebook *getInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "Facebook", index));
	GFacebook *facebook = static_cast<GFacebook*>(object->proxy());
    
	return facebook;
}

static int setAppId(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    
    const char *appId = luaL_checkstring(L, 2);
    
    facebook->setAppId(L, appId);
    
    return 0;
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


static int authorize(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    
    if (lua_isnoneornil(L, 2))
        facebook->authorize(L, NULL);
    else
    {
        std::vector<std::string> permissions = tableToVector(L, 2);

        std::vector<const char*> permissions2;
        for (size_t i = 0; i < permissions.size(); ++i)
            permissions2.push_back(permissions[i].c_str());
        permissions2.push_back(NULL);

        facebook->authorize(L, &permissions2[0]);
    }
    
    return 0;
}

static int logout(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    
    facebook->logout(L);
    
    return 0;
}

static int isSessionValid(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    
    lua_pushboolean(L, facebook->isSessionValid(L));
    
    return 1;
}

static int dialog(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    
    const char *action = luaL_checkstring(L, 2);
    
    if (lua_isnoneornil(L, 3))
    {
        facebook->dialog(L, action, NULL);
    }
    else
    {
        std::map<std::string, std::string> params = tableToMap(L, 3);

        std::vector<gfacebook_Parameter> params2;
        
        std::map<std::string, std::string>::iterator iter, e = params.end();
        for (iter = params.begin(); iter != e; ++iter)
        {
            gfacebook_Parameter param = {iter->first.c_str(), iter->second.c_str()};
            params2.push_back(param);
        }
        gfacebook_Parameter param = {NULL, NULL};
        params2.push_back(param);
        
        facebook->dialog(L, action, &params2[0]);
    }
    
    return 0;
}

static int graphRequest(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    
    const char *graphPath = luaL_checkstring(L, 2);
    
    if (lua_isnoneornil(L, 3) && lua_isnoneornil(L, 4))
    {
        facebook->graphRequest(L, graphPath, NULL, NULL);
    }
    else
    {
        std::map<std::string, std::string> params = lua_isnoneornil(L, 3) ? std::map<std::string, std::string>() : tableToMap(L, 3);

        std::vector<gfacebook_Parameter> params2;
        
        std::map<std::string, std::string>::iterator iter, e = params.end();
        for (iter = params.begin(); iter != e; ++iter)
        {
            gfacebook_Parameter param = {iter->first.c_str(), iter->second.c_str()};
            params2.push_back(param);
        }
        gfacebook_Parameter param = {NULL, NULL};
        params2.push_back(param);
        
        if (lua_isnoneornil(L, 4))
        {
            facebook->graphRequest(L, graphPath, &params2[0], NULL);
        }
        else
        {
            const char *httpMethod = luaL_checkstring(L, 4);
            facebook->graphRequest(L, graphPath, &params2[0], httpMethod);
        }
    }
    
    return 0;
}

static int setAccessToken(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    
    const char *accessToken = luaL_checkstring(L, 2);
    
    facebook->setAccessToken(L, accessToken);
    
    return 0;
}

static int getAccessToken(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    
    const char *accessToken = facebook->getAccessToken(L);
    
    if (accessToken)
        lua_pushstring(L, accessToken);
    else
        lua_pushnil(L);
    
    return 1;
}

static char *time2str(time_t t, char *str)
{
    strftime(str, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
    return str;
}

static time_t str2time(const char *str)
{
    struct tm t;
    memset(&t, 0, sizeof(struct tm));
    strptime(str, "%Y-%m-%d %H:%M:%S", &t);
    t.tm_isdst = -1;  // Not set by strptime(); tells mktime() to determine whether daylight saving time is in effect
    return mktime(&t);
}

static int setExpirationDate(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    const char *date = luaL_checkstring(L, 2);

    facebook->setExpirationDate(L, str2time(date));

    return 0;
}

static int getExpirationDate(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    
    time_t expirationDate = facebook->getExpirationDate(L);
    
    if (expirationDate == -1)
    {
        lua_pushnil(L);
    }
    else
    {
        char buffer[20];
        time2str(expirationDate, buffer);
        lua_pushstring(L, buffer);
    }
    
    return 1;
}

static int extendAccessToken(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    
    facebook->extendAccessToken(L);
    
    return 0;
}

static int extendAccessTokenIfNeeded(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    
    facebook->extendAccessTokenIfNeeded(L);
    
    return 0;
}

static int shouldExtendAccessToken(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    
    lua_pushboolean(L, facebook->shouldExtendAccessToken(L));
    
    return 1;
}

static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
        {"setAppId", setAppId},
        {"authorize", authorize},
        {"logout", logout},
        {"isSessionValid", isSessionValid},
        {"dialog", dialog},
        {"graphRequest", graphRequest},
        {"setAccessToken", setAccessToken},
        {"getAccessToken", getAccessToken},
        {"setExpirationDate", setExpirationDate},
        {"getExpirationDate", getExpirationDate},
        {"extendAccessToken", extendAccessToken},
        {"extendAccessTokenIfNeeded", extendAccessTokenIfNeeded},
        {"shouldExtendAccessToken", shouldExtendAccessToken},
		{NULL, NULL},
	};
    
    g_createClass(L, "Facebook", "EventDispatcher", NULL, destruct, functionlist);
    
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    
	lua_getglobal(L, "Event");
	lua_pushstring(L, LOGIN_COMPLETE);
	lua_setfield(L, -2, "LOGIN_COMPLETE");
	lua_pushstring(L, LOGIN_ERROR);
	lua_setfield(L, -2, "LOGIN_ERROR");
	lua_pushstring(L, LOGIN_CANCEL);
	lua_setfield(L, -2, "LOGIN_CANCEL");
    lua_pushstring(L, LOGOUT_COMPLETE);
	lua_setfield(L, -2, "LOGOUT_COMPLETE");
    lua_pushstring(L, DIALOG_COMPLETE);
	lua_setfield(L, -2, "DIALOG_COMPLETE");
    lua_pushstring(L, DIALOG_ERROR);
	lua_setfield(L, -2, "DIALOG_ERROR");
    lua_pushstring(L, DIALOG_CANCEL);
	lua_setfield(L, -2, "DIALOG_CANCEL");
    lua_pushstring(L, REQUEST_COMPLETE);
	lua_setfield(L, -2, "REQUEST_COMPLETE");
    lua_pushstring(L, REQUEST_ERROR);
	lua_setfield(L, -2, "REQUEST_ERROR");
	lua_pop(L, 1);
	
    GFacebook *facebook = new GFacebook;
	g_pushInstance(L, "Facebook", facebook->object());
    
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, facebook);
	lua_pop(L, 1);
    
	lua_pushvalue(L, -1);
	lua_setglobal(L, "facebook");
    
    return 1;
}
    
static void g_initializePlugin(lua_State *L)
{
    ::L = L;
    
    lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "facebook");
	
	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
    ::L = NULL;
}

REGISTER_PLUGIN("Facebook", "1.0")

