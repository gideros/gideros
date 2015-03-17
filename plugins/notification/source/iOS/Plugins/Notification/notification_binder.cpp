#include "notification_wrapper.h"
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

static char keyWeak = ' ';

static const char *LOCAL_NOTIFICATION = "localNotification";
static const char *PUSH_NOTIFICATION = "pushNotification";
static const char *PUSH_REGISTRATION = "pushRegistration";
static const char *PUSH_REGISTRATION_ERROR = "pushRegistrationError";

class NotificationManager : public GEventDispatcherProxy
{
public:
    NotificationManager()
    {
		gnotification_addCallback(callback_s, this);
    }
    
    ~NotificationManager()
    {
		gnotification_removeCallback(callback_s, this);
    }
	
	void clearLocalNotifications()
	{
		gnotification_clear_local();
	}
	
	void clearPushNotifications()
	{
		gnotification_clear_push();
	}
	
	void cancelAll()
	{
		gnotification_cancel_all();
	}
	
	void cancel(int id)
	{
		gnotification_cancel(id);
	}
	
	gnotification_Group* getScheduledNotifications()
	{
		return gnotification_get_scheduled();
	}
	
	gnotification_Group* getLocalNotifications()
	{
		return gnotification_get_local();
	}
	
	gnotification_Group* getPushNotifications()
	{
		return gnotification_get_push();
	}
	
	void registerForPushNotifications(const char *project)
	{
		gnotification_register_push(project);
	}
	
	void unregisterForPushNotifications()
	{
		gnotification_unregister_push();
	}
    
private:
    
	static void callback_s(int type, void *event, void *udata)
	{
		((NotificationManager*)udata)->callback(type, event);
	}
	
	void callback(int type, void *event)
	{
        dispatchEvent(type, event);
	}
	
	void dispatchEvent(int type, void *event)
	{
		if(L != NULL)
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
				case NOTIFICATION_LOCAL_EVENT:
					lua_pushstring(L, LOCAL_NOTIFICATION);
					break;
				case NOTIFICATION_PUSH_EVENT:
					lua_pushstring(L, PUSH_NOTIFICATION);
					break;
				case NOTIFICATION_PUSH_REGISTER_EVENT:
					lua_pushstring(L, PUSH_REGISTRATION);
					break;
				case NOTIFICATION_PUSH_REGISTER_ERROR_EVENT:
					lua_pushstring(L, PUSH_REGISTRATION_ERROR);
					break;
			}
            
			lua_call(L, 1, 1);
            
			if (type == NOTIFICATION_LOCAL_EVENT)
			{
				gnotification_LocalEvent *event2 = (gnotification_LocalEvent*)event;
				
				lua_pushnumber(L, event2->id);
				lua_setfield(L, -2, "id");
				
				lua_pushstring(L, event2->title);
				lua_setfield(L, -2, "title");
				
				lua_pushstring(L, event2->text);
				lua_setfield(L, -2, "message");
				
				lua_pushnumber(L, event2->number);
				lua_setfield(L, -2, "number");
				
				lua_pushstring(L, event2->sound);
				lua_setfield(L, -2, "sound");
				
				lua_pushstring(L, event2->custom);
				lua_setfield(L, -2, "custom");
				
				lua_pushboolean(L, event2->didOpen);
				lua_setfield(L, -2, "didOpen");
			}
			else if (type == NOTIFICATION_PUSH_EVENT)
			{
				gnotification_PushEvent *event2 = (gnotification_PushEvent*)event;
				
				lua_pushnumber(L, event2->id);
				lua_setfield(L, -2, "id");
				
				lua_pushstring(L, event2->title);
				lua_setfield(L, -2, "title");
				
				lua_pushstring(L, event2->text);
				lua_setfield(L, -2, "message");
				
				lua_pushnumber(L, event2->number);
				lua_setfield(L, -2, "number");
				
				lua_pushstring(L, event2->sound);
				lua_setfield(L, -2, "sound");
				
				lua_pushstring(L, event2->custom);
				lua_setfield(L, -2, "custom");
				
				lua_pushboolean(L, event2->didOpen);
				lua_setfield(L, -2, "didOpen");
			}
			else if (type == NOTIFICATION_PUSH_REGISTER_EVENT)
			{
				gnotification_RegisterPushEvent *event2 = (gnotification_RegisterPushEvent*)event;
				
				lua_pushstring(L, event2->regId);
				lua_setfield(L, -2, "deviceId");
			}
			else if (type == NOTIFICATION_PUSH_REGISTER_ERROR_EVENT)
			{
				gnotification_RegisterPushErrorEvent *event2 = (gnotification_RegisterPushErrorEvent*)event;
				
				lua_pushstring(L, event2->errorId);
				lua_setfield(L, -2, "error");
			}
            
			lua_call(L, 2, 0);
			
			lua_pop(L, 2);
		}
	}
    
private:
    bool initialized_;
};

static int destruct_manager(lua_State* L)
{
	void *ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	NotificationManager *mngr = static_cast<NotificationManager*>(object->proxy());
	
	mngr->unref();
	
	
	return 0;
}

static NotificationManager *getManager(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "NotificationManager", index));
	NotificationManager *mngr = static_cast<NotificationManager*>(object->proxy());
    
	return mngr;
}

NotificationManager *manager;

static int init_mngr(lua_State *L)
{
    
	g_pushInstance(L, "NotificationManager", manager->object());
    
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, manager);
	lua_pop(L, 1);
	
	lua_pushvalue(L, -1);
	return 1;
}

static int clearLocalNotifications(lua_State *L)
{
	NotificationManager *mngr = getManager(L, 1);
    
    mngr->clearLocalNotifications();
    
    return 0;
}

static int clearPushNotifications(lua_State *L)
{
	NotificationManager *mngr = getManager(L, 1);
    
    mngr->clearPushNotifications();
    
    return 0;
}

static int cancelAll(lua_State *L)
{
	NotificationManager *mngr = getManager(L, 1);
    
    mngr->cancelAll();
    
    return 0;
}

static int cancelNotification(lua_State *L)
{
	NotificationManager *mngr = getManager(L, 1);
    int id = luaL_checkinteger(L, 2);
    mngr->cancel(id);
    
    return 0;
}

void group2table(gnotification_Group *group, lua_State* L)
{
	//main table
	lua_newtable(L);
	
	if(group)
	{
		while(group->message != NULL)
		{
			//set subtable to table
			lua_pushinteger(L, group->id);
            
			//create sub table
			lua_newtable(L);
            
			//set key
			lua_pushstring(L, "id");
			lua_pushnumber(L, group->id);
			//back to table
			lua_settable(L, -3);
			
			//set key
			lua_pushstring(L, "title");
			lua_pushstring(L, group->title);
			//back to table
			lua_settable(L, -3);
			
			//set key
			lua_pushstring(L, "message");
			lua_pushstring(L, group->message);
			//back to table
			lua_settable(L, -3);
			
			//set key
			lua_pushstring(L, "number");
			lua_pushnumber(L, group->number);
			//back to table
			lua_settable(L, -3);
			
			//set key
			lua_pushstring(L, "sound");
			lua_pushstring(L, group->sound);
			//back to table
			lua_settable(L, -3);
			
			//set key
			lua_pushstring(L, "custom");
			lua_pushstring(L, group->custom);
			//back to table
			lua_settable(L, -3);
			
			//outer table
			lua_settable(L, -3);
			
			++group;
		}
	}
	lua_pushvalue(L, -1);
}

static int getScheduledNotifications(lua_State* L)
{
	NotificationManager *mngr = getManager(L, 1);
	gnotification_Group *group = mngr->getScheduledNotifications();
	group2table(group, L);
	return 1;
}

static int getLocalNotifications(lua_State* L)
{
	NotificationManager *mngr = getManager(L, 1);
	gnotification_Group *group = mngr->getLocalNotifications();
	group2table(group, L);
	return 1;
}

static int getPushNotifications(lua_State* L)
{
	NotificationManager *mngr = getManager(L, 1);
	gnotification_Group *group = mngr->getPushNotifications();
	group2table(group, L);
	return 1;
}

static int registerForPush(lua_State *L)
{
	NotificationManager *mngr = getManager(L, 1);
    const char *project = luaL_checkstring(L, 2);
    mngr->registerForPushNotifications(project);
    
    return 0;
}

static int unregisterForPush(lua_State *L)
{
	NotificationManager *mngr = getManager(L, 1);
    mngr->unregisterForPushNotifications();
    
    return 0;
}

class Notification : public GProxy
{
public:
    Notification(int id)
    {
		id_ = id;
        gnotification_init(id_);
    }
    
    ~Notification()
    {
		gnotification_cleanup(id_);
    }
	
	void setTitle(const char *title)
	{
		gnotification_set_title(id_, title);
	}
	
	const char* getTitle()
	{
		return gnotification_get_title(id_);
	}
	
	void setBody(const char *body)
	{
		gnotification_set_body(id_, body);
	}
	
	const char* getBody()
	{
		return gnotification_get_body(id_);
	}
	
	void setNumber(int number)
	{
		gnotification_set_number(id_, number);
	}
	
	int getNumber()
	{
		return gnotification_get_number(id_);
	}
	
	void setSound(const char *sound)
	{
		gnotification_set_sound(id_, sound);
	}
	
	const char* getSound()
	{
		return gnotification_get_sound(id_);
	}
	
	void setCustom(const char *custom)
	{
		gnotification_set_custom(id_, custom);
	}
	
	const char* getCustom()
	{
		return gnotification_get_custom(id_);
	}
	
	void dispatchNow()
	{
		gnotification_dispatch_now(id_);
	}
	
	void dispatchAfter(gnotification_Parameter *params1, gnotification_Parameter *params2)
	{
		gnotification_dispatch_after(id_, params1, params2);
	}
	
	void dispatchOn(gnotification_Parameter *params1, gnotification_Parameter *params2)
	{
		gnotification_dispatch_on(id_, params1, params2);
	}
	
	void cancel()
	{
		gnotification_cancel(id_);
	}
	
	int getId()
	{
		return id_;
	}
	
private:
    bool initialized_;
	int id_;
	const char* title_;
	const char* text_;
	const char* sound_;
	int number_;
};

static int destruct_notification(lua_State* L)
{
	void *ptr = *(void**)lua_touserdata(L, 1);
	GReferenced* object = static_cast<GReferenced*>(ptr);
	Notification *n = static_cast<Notification*>(object->proxy());
	
	n->unref();
	
	return 0;
}

static Notification *getInstance(lua_State* L, int index)
{
	GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "Notification", index));
	Notification *n = static_cast<Notification*>(object->proxy());
    
	return n;
}

static int init(lua_State *L)
{
	if(lua_isnumber(L, 1))
	{
		int id = luaL_checkinteger(L, 1);
		Notification *n = new Notification(id);
		g_pushInstance(L, "Notification", n->object());
        
		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
		lua_pushvalue(L, -2);
		luaL_rawsetptr(L, -2, n);
		lua_pop(L, 1);
        
		lua_pushvalue(L, -1);
		return 1;
	}
}

static int setTitle(lua_State *L)
{
    Notification *n = getInstance(L, 1);
    
    const char *title = luaL_checkstring(L, 2);
    
    n->setTitle(title);
    
    return 0;
}

static int getTitle(lua_State *L)
{
    Notification *n = getInstance(L, 1);
    
    const char *title =  n->getTitle();
	
	lua_pushstring(L, title);
    
    return 1;
}

static int setBody(lua_State *L)
{
    Notification *n = getInstance(L, 1);
    
    const char *body = luaL_checkstring(L, 2);
    
    n->setBody(body);
    
    return 0;
}

static int getBody(lua_State *L)
{
    Notification *n = getInstance(L, 1);
    
    const char *body =  n->getBody();
	
	lua_pushstring(L, body);
    
    return 1;
}

static int setSound(lua_State *L)
{
    Notification *n = getInstance(L, 1);
    
    const char *sound = luaL_checkstring(L, 2);
    
    n->setSound(sound);
    
    return 0;
}

static int getSound(lua_State *L)
{
    Notification *n = getInstance(L, 1);
    
    const char *sound =  n->getSound();
	
	lua_pushstring(L, sound);
    
    return 1;
}

static int setCustom(lua_State *L)
{
    Notification *n = getInstance(L, 1);
    
    const char *custom = luaL_checkstring(L, 2);
    
    n->setCustom(custom);
    
    return 0;
}

static int getCustom(lua_State *L)
{
    Notification *n = getInstance(L, 1);
    
    const char *custom =  n->getCustom();
	
	lua_pushstring(L, custom);
    
    return 1;
}

static int setNumber(lua_State *L)
{
    Notification *n = getInstance(L, 1);
    
    int number = luaL_checkinteger(L, 2);
    
    n->setNumber(number);
    
    return 0;
}

static int getNumber(lua_State *L)
{
    Notification *n = getInstance(L, 1);
    
    int number =  n->getNumber();
	
	lua_pushnumber(L, number);
    
    return 1;
}

static int dispatchNow(lua_State *L)
{
    Notification *n = getInstance(L, 1);
    
    n->dispatchNow();
    
    return 0;
}

static int dispatchAfter(lua_State *L)
{
    Notification *n = getInstance(L, 1);
	
	std::map<std::string, std::string> params = tableToMap(L, 2);
    
	std::vector<gnotification_Parameter> params2;
    
	std::map<std::string, std::string>::iterator iter, e = params.end();
	for (iter = params.begin(); iter != e; ++iter)
	{
		gnotification_Parameter param = {iter->first.c_str(), iter->second.c_str()};
		params2.push_back(param);
	}
	gnotification_Parameter param = {NULL, NULL};
	params2.push_back(param);
	
	if (lua_isnoneornil(L, 3))
    {
    	n->dispatchAfter(&params2[0], NULL);
	}
	else
	{
		std::map<std::string, std::string> paramsR = tableToMap(L, 2);
        
		std::vector<gnotification_Parameter> paramsR2;
        
		std::map<std::string, std::string>::iterator iter, e = paramsR.end();
		for (iter = paramsR.begin(); iter != e; ++iter)
		{
			gnotification_Parameter paramR = {iter->first.c_str(), iter->second.c_str()};
			paramsR2.push_back(paramR);
		}
		gnotification_Parameter paramR = {NULL, NULL};
		paramsR2.push_back(paramR);
		
		n->dispatchAfter(&params2[0], &paramsR2[0]);
	}
    
    return 0;
}

static int dispatchOn(lua_State *L)
{
    Notification *n = getInstance(L, 1);
    
    std::map<std::string, std::string> params = tableToMap(L, 2);
    
	std::vector<gnotification_Parameter> params2;
    
	std::map<std::string, std::string>::iterator iter, e = params.end();
	for (iter = params.begin(); iter != e; ++iter)
	{
		gnotification_Parameter param = {iter->first.c_str(), iter->second.c_str()};
		params2.push_back(param);
	}
	gnotification_Parameter param = {NULL, NULL};
	params2.push_back(param);
	
	if (lua_isnoneornil(L, 3))
    {
    	n->dispatchOn(&params2[0], NULL);
	}
	else
	{
		std::map<std::string, std::string> paramsR = tableToMap(L, 2);
        
		std::vector<gnotification_Parameter> paramsR2;
        
		std::map<std::string, std::string>::iterator iter, e = paramsR.end();
		for (iter = paramsR.begin(); iter != e; ++iter)
		{
			gnotification_Parameter paramR = {iter->first.c_str(), iter->second.c_str()};
			paramsR2.push_back(paramR);
		}
		gnotification_Parameter paramR = {NULL, NULL};
		paramsR2.push_back(paramR);
		
		n->dispatchOn(&params2[0], &paramsR2[0]);
	}
    
    return 0;
}

static int cancel(lua_State *L)
{
    Notification *n = getInstance(L, 1);
    
    n->cancel();
    
    return 0;
}

static int getId(lua_State *L)
{
    Notification *n = getInstance(L, 1);
    
    int id = n->getId();
    
	lua_pushnumber(L, id);
	
    return 1;
}

static int loader(lua_State *L)
{
	const luaL_Reg functionlist[] = {
        {"new", init},
        {"setTitle", setTitle},
        {"getTitle", getTitle},
        {"setMessage", setBody},
        {"getMessage", getBody},
        {"setSound", setSound},
        {"getSound", getSound},
		{"setCustom", setCustom},
        {"getCustom", getCustom},
        {"setNumber", setNumber},
        {"getNumber", getNumber},
        {"dispatchNow", dispatchNow},
        {"dispatchAfter", dispatchAfter},
        {"dispatchOn", dispatchOn},
        {"cancel", cancel},
        {"getId", getId},
		{NULL, NULL},
	};
	
	const luaL_Reg functionlist_mngr[] = {
        {"getSharedInstance", init_mngr},
        {"clearLocalNotifications", clearLocalNotifications},
        {"clearPushNotifications", clearPushNotifications},
        {"cancelAllNotifications", cancelAll},
        {"cancelNotification", cancelNotification},
        {"getScheduledNotifications", getScheduledNotifications},
        {"getLocalNotifications", getLocalNotifications},
        {"getPushNotifications", getPushNotifications},
        {"registerForPushNotifications", registerForPush},
        {"unregisterForPushNotifications", unregisterForPush},
		{NULL, NULL},
	};
    
    g_createClass(L, "Notification", NULL, NULL, destruct_notification, functionlist);
	g_createClass(L, "NotificationManager", "EventDispatcher", NULL, NULL, functionlist_mngr);
    
	// create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);
	
	lua_getglobal(L, "Event");
	lua_pushstring(L, LOCAL_NOTIFICATION);
	lua_setfield(L, -2, "LOCAL_NOTIFICATION");
	lua_pushstring(L, PUSH_NOTIFICATION);
	lua_setfield(L, -2, "PUSH_NOTIFICATION");
	lua_pushstring(L, PUSH_REGISTRATION);
	lua_setfield(L, -2, "PUSH_REGISTRATION");
	lua_pushstring(L, PUSH_REGISTRATION_ERROR);
	lua_setfield(L, -2, "PUSH_REGISTRATION_ERROR");
	lua_pop(L, 1);
	
	lua_getglobal(L, "Notification");
	lua_pushstring(L, "default");
	lua_setfield(L, -2, "DEFAULT_SOUND");
	lua_pop(L, 1);
    
    return 0;
}

static void g_initializePlugin(lua_State *L)
{
	::L = L;
    lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "notification");
	
	lua_pop(L, 2);
	
	gnotification_construct();
	
	manager = new NotificationManager();
}

static void g_deinitializePlugin(lua_State *L)
{
	::L = NULL;
	manager->unref();
	manager = NULL;
    gnotification_destroy();
}

REGISTER_PLUGIN("Notification", "1.0")
