#include "eventdispatcherbinder.h"
#include "eventdispatcher.h"
#include "stackchecker.h"
#include "eventvisitor.h"
#include "enterframeevent.h"
#include "mouseevent.h"
#include "touchevent.h"
#include "timerevent.h"
#include "luautil.h"
#include "keys.h"
#include "stageorientationevent.h"
#include "errorevent.h"
#include "progressevent.h"
#include "keyboardevent.h"
#include "completeevent.h"
#include "luaapplication.h"
#include <glog.h>
#include <algorithm>

#include <math.h>

EventDispatcherBinder::EventDispatcherBinder(lua_State* L)
{
	Binder binder(L);
	
	static const luaL_Reg functionList[] = {
		{"addEventListener", EventDispatcherBinder::addEventListener},
		{"removeEventListener", EventDispatcherBinder::removeEventListener},
		{"dispatchEvent", EventDispatcherBinder::dispatchEvent},
		{"hasEventListener", EventDispatcherBinder::hasEventListener},
		{NULL, NULL},
	};

	binder.createClass("EventDispatcher", NULL, create, destruct, functionList);
}

int EventDispatcherBinder::create(lua_State* L)
{
	Binder binder(L);
	EventDispatcher* eventDispatcher = new EventDispatcher;
	binder.pushInstance("EventDispatcher", eventDispatcher);

	return 1;
}

int EventDispatcherBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	EventDispatcher* eventDispatcher = static_cast<EventDispatcher*>(ptr);
	eventDispatcher->unref();

	return 0;
}

class LuaEvent : public Event
{
public:
	typedef EventType<LuaEvent> Type;

	LuaEvent(const Type& type) : Event(type.type())
	{
	}

	virtual void apply(EventVisitor* v)
	{
		v->visitOther(this, 0);
	}
};

class CppLuaBridge : public EventDispatcher
{
public:
	CppLuaBridge(lua_State* L) : L(L)
	{
	}

	virtual ~CppLuaBridge()
	{
		// note: dont do removeEventListener here, because
		// 1. there is no need
		// 2. the EventDispatcher that registers this CppLuaBridge is currently in destruction state

		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);
		if (!lua_isnil(L, -1))	// bu destruct'in envtable'in set'lenmedigi bir fonksiyondan cagirilma ihtimal var. (bi hata sonrasi olabilior)
		{
			lua_pushlightuserdata(L, this);		// key=bridge
			lua_pushnil(L);						// value=nil
			lua_rawset(L, -3);					// envtable["eventClosures"][bridge] = nil
		}
		lua_pop(L, 1);						// pop envtable["eventClosures"]
	}

	void luaEvent(LuaEvent* event);

private:
	lua_State* L;
};

class EventBinderMap : public GReferenced
{
public:
	EventBinderMap()
	{
			
	}

	virtual ~EventBinderMap();

	void push_back(int index, CppLuaBridge* bridge);
	void remove(int index, CppLuaBridge* bridge);

	const std::vector<CppLuaBridge*>& operator[](int index)
	{
		return map_[index];
	}
		
private:
	// TODO: std::vector'u std::set yap
	std::map<int, std::vector<CppLuaBridge*> > map_;
};

class EventTypeVisitor : public EventVisitor
{
public:
	virtual void visit(Event* v)
	{
		type = eEvent;
	}

	virtual void visit(EnterFrameEvent* v)
	{
		type = eEnterFrameEvent;
	}

	virtual void visit(MouseEvent* v)
	{
		type = eMouseEvent;
	}

	virtual void visit(TouchEvent* v)
	{
		type = eTouchEvent;
	}

	virtual void visit(TimerEvent* v)
	{
		type = eTimerEvent;
	}

	virtual void visit(AccelerometerEvent* v)
	{
		type = eAccelerometerEvent;
	}

	virtual void visit(StageOrientationEvent* v)
	{
		type = eStageOrientationEvent;
	}

	virtual void visit(ErrorEvent* v)
	{
		type = eErrorEvent;
	}

	virtual void visit(ProgressEvent* v)
	{
		type = eProgressEvent;
	}

	virtual void visit(KeyboardEvent* v)
	{
		type = eKeyboardEvent;
	}

    virtual void visit(CompleteEvent* v)
    {
        type = eCompleteEvent;
    }

	virtual void visitOther(Event* v, void* data)
	{
		type = eLuaEvent;
	}

	enum Type
	{
		eEvent,
		eEnterFrameEvent,
		eMouseEvent,
		eTouchEvent,
		eTimerEvent,
		eAccelerometerEvent,
		eStageOrientationEvent,
		eErrorEvent,
		eProgressEvent,
		eKeyboardEvent,
        eCompleteEvent,
		eLuaEvent,
	};

	Type type;
};

class PushEventVisitor : public EventVisitor
{
public:
	PushEventVisitor(lua_State* L, CppLuaBridge* bridge) : L(L), bridge_(bridge)
	{

	}

	virtual void visit(Event* v)
	{
		StackChecker checker(L, "visit(Event* v)", 0);

		// get closure
		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);
		lua_pushlightuserdata(L, bridge_);
		lua_rawget(L, -2);
		lua_remove(L, -2);		// remove env["eventClosures"]

		bool newTable = pushEventTable(v, "Event");

		if (newTable == true)
		{
			lua_pushstring(L, v->type()); // TODO: buna artik ihtiyac yok. direk Event'te getType() fonksiyonu var
			lua_setfield(L, -2, "type");
		}

		lua_call(L, 1, 0);
	}

	virtual void visit(EnterFrameEvent* v)
	{
		StackChecker checker(L, "visit(EnterFrameEvent* v)", 0);

		Binder binder(L);

		// get closure
		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);
		lua_pushlightuserdata(L, bridge_);
		lua_rawget(L, -2);
		lua_remove(L, -2);		// remove env["eventClosures"]

		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_EnterFrameEvent);

		lua_getfield(L, -1, "__uniqueid");

		if (lua_isnil(L, -1) || lua_tointeger(L, -1) != v->uniqueid())
		{
			lua_pop(L, 1);

			lua_pushinteger(L, v->uniqueid());
			lua_setfield(L, -2, "__uniqueid");

			binder.setInstance(-1, v);

			lua_pushstring(L, v->type()); // TODO: buna artik ihtiyac yok. direk Event'te getType() fonksiyonu var
			lua_setfield(L, -2, "type");

			lua_pushinteger(L, v->frameCount());
			lua_setfield(L, -2, "frameCount");

//			lua_pushinteger(L, v->deltaFrameCount());
//			lua_setfield(L, -2, "deltaFrameCount");

			lua_pushnumber(L, v->time());
			lua_setfield(L, -2, "time");

			lua_pushnumber(L, v->deltaTime());
			lua_setfield(L, -2, "deltaTime");

			lua_pushnumber(L, v->lastFrameRenderTime());
			lua_setfield(L, -2, "lastFrameRenderTime");
		}
		else
		{
			lua_pop(L, 1);
		}

		lua_call(L, 1, 0);
	}


/*	virtual void visit(EnterFrameEvent* v)
	{
		StackChecker checker(L, "visit(EnterFrameEvent* v)", 0);
		lua_getfield(L, LUA_ENVIRONINDEX, "bridges");
		lua_pushlightuserdata(L, bridge_);
		lua_rawget(L, -2);
		lua_remove(L, -2);		// remove env["bridges"]

		if (lua_isnil(L, -1))	// bridge is garbage collected
		{
			lua_pop(L, 1);
			return;
		}

		bool newTable = pushEventTable(v, "Event");

		lua_pushvalue(L, -2);
		lua_setfield(L, -2, "target");

		if (newTable == true)
		{
			lua_pushstring(L, v->type()); // TODO: buna artik ihtiyac yok. direk Event'te getType() fonksiyonu var
			lua_setfield(L, -2, "type");

			lua_pushinteger(L, v->frameCount());
			lua_setfield(L, -2, "frameCount");

			lua_pushinteger(L, v->deltaFrameCount());
			lua_setfield(L, -2, "deltaFrameCount");
		
			lua_pushnumber(L, v->time());
			lua_setfield(L, -2, "time");

			lua_pushnumber(L, v->deltaTime());
			lua_setfield(L, -2, "deltaTime");
		}

		call();

		lua_pop(L, 2);
	}*/

	virtual void visit(MouseEvent* v)
	{
		StackChecker checker(L, "visit(MouseEvent* v)", 0);

		Binder binder(L);

		// get closure
		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);
		lua_pushlightuserdata(L, bridge_);
		lua_rawget(L, -2);
		lua_remove(L, -2);		// remove env["eventClosures"]

		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_MouseEvent);

		lua_getfield(L, -1, "__uniqueid");

		if (lua_isnil(L, -1) || lua_tointeger(L, -1) != v->uniqueid())
		{
			lua_pop(L, 1);

			lua_pushinteger(L, v->uniqueid());
			lua_setfield(L, -2, "__uniqueid");

			binder.setInstance(-1, v);

			lua_pushstring(L, v->type()); // TODO: buna artik ihtiyac yok. direk Event'te getType() fonksiyonu var
			lua_setfield(L, -2, "type");

            float rx = (v->x - v->tx) / v->sx;
            float ry = (v->y - v->ty) / v->sy;

            lua_pushinteger(L, floor(rx));
			lua_setfield(L, -2, "x");

            lua_pushinteger(L, floor(ry));
			lua_setfield(L, -2, "y");

            lua_pushnumber(L, rx);
            lua_setfield(L, -2, "rx");

            lua_pushnumber(L, ry);
            lua_setfield(L, -2, "ry");

            lua_pushnumber(L, v->wheel);
            lua_setfield(L, -2, "wheel");

            lua_pushnumber(L, v->button);
            lua_setfield(L, -2, "button");
		}
		else
		{
			lua_pop(L, 1);
		}

		lua_call(L, 1, 0);
	}

    void getOrCreateTouch(ginput_Touch* touch, float sx, float sy, float tx, float ty)
	{
		StackChecker checker(L, "createTouch", 1);

		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_touches);
		lua_pushinteger(L, touch->id);
		lua_rawget(L, -2);

		if (lua_isnil(L, -1))
		{
			lua_pop(L, 1);
			lua_newtable(L);

			lua_pushinteger(L, touch->id);
			lua_pushvalue(L, -2);
			lua_rawset(L, -4);
		}

		lua_remove(L, -2);

        lua_pushinteger(L, touch->id + 1);
		lua_setfield(L, -2, "id");

        float rx = (touch->x - tx) / sx;
        float ry = (touch->y - ty) / sy;

        lua_pushinteger(L, floor(rx));
        lua_setfield(L, -2, "x");

        lua_pushinteger(L, floor(ry));
        lua_setfield(L, -2, "y");

        lua_pushnumber(L, rx);
        lua_setfield(L, -2, "rx");

        lua_pushnumber(L, ry);
        lua_setfield(L, -2, "ry");
		
		
        lua_pushnumber(L, touch->pressure);
        lua_setfield(L, -2, "pressure");

        switch (touch->touchType){
            case 0: {lua_pushstring(L, "finger"); break;}
            case 1: {lua_pushstring(L, "pen"); break;}
            case 2: {lua_pushstring(L, "mouse"); break;}
            case 3: {lua_pushstring(L, "penTablet"); break;}
        }
        lua_setfield(L, -2, "type");
		
		
    }

	virtual void visit(TouchEvent* v)
	{
		StackChecker checker(L, "visit(TouchEvent* v)", 0);

		Binder binder(L);

		// get closure
		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);
		lua_pushlightuserdata(L, bridge_);
		lua_rawget(L, -2);
		lua_remove(L, -2);		// remove env["eventClosures"]

		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_TouchEvent);

		lua_getfield(L, -1, "__uniqueid");

		if (lua_isnil(L, -1) || lua_tointeger(L, -1) != v->uniqueid())
		{
			lua_pop(L, 1);

			lua_pushinteger(L, v->uniqueid());
			lua_setfield(L, -2, "__uniqueid");

			binder.setInstance(-1, v);

			lua_pushstring(L, v->type()); // TODO: buna artik ihtiyac yok. direk Event'te getType() fonksiyonu var
			lua_setfield(L, -2, "type");

            // touch
            getOrCreateTouch(&v->event->touch, v->sx, v->sy, v->tx, v->ty);
            lua_setfield(L, -2, "touch");

            // touches (it has only 1 element)
			lua_getfield(L, -1, "touches");
			if (lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				lua_newtable(L);
			}
			else
			{
				int n = lua_objlen(L, -1);
				for (int i = n; i >= 1; --i)
				{
					lua_pushnil(L);
					lua_rawseti(L, -2, i);
				}
			}
            getOrCreateTouch(&v->event->touch, v->sx, v->sy, v->tx, v->ty);
            lua_rawseti(L, -2, 1);
			lua_setfield(L, -2, "touches");

            // allTouches
			lua_getfield(L, -1, "allTouches");
			if (lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				lua_newtable(L);
			}
			else
			{
				int n = lua_objlen(L, -1);
				for (int i = n; i >= 1; --i)
				{
					lua_pushnil(L);
					lua_rawseti(L, -2, i);
				}
			}
            for (std::size_t i = 0; i < v->event->allTouchesCount; ++i)
			{
                getOrCreateTouch(&v->event->allTouches[i], v->sx, v->sy, v->tx, v->ty);
                lua_rawseti(L, -2, i + 1);
			}
			lua_setfield(L, -2, "allTouches");
		}
		else
		{
			lua_pop(L, 1);
		}
		
		lua_call(L, 1, 0);
	}

	virtual void visit(TimerEvent* v)
	{
		StackChecker checker(L, "visit(TimerEvent* v)", 0);

		Binder binder(L);

		// get closure
		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);
		lua_pushlightuserdata(L, bridge_);
		lua_rawget(L, -2);
		lua_remove(L, -2);		// remove env["eventClosures"]

		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_TimerEvent);

		lua_getfield(L, -1, "__uniqueid");

		if (lua_isnil(L, -1) || lua_tointeger(L, -1) != v->uniqueid())
		{
			lua_pop(L, 1);

			lua_pushinteger(L, v->uniqueid());
			lua_setfield(L, -2, "__uniqueid");

			binder.setInstance(-1, v);

			lua_pushstring(L, v->type()); // TODO: buna artik ihtiyac yok. direk Event'te getType() fonksiyonu var
			lua_setfield(L, -2, "type");
		}
		else
		{
			lua_pop(L, 1);
		}

		lua_call(L, 1, 0);
	}

	virtual void visit(AccelerometerEvent* v)
	{
		//TODO

	}

	virtual void visit(StageOrientationEvent* v)
	{
		StackChecker checker(L, "visit(StageOrientationEvent* v)", 0);

		// get closure
		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);
		lua_pushlightuserdata(L, bridge_);
		lua_rawget(L, -2);
		lua_remove(L, -2);		// remove env["eventClosures"]

		bool newTable = pushEventTable(v, "Event");

		if (newTable == true)
		{
			lua_pushstring(L, v->type()); // TODO: buna artik ihtiyac yok. direk Event'te getType() fonksiyonu var
			lua_setfield(L, -2, "type");

            switch (v->orientation)
			{
            case GAPPLICATION_PORTRAIT:
				lua_pushstring(L, "portrait");
				break;
            case GAPPLICATION_PORTRAIT_UPSIDE_DOWN:
				lua_pushstring(L, "portraitUpsideDown");
				break;
            case GAPPLICATION_LANDSCAPE_LEFT:
				lua_pushstring(L, "landscapeLeft");
				break;
            case GAPPLICATION_LANDSCAPE_RIGHT:
				lua_pushstring(L, "landscapeRight");
				break;
			}
            lua_setfield(L, -2, "orientation");
		}

		lua_call(L, 1, 0);
	}

	virtual void visit(ErrorEvent* v)
	{
		StackChecker checker(L, "visit(ErrorEvent* v)", 0);

		// get closure
		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);
		lua_pushlightuserdata(L, bridge_);
		lua_rawget(L, -2);
		lua_remove(L, -2);		// remove env["eventClosures"]

		bool newTable = pushEventTable(v, "Event");

		if (newTable == true)
		{
			lua_pushstring(L, v->type()); // TODO: buna artik ihtiyac yok. direk Event'te getType() fonksiyonu var
			lua_setfield(L, -2, "type");
		}

		lua_call(L, 1, 0);
	}


	virtual void visit(ProgressEvent* v)
	{
		StackChecker checker(L, "visit(ProgressEvent* v)", 0);

		// get closure
		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);
		lua_pushlightuserdata(L, bridge_);
		lua_rawget(L, -2);
		lua_remove(L, -2);		// remove env["eventClosures"]

		bool newTable = pushEventTable(v, "Event");

		if (newTable == true)
		{
			lua_pushstring(L, v->type()); // TODO: buna artik ihtiyac yok. direk Event'te getType() fonksiyonu var
			lua_setfield(L, -2, "type");

			lua_pushinteger(L, v->bytesLoaded);
			lua_setfield(L, -2, "bytesLoaded");

			lua_pushinteger(L, v->bytesTotal);
			lua_setfield(L, -2, "bytesTotal");
		}

		lua_call(L, 1, 0);
	}

	virtual void visit(KeyboardEvent* v)
	{
        StackChecker checker(L, "visit(KeyboardEvent* v)", 0);

		Binder binder(L);

		// get closure
		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);
		lua_pushlightuserdata(L, bridge_);
		lua_rawget(L, -2);
		lua_remove(L, -2);		// remove env["eventClosures"]

		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_KeyboardEvent);

		lua_getfield(L, -1, "__uniqueid");

		if (lua_isnil(L, -1) || lua_tointeger(L, -1) != v->uniqueid())
		{
			lua_pop(L, 1);

			lua_pushinteger(L, v->uniqueid());
			lua_setfield(L, -2, "__uniqueid");

			binder.setInstance(-1, v);

			lua_pushstring(L, v->type()); // TODO: buna artik ihtiyac yok. direk Event'te getType() fonksiyonu var
			lua_setfield(L, -2, "type");

			lua_pushinteger(L, v->keyCode);
			lua_setfield(L, -2, "keyCode");

            lua_pushinteger(L, v->realCode);
            lua_setfield(L, -2, "realCode");

            lua_pushstring(L, v->charCode.c_str());
            lua_setfield(L, -2, "text");
		}
		else
		{
			lua_pop(L, 1);
		}

		lua_call(L, 1, 0);
	}

    virtual void visit(CompleteEvent* v)
    {
        StackChecker checker(L, "visit(CompleteEvent* v)", 0);

        Binder binder(L);

        // get closure
        luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);
        lua_pushlightuserdata(L, bridge_);
        lua_rawget(L, -2);
        lua_remove(L, -2);		// remove env["eventClosures"]

        luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_CompleteEvent);

        lua_getfield(L, -1, "__uniqueid");

        if (lua_isnil(L, -1) || lua_tointeger(L, -1) != v->uniqueid())
        {
            lua_pop(L, 1);

            lua_pushinteger(L, v->uniqueid());
            lua_setfield(L, -2, "__uniqueid");

            binder.setInstance(-1, v);

            lua_pushstring(L, v->type()); // TODO: buna artik ihtiyac yok. direk Event'te getType() fonksiyonu var
            lua_setfield(L, -2, "type");
        }
        else
        {
            lua_pop(L, 1);
        }

        lua_call(L, 1, 0);
    }

private:
	bool pushEventTable(Event* v, const char* eventName)
	{
		StackChecker checker(L, "pushEventTable", 1);

		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_events); // push envTable["events"]
		lua_pushinteger(L, v->uniqueid());
		lua_gettable(L, -2);	// push envTable["events"][v]

		bool newTable;
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 1);

			//lua_newtable(L);
			lua_getglobal(L, eventName);
			lua_getfield(L, -1, "new");
			lua_pushlightuserdata(L, v);
			lua_call(L, 1, 1);
			lua_remove(L, -2);
			
			lua_pushinteger(L, v->uniqueid()); // this is key
			lua_pushvalue(L, -2); // make copy of new table. this is value
			lua_settable(L, -4);  // envTable["events"][v] = {}

			newTable = true;
		}
		else
		{
			newTable = false;
		}
		
		lua_remove(L, -2);		// remove envTable["events"]

		// stack top is envTable["events"][v]
		return newTable;
	} 
	
	//void call()
	//{
	//	// TODO: hemen hemen diger fonksiyonla ayni. bunu tek fonksiyon haline getir.
	//	lua_getfield(L, 1, "__events");

	//	lua_pushlightuserdata(L, bridge_);
	//	lua_gettable(L, -2);

	//	lua_getfield(L, -1, "data"); // check if table has field "data"
	//	bool hasData = (lua_isnil(L, -1) == 0);
	//	lua_pop(L, 1);

	//	if (hasData)
	//	{
	//		lua_getfield(L, -1, "function");
	//		lua_getfield(L, -2, "data");
	//		lua_pushvalue(L, 2);
	//		lua_call(L, 2, 0);
	//	}
	//	else
	//	{
	//		lua_getfield(L, -1, "function");
	//		lua_pushvalue(L, 2);
	//		lua_call(L, 1, 0);
	//	}

	//	lua_pop(L, 2);
	//}

private:
	lua_State* L;
	CppLuaBridge* bridge_;
};

//static int callCppEvent(lua_State* L)
//{
//	CppLuaBridge* bridge = static_cast<CppLuaBridge*>(lua_touserdata(L, 1));
//	LuaEvent* event = static_cast<LuaEvent*>(lua_touserdata(L, 2));
//	lua_pop(L, 2);
//
//	PushEventVisitor v(L, bridge);
//	event->apply(&v);
//
//	return 0;
//}


void CppLuaBridge::luaEvent(LuaEvent* event)
{
	StackChecker checker(L, "CppLuaBridge::luaEvent", 0);

	EventTypeVisitor v;
	event->apply(&v);

	if (v.type == EventTypeVisitor::eLuaEvent)
	{
		// get closure
		luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);
		lua_pushlightuserdata(L, this);
		lua_rawget(L, -2);
		lua_remove(L, -2);		// remove env["eventClosures"]

        lua_pushvalue(L, -2);	// push event

		lua_call(L, 1, 0);		// call event closure
	}
	else
	{
#if 1
		PushEventVisitor v(L, this);
		event->apply(&v);
#else
		// yukardaki iki satiri callCppEvent adli function'a almamizin sebebi, 
		// o iki satir cagirildigi zaman lua_getttop()'in 0 olmasini istememiz 
		lua_pushcfunction(L, callCppEvent);
		lua_pushlightuserdata(L, this);
		lua_pushlightuserdata(L, event);
		lua_call(L, 2, 0);
#endif
	}
}

EventBinderMap::~EventBinderMap()
{
	for(std::map<int, std::vector<CppLuaBridge*> >::iterator iter = map_.begin(); iter != map_.end(); ++iter)
		for (std::size_t i = 0; i < iter->second.size(); ++i)
			iter->second[i]->unref();

	map_.clear();
}

void EventBinderMap::push_back(int index, CppLuaBridge* bridge)
{
	bridge->ref();
	map_[index].push_back(bridge);
}

void EventBinderMap::remove(int index, CppLuaBridge* bridge)
{
	std::vector<CppLuaBridge*>& bridges = map_[index];
	bridges.erase(std::find(bridges.begin(), bridges.end(), bridge));
	bridge->unref();
}

static int eventClosure(lua_State* L)
{
    // 1: {self, function}

	lua_rawgeti(L, lua_upvalueindex(1), 1);	// self
	if (lua_isnil(L, -1))	// self collect edilmis mi?
	{
		lua_pop(L, 1);
		return 0;
	}

	lua_setfield(L, 1, "__target");	// set event.__target=self

    lua_rawgeti(L, lua_upvalueindex(1), 2);	// function
    if (lua_isnil(L, -1))	// function collect edilmis mi?
    {
        lua_pop(L, 1);
        return 0;
    }

	lua_pushvalue(L, 1);					// event

	lua_call(L, 1, 0);

    lua_pushnil(L);
    lua_setfield(L, 1, "__target");	// set event.__target=nil

	return 0;
}

static int eventClosureWithData(lua_State* L)
{
    // 1: {self, function, data}

	lua_rawgeti(L, lua_upvalueindex(1), 1);	// self
	if (lua_isnil(L, -1))	// self collect edilmis mi?
	{
		lua_pop(L, 1);
		return 0;
	}

	lua_setfield(L, 1, "__target");	// set event.__target=self

    lua_rawgeti(L, lua_upvalueindex(1), 2);	// function
    if (lua_isnil(L, -1))	// function collect edilmis mi?
    {
        lua_pop(L, 1);
        return 0;
    }

    lua_rawgeti(L, lua_upvalueindex(1), 3);	// data
	if (lua_isnil(L, -1))	// data collect edilmis mi?
	{
		lua_pop(L, 2);
		return 0;
	}

	lua_pushvalue(L, 1);					// event

	lua_call(L, 2, 0);

    lua_pushnil(L);
    lua_setfield(L, 1, "__target");	// set event.__target=nil

	return 0;
}

static int eventCheckClosure(lua_State* L)
{
	// 1: function
	lua_pushboolean(L, lua_rawequal(L, lua_upvalueindex(1), 1));
	return 1;
}

static int eventCheckClosureWithData(lua_State* L)
{
	// 1: function
	// 2: data
	lua_pushboolean(L, lua_rawequal(L, lua_upvalueindex(1), 1) && lua_rawequal(L, lua_upvalueindex(2), 2));
	return 1;
}


static void createEventsTable(lua_State* L, int index)
{
	lua_getfield(L, index, "__events");
	if (lua_isnil(L, -1) != 0 || lua_istable(L, -1) == 0) // if it is nil or it isn't a table
	{
		lua_pop(L, 1);
		lua_newtable(L);
		lua_setfield(L, index, "__events");		// create __events table if it's not created
	}
	else
		lua_pop(L, 1);		// pop __events
}


static char key_map = ' ';

static EventBinderMap& getOrCreateEventBinderMap(EventDispatcher* eventDispatcher)
{
    if (eventDispatcher->data(&key_map) == 0)
	{
		EventBinderMap* map = new EventBinderMap;
        eventDispatcher->setData(&key_map, map);
		map->unref();
	}

    return *static_cast<EventBinderMap*>(eventDispatcher->data(&key_map));
}

int EventDispatcherBinder::addEventListener(lua_State* L)
{
	StackChecker checker(L, "EventDispatcherBinder::addEventListener", 0);

	Binder binder(L);
	EventDispatcher* eventDispatcher = static_cast<EventDispatcher*>(binder.getInstance("EventDispatcher", 1));

	luaL_checktype(L, 2, LUA_TSTRING);
	luaL_checktype(L, 3, LUA_TFUNCTION);

    bool hasData = !lua_isnoneornil(L, 4);

	createEventsTable(L, 1);	// create self.__events table if it's not created

	EventBinderMap& map = getOrCreateEventBinderMap(eventDispatcher);

	const char* event = lua_tostring(L, 2);
	int eventid = StringId::instance().id(event);

	const std::vector<CppLuaBridge*>& bridges = map[eventid]; 

	lua_getfield(L, 1, "__events");		// key is CppLuaBridge*, value is 'event check closure'

	// check if the event is already registered
	bool isFound = false;
	for (std::size_t i = 0; i < bridges.size(); ++i)
	{
		lua_pushlightuserdata(L, bridges[i]);
		lua_rawget(L, -2);	// we get the event check closure
		if (hasData == false)
		{
			lua_pushvalue(L, 3);	// function
			lua_call(L, 1, 1);
		}
		else
		{
			lua_pushvalue(L, 3);	// function
			lua_pushvalue(L, 4);	// data
			lua_call(L, 2, 1);
		}

		if (lua_toboolean(L, -1))
		{
			lua_pop(L, 1);
			isFound = true;
			break;
		}
		else
			lua_pop(L, 1);
	}

	if (isFound == true)
	{
		lua_pop(L, 1);		// pop __events, leave stack as it is
		return 0;
	}

    LuaApplication *application = (LuaApplication*)luaL_getdata(L);
    lua_State *mainL = application->getLuaState();

    CppLuaBridge* bridge = new CppLuaBridge(mainL);

	// create event closure
	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);
	lua_pushlightuserdata(L, bridge);	// key=bridge
	if (hasData == false)				// value=closure
	{
        // self ve function'in eventClosure'in icine upvalue olarak koyulmasi garbage collect edilmesini engelliyor
        // bu yuzden {self, function} seklinde bi weak table yaratip ilk upvalue olarak onu set ediyoruz
		luaL_newweaktable(L);

        lua_pushvalue(L, 1);	// self
		lua_rawseti(L, -2, 1);

		lua_pushvalue(L, 3);	// function
        lua_rawseti(L, -2, 2);

        lua_pushcclosure(L, &eventClosure, 1);
	}
	else
	{
        // self, function ve data'nin eventClosure'in icine upvalue olarak koyulmasi garbage collect edilmesini engelliyor
        // bu yuzden {self, function, data} seklinde bi weak table yaratip ilk upvalue olarak onu set ediyoruz
		luaL_newweaktable(L);

        lua_pushvalue(L, 1);	// self
		lua_rawseti(L, -2, 1);

        lua_pushvalue(L, 3);	// function
        lua_rawseti(L, -2, 2);

        lua_pushvalue(L, 4);	// data
        lua_rawseti(L, -2, 3);

        lua_pushcclosure(L, &eventClosureWithData, 1);
	}
	lua_rawset(L, -3);					// envtable["eventClosures"][bridge] = closure
	lua_pop(L, 1);						// pop envtable["eventClosures"]

	// create event check closure
	lua_pushlightuserdata(L, bridge);
	if (hasData == false)
	{
		lua_pushvalue(L, 3);	// function
		lua_pushcclosure(L, &eventCheckClosure, 1);
	}
	else
	{
		lua_pushvalue(L, 3);	// function
		lua_pushvalue(L, 4);	// data
		lua_pushcclosure(L, &eventCheckClosureWithData, 2);
	}
	lua_rawset(L, -3);
	
	map.push_back(eventid, bridge);

	bridge->unref();
	
	eventDispatcher->addEventListener(LuaEvent::Type(event), bridge, &CppLuaBridge::luaEvent);

	lua_pop(L, 1);			// pop __events, leave stack as it is

	return 0;
}

int EventDispatcherBinder::dispatchEvent(lua_State* L)
{
	StackChecker checker(L, "EventDispatcherBinder::dispatchEvent", 0);

	Binder binder(L);
	EventDispatcher* eventDispatcher = static_cast<EventDispatcher*>(binder.getInstance("EventDispatcher", 1));

	luaL_checktype(L, 2, LUA_TTABLE);
//	lua_getfield(L, 2, "type");
	lua_getfield(L, 2, "getType");
	lua_pushvalue(L, 2);
	lua_call(L, 1, 1);
	std::string event = luaL_checkstring(L, -1);
	lua_pop(L, 1);
	LuaEvent e = LuaEvent(LuaEvent::Type(event.c_str()));

    LuaApplication *application = (LuaApplication*)luaL_getdata(L);
    lua_State *mainL = application->getLuaState();

    lua_pushvalue(L, 2);    // push event to main thread
    if (mainL != L)
        lua_xmove(L, mainL, 1);

    eventDispatcher->dispatchEvent(&e);

    lua_pop(mainL, 1);      // pop event from main thread

	return 0;
}

int EventDispatcherBinder::removeEventListener(lua_State* L)
{
	StackChecker checker(L, "EventDispatcherBinder::removeEventListener", 0);

	Binder binder(L);
	EventDispatcher* eventDispatcher = static_cast<EventDispatcher*>(binder.getInstance("EventDispatcher"));

	luaL_checktype(L, 2, LUA_TSTRING);
	luaL_checktype(L, 3, LUA_TFUNCTION);

	bool hasData = lua_gettop(L) >= 4;

	createEventsTable(L, 1);	// create __events table if it's not created

	EventBinderMap& map = getOrCreateEventBinderMap(eventDispatcher);

	const char* event = lua_tostring(L, 2);
	int eventid = StringId::instance().id(event);

	const std::vector<CppLuaBridge*>& bridges = map[eventid]; 

	lua_getfield(L, 1, "__events");

	// check if the event is already registered
	CppLuaBridge* bridge = 0;
	for (std::size_t i = 0; i < bridges.size(); ++i)
	{
		lua_pushlightuserdata(L, bridges[i]);
		lua_rawget(L, -2);	// we get the event check closure
		if (hasData == false)
		{
			lua_pushvalue(L, 3);	// function
			lua_call(L, 1, 1);
		}
		else
		{
			lua_pushvalue(L, 3);	// function
			lua_pushvalue(L, 4);	// data
			lua_call(L, 2, 1);
		}

		if (lua_toboolean(L, -1))
		{
			bridge = bridges[i];
			lua_pop(L, 1);
			break;
		}
		else
			lua_pop(L, 1);
	}

	if (bridge == 0)	// event is not registered
	{
		lua_pop(L, 1);		// pop __events
		return 0;
	}

	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);
	lua_pushlightuserdata(L, bridge);	// key=bridge
	lua_pushnil(L);						// value=nil
	lua_rawset(L, -3);					// envtable["eventClosures"][bridge] = nil
	lua_pop(L, 1);						// pop envtable["eventClosures"]

	lua_pushlightuserdata(L, bridge);	// key=bridge
	lua_pushnil(L);						// value = nil
	lua_settable(L, -3);				// __events[bridge] = nil

	eventDispatcher->removeEventListener(LuaEvent::Type(event), bridge, &CppLuaBridge::luaEvent);

	map.remove(eventid, bridge);
		
	lua_pop(L, 1);		// pop __events

	return 0;
}

int EventDispatcherBinder::hasEventListener(lua_State* L)
{
	StackChecker checker(L, "EventDispatcherBinder::hasEventListener", 1);

	Binder binder(L);
	EventDispatcher* eventDispatcher = static_cast<EventDispatcher*>(binder.getInstance("EventDispatcher"));

	luaL_checktype(L, 2, LUA_TSTRING);

    if (eventDispatcher->data(&key_map) == NULL)
	{
		lua_pushboolean(L, 0);
	}
	else
	{
        EventBinderMap& map = *static_cast<EventBinderMap*>(eventDispatcher->data(&key_map));
		
		const char* event = lua_tostring(L, 2);
		int eventid = StringId::instance().id(event);
		
		const std::vector<CppLuaBridge*>& bridges = map[eventid]; 
		
		lua_pushboolean(L, bridges.empty() ? 0 : 1);
	}

	return 1;
}
