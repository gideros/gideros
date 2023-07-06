#include "gideros.h"
#include "lua.hpp"
#include <lauxlib.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <gevent.h>
#include <stdlib.h>
static lua_State* luaState = NULL;

extern "C" EMSCRIPTEN_KEEPALIVE void JSNative_enqueueEvent(const char *type,int context, int value, const char *data, int datasize);

static g_id gid_;

struct JSNative_Event {
	char *eventType;
	char *eventData;
	int eventDataSize;
	int eventValue;
	int eventContext;
};

class JSNative : public GEventDispatcherProxy
{
public:
    JSNative()
    {
    }

    ~JSNative()
    {
    }
};

static int JSNative_eval(lua_State *L) {

	const char *str=luaL_checkstring(L,-1);

	char *ret=(char *) EM_ASM_INT({
	 return allocate(intArrayFromString(String(eval(UTF8ToString($0)))), ALLOC_STACK);
	},str);

	lua_pushstring(L,ret);

	return 1;
}

static void JSNative_callback(int type, void *event, void *udata)
{
	lua_State* L = luaState;
	if (L == NULL)
		return;
	lua_getglobal(L, "JS");// instance
	lua_getfield(L, -1, "dispatchEvent");
	lua_pushvalue(L, -2);

	lua_getglobal(L, "Event");
	lua_getfield(L, -1, "new");
	lua_remove(L, -2);

	struct JSNative_Event *e=(struct JSNative_Event *)event;
	lua_pushstring(L, e->eventType);
	lua_call(L, 1, 1);
	lua_pushlstring(L, e->eventData,e->eventDataSize);
	lua_setfield(L, -2, "data");
	lua_pushinteger(L, e->eventValue);
	lua_setfield(L, -2, "value");
	lua_pushinteger(L, e->eventContext);
	lua_setfield(L, -2, "context");
	lua_call(L, 2, 0);
	lua_pop(L,1);
}

extern "C" void JSNative_enqueueEvent(const char *type,int context, int value, const char *data, int datasize)
{
	if (datasize==-1) datasize=strlen(data);
	struct JSNative_Event *event = (struct JSNative_Event *)malloc(sizeof(struct JSNative_Event)+strlen(type)+1+datasize);
	event->eventType=(char *)(event+1);
	event->eventData=event->eventType+strlen(type)+1;
	event->eventDataSize=datasize;
	event->eventContext=context;
	event->eventValue=value;
	strcpy(event->eventType,type);
	memcpy(event->eventData,data,datasize);
	gevent_EnqueueEvent(gid_, JSNative_callback, 0, event, 1, NULL);
}

static void g_initializePlugin(lua_State *L) {
	luaState = L;
	gid_ = g_NextId();
	luaL_Reg reg[] = { { "eval", JSNative_eval }, { NULL, NULL } };

	g_createClass(L, "JSNative", "EventDispatcher", NULL, NULL, reg);
	JSNative *r = new JSNative();
    g_pushInstance(L, "JSNative", r->object());
	lua_setglobal(L, "JS");
}

static void g_deinitializePlugin(lua_State *L) {
}

REGISTER_PLUGIN_NAMED("JSNative", "1.0", JSNative)
