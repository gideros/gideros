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
static const char *LOGOUT_COMPLETE = "logoutComplete";
static const char *LOGOUT_ERROR = "logoutError";
static const char *OPEN_URL = "openUrl";
static const char *DIALOG_COMPLETE = "dialogComplete";
static const char *DIALOG_ERROR = "dialogError";
static const char *REQUEST_COMPLETE = "requestComplete";
static const char *REQUEST_ERROR = "requestError";

static const int HTTP_GET = 0;
static const int HTTP_POST = 1;
static const int HTTP_DELETE = 2;

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

    void login(const char* appId, const char * const *permissions)
    {
        gfacebook_login(appId, permissions);
		appId_ = strdup(appId);
    }

    void logout()
    {
        gfacebook_logout();
    }
	
	void upload(const char* path, const char* orig)
    {
        gfacebook_upload(path, orig);
    }
    
    const char* getAccessToken(){
        return gfacebook_getAccessToken();
    }
    
    time_t getExpirationDate(){
        return gfacebook_getExpirationDate();
    }
    
    void dialog(const char *action, gfacebook_Parameter *params)
    {
        gfacebook_dialog(action, params);
    }

    void request(const char *graphPath, gfacebook_Parameter *params, int httpMethod)
    {
        gfacebook_request(graphPath, params, httpMethod);
    }
	
	const char* getAppId(){
		return appId_;
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
            case GFACEBOOK_LOGOUT_COMPLETE_EVENT:
                lua_pushstring(L, LOGOUT_COMPLETE);
                break;
			case GFACEBOOK_LOGOUT_ERROR_EVENT:
                lua_pushstring(L, LOGOUT_ERROR);
                break;
			case GFACEBOOK_OPEN_URL_EVENT:
                lua_pushstring(L, OPEN_URL);
                break;
            case GFACEBOOK_DIALOG_COMPLETE_EVENT:
                lua_pushstring(L, DIALOG_COMPLETE);
                break;
            case GFACEBOOK_DIALOG_ERROR_EVENT:
                lua_pushstring(L, DIALOG_ERROR);
                break;
            case GFACEBOOK_REQUEST_COMPLETE_EVENT:
                lua_pushstring(L, REQUEST_COMPLETE);
                break;
            case GFACEBOOK_REQUEST_ERROR_EVENT:
                lua_pushstring(L, REQUEST_ERROR);
                break;
        }
        lua_call(L, 1, 1);

        if (type == GFACEBOOK_LOGIN_ERROR_EVENT || type == GFACEBOOK_LOGOUT_ERROR_EVENT)
        {
            gfacebook_SimpleEvent *event2 = (gfacebook_SimpleEvent*)event;
            
			lua_pushstring(L, event2->value);
			lua_setfield(L, -2, "error");
        }
		else if(type == GFACEBOOK_OPEN_URL_EVENT){
            gfacebook_SimpleEvent *event2 = (gfacebook_SimpleEvent*)event;
            
			lua_pushstring(L, event2->value);
			lua_setfield(L, -2, "url");
        }
		else if(type == GFACEBOOK_DIALOG_COMPLETE_EVENT)
		{
			gfacebook_DoubleEvent *event2 = (gfacebook_DoubleEvent*)event;
            
			lua_pushstring(L, event2->type);
			lua_setfield(L, -2, "type");
			
			lua_pushstring(L, event2->value);
			lua_setfield(L, -2, "response");
		}
        else if (type == GFACEBOOK_REQUEST_COMPLETE_EVENT)
        {
            gfacebook_ResponseEvent *event2 = (gfacebook_ResponseEvent*)event;
			
			lua_pushstring(L, event2->type);
			lua_setfield(L, -2, "type");
            
            lua_pushlstring(L, event2->response, event2->responseLength);
			lua_setfield(L, -2, "response");
        }
        else if (type == GFACEBOOK_REQUEST_ERROR_EVENT || type == GFACEBOOK_DIALOG_ERROR_EVENT)
        {
            gfacebook_DoubleEvent *event2 = (gfacebook_DoubleEvent*)event;
            
			lua_pushstring(L, event2->type);
			lua_setfield(L, -2, "type");
			
			lua_pushstring(L, event2->value);
			lua_setfield(L, -2, "error");
        }

		lua_call(L, 2, 0);
		
		lua_pop(L, 2);
	}

private:
    bool initialized_;
	const char* appId_;
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
		
		if(key == "path")
			value = g_pathForFile(value.c_str());
			
		
		result[key] = value;
		
		lua_pop(L, 1);
	}
    
    return result;
}


static int login(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    
    if (lua_isnoneornil(L, 3))
        facebook->login(luaL_checkstring(L, 2), NULL);
    else
    {
        std::vector<std::string> permissions = tableToVector(L, 3);

        std::vector<const char*> permissions2;
        for (size_t i = 0; i < permissions.size(); ++i)
            permissions2.push_back(permissions[i].c_str());
        permissions2.push_back(NULL);

        facebook->login(luaL_checkstring(L, 2), &permissions2[0]);
    }
    
    return 0;
}

static int logout(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);   
    facebook->logout();
    return 0;
}

static int upload(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);   
	const char *path = luaL_checkstring(L, 2);
    facebook->upload(g_pathForFile(path), path);
    return 0;
}

static int getAccessToken(lua_State *L){
    GFacebook *facebook = getInstance(L, 1);
    lua_pushstring(L, facebook->getAccessToken());
    return 1;
}

static char *time2str(time_t t, char *str)
{
    strftime(str, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
    return str;
}

static int getExpirationDate(lua_State *L){
    GFacebook *facebook = getInstance(L, 1);
    time_t expirationDate = facebook->getExpirationDate();
    
    char buffer[20];
    time2str(expirationDate, buffer);
    lua_pushstring(L, buffer);
    return 1;
}

static int dialog(lua_State *L)
{
    GFacebook *facebook = getInstance(L, 1);
    
    const char *action = luaL_checkstring(L, 2);
    
    if (lua_isnoneornil(L, 3))
    {
        facebook->dialog(action, NULL);
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
        
        facebook->dialog(action, &params2[0]);
    }
    
    return 0;
}

static int dialog(lua_State *L, const char* action)
{
    GFacebook *facebook = getInstance(L, 1);
    
    if (lua_isnoneornil(L, 2))
    {
        facebook->dialog(action, NULL);
    }
    else
    {
        std::map<std::string, std::string> params = tableToMap(L, 2);

        std::vector<gfacebook_Parameter> params2;
        
        std::map<std::string, std::string>::iterator iter, e = params.end();
        for (iter = params.begin(); iter != e; ++iter)
        {
            gfacebook_Parameter param = {iter->first.c_str(), iter->second.c_str()};
            params2.push_back(param);
        }
        gfacebook_Parameter param = {NULL, NULL};
        params2.push_back(param);
        
        facebook->dialog(action, &params2[0]);
    }
    
    return 0;
}

static void graphRequest(lua_State *L, int method)
{
    GFacebook *facebook = getInstance(L, 1);
    
    const char *graphPath = luaL_checkstring(L, 2);
    
    if (lua_isnoneornil(L, 3))
    {
        facebook->request(graphPath, NULL, method);
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
        
		facebook->request(graphPath, &params2[0], method);
    }
}

static void graphRequest(lua_State *L, const char* graphPath, int method)
{
    GFacebook *facebook = getInstance(L, 1);
    
    if (lua_isnoneornil(L, 2))
    {
        facebook->request(graphPath, NULL, method);
    }
    else
    {
        std::map<std::string, std::string> params = lua_isnoneornil(L, 2) ? std::map<std::string, std::string>() : tableToMap(L, 2);

        std::vector<gfacebook_Parameter> params2;
        
        std::map<std::string, std::string>::iterator iter, e = params.end();
        for (iter = params.begin(); iter != e; ++iter)
        {
            gfacebook_Parameter param = {iter->first.c_str(), iter->second.c_str()};
            params2.push_back(param);
        }
        gfacebook_Parameter param = {NULL, NULL};
        params2.push_back(param);
        
		facebook->request(graphPath, &params2[0], method);
    }
}

static int request(lua_State *L){
	graphRequest(L, HTTP_GET);
    return 0;
}

static int post(lua_State *L){
	graphRequest(L, HTTP_POST);
    return 0;
}

static int deleteRequest(lua_State *L){
	graphRequest(L, HTTP_DELETE);
    return 0;
}

static int getProfile(lua_State *L){
	graphRequest(L, "me", HTTP_GET);
    return 0;
}

static int getFriends(lua_State *L){
	graphRequest(L, "me/friends", HTTP_GET);
    return 0;
}

static int getAlbums(lua_State *L){
	graphRequest(L, "me/albums", HTTP_GET);
    return 0;
}

static int getAppRequests(lua_State *L){
	graphRequest(L, "me/apprequests", HTTP_GET);
    return 0;
}

static int getScores(lua_State *L){
	GFacebook *facebook = getInstance(L, 1);
	std::string c = std::string(facebook->getAppId()) + "/scores";
	graphRequest(L, c.c_str(), HTTP_GET);
    return 0;
}

static int postScore(lua_State *L){
	graphRequest(L, "me/scores", HTTP_POST);
    return 0;
}

static int postPhoto(lua_State *L){
	//graphRequest(L, "me/photos", HTTP_POST);
	
	GFacebook *facebook = getInstance(L, 1);
    
    const char *graphPath = g_pathForFile(luaL_checkstring(L, 2));
    
    if (lua_isnoneornil(L, 3))
    {
		std::vector<gfacebook_Parameter> params2;
		
		gfacebook_Parameter parama = {"path", graphPath};
        params2.push_back(parama);
		
        gfacebook_Parameter param = {NULL, NULL};
        params2.push_back(param);
		
        facebook->request("me/photos", &params2[0], HTTP_POST);
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
		gfacebook_Parameter parama = {"path", graphPath};
        params2.push_back(parama);
		
        gfacebook_Parameter param = {NULL, NULL};
        params2.push_back(param);
        
		facebook->request("me/photos", &params2[0], HTTP_POST);
    }
    return 0;
}

static int postToFeed(lua_State *L){
	graphRequest(L, "me/feed", HTTP_POST);
    return 0;
}

static int inviteFriends(lua_State *L){
	dialog(L, "apprequests");
    return 0;
}

static int share(lua_State *L){
	dialog(L, "feed");
    return 0;
}

static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
        {"login", login},
        {"logout", logout},
        {"getAccessToken", getAccessToken},
        {"getExpirationDate", getExpirationDate},
        {"dialog", dialog},
        {"get", request},
        {"post", post},
        {"delete", deleteRequest},
		{"getProfile", getProfile},
		{"getFriends", getFriends},
		{"getAlbums", getAlbums},
		{"getAppRequests", getAppRequests},
		{"getScores", getScores},
		{"postScore", postScore},
		{"postPhoto", postPhoto},
		{"postToFeed", postToFeed},
		{"inviteFriends", inviteFriends},
		{"share", share},
		{"upload", upload},
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
    lua_pushstring(L, LOGOUT_COMPLETE);
	lua_setfield(L, -2, "LOGOUT_COMPLETE");
	lua_pushstring(L, LOGOUT_ERROR);
	lua_setfield(L, -2, "LOGOUT_ERROR");
	lua_pushstring(L, OPEN_URL);
	lua_setfield(L, -2, "OPEN_URL");
    lua_pushstring(L, DIALOG_COMPLETE);
	lua_setfield(L, -2, "DIALOG_COMPLETE");
    lua_pushstring(L, DIALOG_ERROR);
	lua_setfield(L, -2, "DIALOG_ERROR");
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

