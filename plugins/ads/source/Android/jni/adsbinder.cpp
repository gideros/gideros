#include "ads.h"
#include "gideros.h"
#include <glog.h>
#include <map>
#include <string>

// some Lua helper functions
#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif

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

static float getLogicalScaleX(lua_State *L)
{
	lua_getglobal(L, "application");
	lua_getfield(L, -1, "getLogicalScaleX");
	lua_pushvalue(L, -2);
	lua_call(L, 1, 1);
	float scale = lua_tonumber(L, -1);
	lua_remove(L, -1);
	lua_remove(L, -1);
	return scale;
}

static float getLogicalScaleY(lua_State *L)
{
	lua_getglobal(L, "application");
	lua_getfield(L, -1, "getLogicalScaleY");
	lua_pushvalue(L, -2);
	lua_call(L, 1, 1);
	float scale = lua_tonumber(L, -1);
	lua_remove(L, -1);
	lua_remove(L, -1);
	return scale;
}

static float getLogicalTranslateX(lua_State *L)
{
	lua_getglobal(L, "application");
	lua_getfield(L, -1, "getLogicalTranslateX");
	lua_pushvalue(L, -2);
	lua_call(L, 1, 1);
	float scale = lua_tonumber(L, -1);
	lua_remove(L, -1);
	lua_remove(L, -1);
	return scale;
}

static float getLogicalTranslateY(lua_State *L)
{
	lua_getglobal(L, "application");
	lua_getfield(L, -1, "getLogicalTranslateY");
	lua_pushvalue(L, -2);
	lua_call(L, 1, 1);
	float scale = lua_tonumber(L, -1);
	lua_remove(L, -1);
	lua_remove(L, -1);
	return scale;
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

static const char *AD_RECEIVED = "adReceived";
static const char *AD_FAILED = "adFailed";
static const char *AD_ACTION_BEGIN = "adActionBegin";
static const char *AD_ACTION_END = "adActionEnd";
static const char *AD_DISMISSED = "adDismissed";
static const char *AD_DISPLAYED = "adDisplayed";
static const char *AD_ERROR = "adError";

static char keyWeak = ' ';

class Ads : public GEventDispatcherProxy
{
public:
    Ads(lua_State *L, const char *ad)
    {
		ad_ = strdup(ad);
        gads_initialize(ad);
		gads_addCallback(callback_s, this);
    }
    
    ~Ads()
    {
		gads_destroy(ad_);
		gads_removeCallback(callback_s, this);
		free((char*)ad_);
    }
	
	void setKey(gads_Parameter *params)
	{
		gads_setKey(ad_, params);
	}
	
	void loadAd(gads_Parameter *params)
	{
		gads_loadAd(ad_, params);
	}
	
	void showAd(gads_Parameter *params)
	{
		gads_showAd(ad_, params);
	}
	
	void hideAd(const char *type)
	{
		gads_hideAd(ad_, type);
	}
	
	void enableTesting()
	{
		gads_enableTesting(ad_);
	}
	
	void setAlignment(const char *hor, const char *ver)
	{
		gads_setAlignment(ad_, hor, ver);
	}
	
	void setX(int x)
	{
		gads_setX(ad_, x);
	}
	
	void setY(int y)
	{
		gads_setY(ad_, y);
	}
	
	int getX()
	{
		return gads_getX(ad_);
	}
	
	int getY()
	{
		return gads_getY(ad_);
	}
	
	int getWidth()
	{
		return gads_getWidth(ad_);
	}
	
	int getHeight()
	{
		return gads_getHeight(ad_);
	}
    
private:
	static void callback_s(int type, void *event, void *udata)
	{
		static_cast<Ads*>(udata)->callback(type, event);
	}
	
	void callback(int type, void *event)
	{
        dispatchEvent(type, event);
	}
	
	void dispatchEvent(int type, void *event)
	{
		if(L != NULL)
		{
			int shouldDispatch = 0;
			if (type == GADS_AD_ERROR_EVENT)
			{
				gads_AdErrorEvent *event2 = (gads_AdErrorEvent*)event;
				if(strcmp(event2->ad, ad_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else if (type == GADS_AD_FAILED_EVENT)
			{
				gads_AdFailedEvent *event2 = (gads_AdFailedEvent*)event;
				if(strcmp(event2->ad, ad_) == 0)
				{
					shouldDispatch = 1;
				}
			}
			else
			{
				gads_SimpleEvent *event2 = (gads_SimpleEvent*)event;
				if(strcmp(event2->ad, ad_) == 0)
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
					case GADS_AD_RECEIVED_EVENT:
						lua_pushstring(L, AD_RECEIVED);
						break;
					case GADS_AD_FAILED_EVENT:
						lua_pushstring(L, AD_FAILED);
						break;
					case GADS_AD_ACTION_BEGIN_EVENT:
						lua_pushstring(L, AD_ACTION_BEGIN);
						break;
					case GADS_AD_ACTION_END_EVENT:
						lua_pushstring(L, AD_ACTION_END);
						break;
					case GADS_AD_DISMISSED_EVENT:
						lua_pushstring(L, AD_DISMISSED);
						break;
                    case GADS_AD_DISPLAYED_EVENT:
						lua_pushstring(L, AD_DISPLAYED);
						break;
					case GADS_AD_ERROR_EVENT:
						lua_pushstring(L, AD_ERROR);
						break;
				}
                
				lua_call(L, 1, 1);
				
				if (type == GADS_AD_FAILED_EVENT)
				{
					gads_AdFailedEvent *event2 = (gads_AdFailedEvent*)event;
					
					lua_pushstring(L, event2->type);
					lua_setfield(L, -2, "type");
					
					lua_pushstring(L, event2->error);
					lua_setfield(L, -2, "error");
				}
				else if (type == GADS_AD_ERROR_EVENT)
				{
					gads_AdErrorEvent *event2 = (gads_AdErrorEvent*)event;
					
					lua_pushstring(L, event2->error);
					lua_setfield(L, -2, "error");
				}
				else
				{
					gads_SimpleEvent *event2 = (gads_SimpleEvent*)event;
					
					lua_pushstring(L, event2->type);
					lua_setfield(L, -2, "type");
				}
                
				lua_call(L, 2, 0);
				
				lua_pop(L, 2);
			}
		}
	}
    
private:
	const char* ad_;
};

static int destruct(lua_State* L)
{
	void *ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	Ads *ads = static_cast<Ads*>(object->proxy());
	
	ads->unref();
	
	return 0;
}

static Ads *getInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "Ads", index));
	Ads *ads = static_cast<Ads*>(object->proxy());
    
	return ads;
}

static int init(lua_State *L)
{
    
    const char *ad = luaL_checkstring(L, 1);
	Ads *ads = new Ads(L, ad);
	g_pushInstance(L, "Ads", ads->object());
    
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, ads);
	lua_pop(L, 1);
    
	lua_pushvalue(L, -1);
    return 1;
}

static int setKey(lua_State *L)
{
    Ads *ads = getInstance(L, 1);
	int i = 2;
	std::vector<gads_Parameter> params2;
	while(!lua_isnoneornil(L, i))
	{
		gads_Parameter param = {luaL_checkstring(L, i)};
		params2.push_back(param);
		i++;
	}
	gads_Parameter param = {NULL};
	params2.push_back(param);
	ads->setKey(&params2[0]);
    return 0;
}

static int loadAd(lua_State *L)
{
	Ads *ads = getInstance(L, 1);
	int i = 2;
	std::vector<gads_Parameter> params2;
	while(!lua_isnoneornil(L, i))
	{
		gads_Parameter param = {luaL_checkstring(L, i)};
		params2.push_back(param);
		i++;
	}
	gads_Parameter param = {NULL};
	params2.push_back(param);
	ads->loadAd(&params2[0]);
    return 0;
}

static int showAd(lua_State *L)
{
	Ads *ads = getInstance(L, 1);
	int i = 2;
	std::vector<gads_Parameter> params2;
	while(!lua_isnoneornil(L, i))
	{
		gads_Parameter param = {luaL_checkstring(L, i)};
		params2.push_back(param);
		i++;
	}
	gads_Parameter param = {NULL};
	params2.push_back(param);
	ads->showAd(&params2[0]);
    return 0;
}

static int hideAd(lua_State *L)
{
    Ads *ads = getInstance(L, 1);
	const char *type = "";
	if(lua_isnoneornil(L, 2) == 0)
		type = lua_tostring(L, 2);
    ads->hideAd(type);
    return 0;
}

static int enableTesting(lua_State *L)
{
    Ads *ads = getInstance(L, 1);
    ads->enableTesting();
    return 0;
}

static int setAlignment(lua_State *L)
{
    Ads *ads = getInstance(L, 1);
    const char *hor = luaL_checkstring(L, 2);
    const char *ver = luaL_checkstring(L, 3);
    ads->setAlignment(hor, ver);
    return 0;
}

static int setPosition(lua_State *L)
{
    Ads *ads = getInstance(L, 1);
    int x = lua_tonumber(L, 2)*getLogicalScaleX(L)+getLogicalTranslateX(L);
    int y = lua_tonumber(L, 3)*getLogicalScaleY(L)+getLogicalTranslateY(L);
    ads->setX(x);
    ads->setY(y);
    return 0;
}

static int setX(lua_State *L)
{
    Ads *ads = getInstance(L, 1);
    int x = lua_tonumber(L, 2)*getLogicalScaleX(L)+getLogicalTranslateX(L);
	ads->setX(x);
    return 0;
}

static int setY(lua_State *L)
{
    Ads *ads = getInstance(L, 1);
    int y = lua_tonumber(L, 2)*getLogicalScaleY(L)+getLogicalTranslateY(L);
	ads->setY(y);
    return 0;
}

static int set(lua_State *L)
{
    Ads *ads = getInstance(L, 1);
	const char *key = luaL_checkstring(L, 2);
    int value = lua_tonumber(L, 3);
	if(strcmp(key, "x") == 0)
	{
		ads->setX(value*getLogicalScaleX(L)+getLogicalTranslateX(L));
	}
	else if(strcmp(key, "y") == 0)
	{
		ads->setY(value*getLogicalScaleY(L)+getLogicalTranslateY(L));
	}
    return 0;
}


static int getPosition(lua_State *L)
{
    Ads *ads = getInstance(L, 1);
    int x = (ads->getX() - getLogicalTranslateX(L))/getLogicalScaleX(L);
    int y = (ads->getY() - getLogicalTranslateY(L))/getLogicalScaleY(L);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
    return 2;
}

static int get(lua_State *L)
{
    Ads *ads = getInstance(L, 1);
	const char *key = luaL_checkstring(L, 2);
	if(strcmp(key, "x") == 0)
	{
		int x = (ads->getX() - getLogicalTranslateX(L))/getLogicalScaleX(L);
		lua_pushnumber(L, x);
	}
	else if(strcmp(key, "y") == 0)
	{
		int y = (ads->getY() - getLogicalTranslateY(L))/getLogicalScaleY(L);
		lua_pushnumber(L, y);
	}
    return 1;
}

static int getX(lua_State *L)
{
    Ads *ads = getInstance(L, 1);
    int x = (ads->getX() - getLogicalTranslateX(L))/getLogicalScaleX(L);
	lua_pushnumber(L, x);
    return 1;
}

static int getY(lua_State *L)
{
    Ads *ads = getInstance(L, 1);
    int y = (ads->getY() - getLogicalTranslateY(L))/getLogicalScaleY(L);
	lua_pushnumber(L, y);
    return 1;
}

static int getWidth(lua_State *L)
{
    Ads *ads = getInstance(L, 1);
    int width = ads->getWidth()/getLogicalScaleX(L);
	lua_pushnumber(L, width);
    return 1;
}

static int getHeight(lua_State *L)
{
    Ads *ads = getInstance(L, 1);
    int height = ads->getHeight()/getLogicalScaleY(L);
	lua_pushnumber(L, height);
    return 1;
}

static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
        {"new", init},
        {"setKey", setKey},
        {"loadAd", loadAd},
        {"showAd", showAd},
        {"hideAd", hideAd},
        {"enableTesting", enableTesting},
        {"setAlignment", setAlignment},
        {"setPosition", setPosition},
        {"getPosition", getPosition},
        {"getX", getX},
        {"getY", getY},
        {"setX", setX},
        {"setY", setY},
        {"getWidth", getWidth},
        {"getHeight", getHeight},
		{"set", set},
		{"get", get},
		{NULL, NULL},
	};
    
    g_createClass(L, "Ads", "EventDispatcher", NULL, destruct, functionlist);
    
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    
	lua_getglobal(L, "Event");
	lua_pushstring(L, AD_RECEIVED);
	lua_setfield(L, -2, "AD_RECEIVED");
	lua_pushstring(L, AD_FAILED);
	lua_setfield(L, -2, "AD_FAILED");
	lua_pushstring(L, AD_ACTION_BEGIN);
	lua_setfield(L, -2, "AD_ACTION_BEGIN");
	lua_pushstring(L, AD_ACTION_END);
	lua_setfield(L, -2, "AD_ACTION_END");
	lua_pushstring(L, AD_DISMISSED);
	lua_setfield(L, -2, "AD_DISMISSED");
    lua_pushstring(L, AD_DISPLAYED);
	lua_setfield(L, -2, "AD_DISPLAYED");
	lua_pushstring(L, AD_ERROR);
	lua_setfield(L, -2, "AD_ERROR");
	lua_pop(L, 1);
    
    return 0;
}

static void g_initializePlugin(lua_State *L)
{
	::L = L;
    lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "ads");
	
	lua_pop(L, 2);
	
	gads_init();
}

static void g_deinitializePlugin(lua_State *L)
{
	::L = NULL;
    gads_cleanup();
}

REGISTER_PLUGIN("Ads", "1.0")
