#include "gideros.h"
#include "lua.hpp"
#include <lauxlib.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include "webxr/webxr.h"
#include <gevent.h>
#include <stdlib.h>
#include "luaapplication.h"
#include "luautil.h"
#include "binder.h"
static lua_State* luaState = NULL;
static LuaApplication *application_=NULL;

static g_id gid_;
bool inWebXR=false;

struct WebXR_Event {
	char *eventType;
	char *eventData;
	int eventDataSize;
	int eventValue;
	int eventContext;
};

struct {
	WebXRSessionMode mode;
	WebXRSessionFeatures features;
	WebXRSessionFeatures options;
	bool request;
} webxr_status;

#define WEBXR_STARTED		0
#define WEBXR_STOPPED		1
#define WEBXR_ERROR			2
#define WEBXR_SELECT		3
#define WEBXR_SELECT_START	4
#define WEBXR_SELECT_END	5

void WebXR_EventTrigger()
{
	if (webxr_status.request) {
		webxr_status.request=false;
		webxr_request_session(webxr_status.mode,webxr_status.features,webxr_status.options);
	}
}

static void WebXR_callback(int type, void *event, void *udata)
{
	lua_State* L = luaState;
	if (L == NULL)
		return;
	lua_getglobal(L, "webxr");// instance
	lua_getfield(L, -1, "dispatchEvent");
	lua_pushvalue(L, -2);

	lua_getglobal(L, "Event");
	lua_getfield(L, -1, "new");
	lua_remove(L, -2);

	switch (type) {
	case WEBXR_STARTED: lua_pushstring(L, "webxrStarted"); break;
	case WEBXR_STOPPED: lua_pushstring(L, "webxrStopped"); break;
	case WEBXR_ERROR: lua_pushstring(L, "webxrError"); break;
	case WEBXR_SELECT: lua_pushstring(L, "webxrSelect"); break;
	case WEBXR_SELECT_START: lua_pushstring(L, "webxrSelectStart"); break;
	case WEBXR_SELECT_END: lua_pushstring(L, "webxrSelectEnd"); break;
	}
	lua_call(L, 1, 1);
	switch (type) {
	case WEBXR_STARTED:
	case WEBXR_STOPPED:
		lua_pushnumber(L,(intptr_t)(event));
		lua_setfield(L, -2, "mode");
		break;
	case WEBXR_ERROR:
		lua_pushnumber(L,(intptr_t)(event));
		lua_setfield(L, -2, "error");
		break;
	case WEBXR_SELECT:
	case WEBXR_SELECT_START:
	case WEBXR_SELECT_END:
		lua_pushnumber(L,(intptr_t)(event));
		lua_setfield(L, -2, "hand");
		break;
	}
	lua_call(L, 2, 0);
	lua_pop(L,1);
}


static void lua_pushvector4(lua_State *L,float x,float y,float z,float w)
{
#if LUA_VECTOR_SIZE == 4
	lua_pushvector(L,x,y,z,w);
#else
	lua_createtable(L,4,0);
	lua_pushnumber(L,x); lua_rawseti(L,-2,1);
	lua_pushnumber(L,y); lua_rawseti(L,-2,2);
	lua_pushnumber(L,z); lua_rawseti(L,-2,3);
	lua_pushnumber(L,w); lua_rawseti(L,-2,4);
#endif
}

static void lua_pushvector3(lua_State *L,float x,float y,float z)
{
#if LUA_VECTOR_SIZE == 4
	lua_pushvector(L,x,y,z,0);
#else
	lua_pushvector(L,x,y,z);
#endif
}

extern void looptick(void *a);
class WebXR : public GEventDispatcherProxy
{
public:
	WebXRRigidTransform headPose;
	WebXR()
    {
  	  webxr_init(
  	          /* Frame callback */
  	          [](void* userData, int, WebXRRigidTransform* head, WebXRView* views,int vcount) {
  	              static_cast<WebXR*>(userData)->drawFrame(head,views,vcount);
  	          },
  	          /* Session end callback */
  	          [](void* userData,int mode) {
  	              static_cast<WebXR*>(userData)->sessionStart(mode);
  	          },
  	          /* Session end callback */
  	          [](void* userData,int mode) {
  	              static_cast<WebXR*>(userData)->sessionEnd(mode);
  	          },
  	          /* Error callback */
  	          [](void* userData, int error) {
  	              static_cast<WebXR*>(userData)->onError(error);
  	          },
  	          /* userData */
  	          this);

  	  webxr_set_select_callback([](WebXRInputSource* inputSource,void* userData) {
            static_cast<WebXR*>(userData)->select(inputSource);
      },this);
  	  webxr_set_select_start_callback([](WebXRInputSource* inputSource,void* userData) {
            static_cast<WebXR*>(userData)->selectStart(inputSource);
      },this);
  	  webxr_set_select_end_callback([](WebXRInputSource* inputSource,void* userData) {
            static_cast<WebXR*>(userData)->selectEnd(inputSource);
      },this);
    }

    ~WebXR()
    {
    }

    void select(WebXRInputSource *source) {
    	gevent_EnqueueEvent(gid_, WebXR_callback, WEBXR_SELECT, (void*)source->handedness, 0, nullptr);
    }

    void selectStart(WebXRInputSource *source) {
    	gevent_EnqueueEvent(gid_, WebXR_callback, WEBXR_SELECT_START, (void*)source->handedness, 0, nullptr);
    }

    void selectEnd(WebXRInputSource *source) {
    	gevent_EnqueueEvent(gid_, WebXR_callback, WEBXR_SELECT_END, (void*)source->handedness, 0, nullptr);
    }

    void drawFrame(WebXRRigidTransform* head,WebXRView *views,int vcount)
    {
    	if (head)
    		headPose=*head;
    	if (application_) {
    		looptick(NULL);
    		lua_State* L = luaState;
			if (!L) return;
		    lua_getglobal(L, "webxr");
		    lua_getfield(L,-1, "inputEventHandler");
		    if (lua_isfunction(L,-1)) {
				WebXRInputSource sources[5];
				int sourcesCount = 0;
				webxr_get_input_sources(sources, 5, &sourcesCount);
				for(int i = 0; i < sourcesCount; ++i) {
					WebXRRigidTransform ctl;
					webxr_get_input_pose(sources+i, &ctl);

					lua_pushvalue(L, -1);
					lua_newtable(L);
					lua_pushinteger(L,sources[i].handedness);
					lua_setfield(L, -2, "hand");

					//Pose
					lua_pushvector3(L,ctl.position[0],ctl.position[1],ctl.position[2]);
					lua_setfield(L, -2, "position");
					lua_pushvector4(L,ctl.orientation[0],ctl.orientation[1],ctl.orientation[2],ctl.orientation[3]);
					lua_setfield(L, -2, "rotation");
					/*
					pushVector(L,input.velPos);
					lua_setfield(L, -2, "linearVelocity");
					pushVector(L,input.velRot);
					lua_setfield(L, -2, "angularVelocity");
					pushVector(L,input.accPos);
					lua_setfield(L, -2, "linearAcceleration");
					pushVector(L,input.accRot);
					lua_setfield(L, -2, "angularAcceleration");
					*/

					lua_call(L, 1, 0);
				}
   		    }
	    	lua_pop(L,2);
			application_->clearBuffers();
    		for (int k=0;k<vcount;k++) {
    			Matrix4 vmat(views[k].viewPose.matrix);
    			vmat.invert();
				application_->renderScene((k==0)?1:0,vmat.raw(),views[k].projectionMatrix,[=](ShaderEngine *gfx,Matrix4 &xform)
						{
							gfx->setViewport(views[k].viewport[0],views[k].viewport[1],views[k].viewport[2],views[k].viewport[3]);
						});
    		}
    	}
    }

    void sessionStart(int mode)
    {
    	gevent_EnqueueEvent(gid_, WebXR_callback, WEBXR_STARTED, (void*)mode, 0, nullptr);
    	inWebXR=true;
    }

    void sessionEnd(int mode)
    {
    	gevent_EnqueueEvent(gid_, WebXR_callback, WEBXR_STOPPED, (void*)mode, 0, nullptr);
    	inWebXR=false;
    }

    void onError(int error)
    {
    	gevent_EnqueueEvent(gid_, WebXR_callback, WEBXR_ERROR, (void *)error, 0, nullptr);
    }

};

static int WebXR_start(lua_State *L) {
	webxr_status.mode=(WebXRSessionMode)luaL_checkinteger(L,2);
	webxr_status.features=(WebXRSessionFeatures)luaL_optinteger(L,3,0);
	webxr_status.options=(WebXRSessionFeatures)luaL_optinteger(L,4,0);
	webxr_status.request=true;
	return 0;
}

static int WebXR_stop(lua_State *L) {
	webxr_status.request=false;
	webxr_request_exit();
	return 0;
}

static int WebXR_getHeadPose(lua_State *L) {
    Binder binder(L);

    WebXR *r = static_cast<WebXR*>(binder.getInstance("WebXR", 1));
	lua_newtable(L);
	//Pose
	lua_pushvector3(L,r->headPose.position[0],r->headPose.position[1],r->headPose.position[2]);
	lua_setfield(L, -2, "position");
	lua_pushvector4(L,r->headPose.orientation[0],r->headPose.orientation[1],r->headPose.orientation[2],r->headPose.orientation[3]);
	lua_setfield(L, -2, "rotation");
	return 1;
}

static void g_initializePlugin(lua_State *L) {
	luaState = L;
	gid_ = g_NextId();
	luaL_Reg reg[] = {
			{ "start", WebXR_start },
			{ "stop", WebXR_stop },
			{ "getHeadPose", WebXR_getHeadPose },
			{ NULL, NULL } };

	g_createClass(L, "WebXR", "EventDispatcher", NULL, NULL, reg);
	WebXR *r = new WebXR();
    g_pushInstance(L, "WebXR", r);
	lua_setglobal(L, "webxr");

	lua_getglobal(L,"WebXR");
    lua_pushstring(L, "webxrStarted"); lua_setfield(L,-2,"EVENT_STARTED");
	lua_pushstring(L, "webxrStopped"); lua_setfield(L,-2,"EVENT_STOPPED");
	lua_pushstring(L, "webxrError"); lua_setfield(L,-2,"EVENT_ERROR");
	lua_pushstring(L, "webxrSelect"); lua_setfield(L,-2,"EVENT_SELECT");
	lua_pushstring(L, "webxrSelectStart"); lua_setfield(L,-2,"EVENT_SELECT_START");
	lua_pushstring(L, "webxrSelectEnd"); lua_setfield(L,-2,"EVENT_SELECT_END");
	lua_pop(L,1);

	application_=(LuaApplication *)luaL_getdata(L);
}

static void g_deinitializePlugin(lua_State *L) {
	if (inWebXR) {
		webxr_request_exit();
	}
	application_=NULL;
}

REGISTER_PLUGIN_NAMED("WebXR", "1.0", WebXR)
