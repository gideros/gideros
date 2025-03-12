#include "gideros.h"
#include "lua.hpp"
#include <lauxlib.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <gevent.h>
#include <stdlib.h>
static lua_State* luaState = NULL;

extern "C" EMSCRIPTEN_KEEPALIVE void JSNative_enqueueEvent(const char *type,int context, int value, const char *data, int datasize,const char *meta);

static g_id gid_;

struct JSNative_Event {
	char *eventType;
	char *eventMeta;
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

	const char *str=luaL_checkstring(L,1);
	intptr_t a[8];
	for (int k=0;k<8;k++) {
		intptr_t p=0;
		if (lua_type(L,(2+k))==LUA_TNUMBER)
			p=luaL_optinteger(L,2+k,0);
		else {
			const char *s=luaL_optstring(L,2+k,NULL);
			p=(intptr_t)s;
		}
		a[k]=p;
	}

	char *ret=(char *) EM_ASM_PTR({
		Module.GiderosJSArgs=Array( $1, $2, $3, $4, $5, $6, $7, $8);
		var r=stringToNewUTF8(String(eval(UTF8ToString($0))));
		Module.GiderosJSArgs=null;
		return r;
	},str,a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);

	lua_pushstring(L,ret);
	free(ret);

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
	lua_pushstring(L, e->eventMeta);
	lua_setfield(L, -2, "meta");
	lua_pushlstring(L, e->eventData,e->eventDataSize);
	lua_setfield(L, -2, "data");
	lua_pushinteger(L, e->eventValue);
	lua_setfield(L, -2, "value");
	lua_pushinteger(L, e->eventContext);
	lua_setfield(L, -2, "context");
	lua_call(L, 2, 0);
	lua_pop(L,1);
}

extern "C" void JSNative_enqueueEvent(const char *type,int context, int value, const char *data, int datasize,const char *meta)
{
	if (datasize==-1) datasize=strlen(data);
	struct JSNative_Event *event = (struct JSNative_Event *)malloc(sizeof(struct JSNative_Event)+strlen(type)+1+strlen(meta)+1+datasize);
	event->eventType=(char *)(event+1);
	event->eventMeta=event->eventType+strlen(type)+1;
	event->eventData=event->eventMeta+strlen(meta)+1;
	event->eventDataSize=datasize;
	event->eventContext=context;
	event->eventValue=value;
	strcpy(event->eventType,type);
	strcpy(event->eventMeta,meta);
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
