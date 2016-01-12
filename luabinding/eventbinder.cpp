#include "eventbinder.h"
#include "stackchecker.h"
#include "event.h"
#include "stageorientationevent.h"

#include "mouseevent.h"
#include "touchevent.h"
#include "timerevent.h"
#include "errorevent.h"
#include "progressevent.h"
#include "keyboardevent.h"
#include "completeevent.h"

EventBinder::EventBinder(lua_State* L)
{
	StackChecker checker(L, "EventBinder::EventBinder", 0);

	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"getType", &EventBinder::getType},
		{"getTarget", &EventBinder::getTarget},
		{"stopPropagation", &EventBinder::stopPropagation},
		{NULL, NULL},
	};

	binder.createClass("Event", NULL, create, 0, functionList);

	lua_getglobal(L, "Event");	// get metatable

	lua_pushstring(L, Event::ENTER_FRAME.type());
	lua_setfield(L, -2, "ENTER_FRAME");

	lua_pushstring(L, Event::SOUND_COMPLETE.type());
	lua_setfield(L, -2, "SOUND_COMPLETE");
	
	lua_pushstring(L, Event::ADDED_TO_STAGE.type());
	lua_setfield(L, -2, "ADDED_TO_STAGE");

	lua_pushstring(L, Event::REMOVED_FROM_STAGE.type());
	lua_setfield(L, -2, "REMOVED_FROM_STAGE");

/*
	lua_pushstring(L, Event::APPLICATION_DID_FINISH_LAUNCHING.type());
	lua_setfield(L, -2, "APPLICATION_DID_FINISH_LAUNCHING");

	lua_pushstring(L, Event::APPLICATION_WILL_TERMINATE.type());
	lua_setfield(L, -2, "APPLICATION_WILL_TERMINATE");
	*/

    lua_pushstring(L, CompleteEvent::COMPLETE.type());
	lua_setfield(L, -2, "COMPLETE");

	lua_pushstring(L, Event::APPLICATION_START.type());
	lua_setfield(L, -2, "APPLICATION_START");

	lua_pushstring(L, Event::APPLICATION_EXIT.type());
	lua_setfield(L, -2, "APPLICATION_EXIT");

	lua_pushstring(L, Event::APPLICATION_SUSPEND.type());
	lua_setfield(L, -2, "APPLICATION_SUSPEND");

	lua_pushstring(L, Event::APPLICATION_RESUME.type());
	lua_setfield(L, -2, "APPLICATION_RESUME");

    lua_pushstring(L, Event::APPLICATION_BACKGROUND.type());
    lua_setfield(L, -2, "APPLICATION_BACKGROUND");

    lua_pushstring(L, Event::APPLICATION_FOREGROUND.type());
    lua_setfield(L, -2, "APPLICATION_FOREGROUND");

    lua_pushstring(L, Event::APPLICATION_RESIZE.type());
    lua_setfield(L, -2, "APPLICATION_RESIZE");

    lua_pushstring(L, Event::MEMORY_WARNING.type());
    lua_setfield(L, -2, "MEMORY_WARNING");

	lua_pushstring(L, StageOrientationEvent::ORIENTATION_CHANGE.type());
	lua_setfield(L, -2, "ORIENTATION_CHANGE");

	lua_pushstring(L, MouseEvent::MOUSE_UP.type());
	lua_setfield(L, -2, "MOUSE_UP");

	lua_pushstring(L, MouseEvent::MOUSE_DOWN.type());
	lua_setfield(L, -2, "MOUSE_DOWN");

	lua_pushstring(L, MouseEvent::MOUSE_MOVE.type());
	lua_setfield(L, -2, "MOUSE_MOVE");

    lua_pushstring(L, MouseEvent::MOUSE_HOVER.type());
    lua_setfield(L, -2, "MOUSE_HOVER");

	lua_pushstring(L, MouseEvent::MOUSE_WHEEL.type());
	lua_setfield(L, -2, "MOUSE_WHEEL");


	lua_pushstring(L, TouchEvent::TOUCHES_BEGIN.type());
	lua_setfield(L, -2, "TOUCHES_BEGIN");

	lua_pushstring(L, TouchEvent::TOUCHES_MOVE.type());
	lua_setfield(L, -2, "TOUCHES_MOVE");

	lua_pushstring(L, TouchEvent::TOUCHES_END.type());
	lua_setfield(L, -2, "TOUCHES_END");

	lua_pushstring(L, TouchEvent::TOUCHES_CANCEL.type());
	lua_setfield(L, -2, "TOUCHES_CANCEL");



	lua_pushstring(L, TimerEvent::TIMER.type());
	lua_setfield(L, -2, "TIMER");

	lua_pushstring(L, TimerEvent::TIMER_COMPLETE.type());
	lua_setfield(L, -2, "TIMER_COMPLETE");

	lua_pushstring(L, ErrorEvent::ERROR.type());
	lua_setfield(L, -2, "ERROR");

	lua_pushstring(L, ProgressEvent::PROGRESS.type());
	lua_setfield(L, -2, "PROGRESS");


	lua_pushstring(L, KeyboardEvent::KEY_UP.type());
	lua_setfield(L, -2, "KEY_UP");

	lua_pushstring(L, KeyboardEvent::KEY_DOWN.type());
	lua_setfield(L, -2, "KEY_DOWN");

	lua_pushstring(L, KeyboardEvent::KEY_CHAR.type());
	lua_setfield(L, -2, "KEY_CHAR");

	lua_pop(L, 1);
}

//TODO: adam Event.ENTER_FRAME'e register olmus olsun. ona gelen Event'i killigina baska bi yere saklamis olsun. o frame bittikten sonra, Event icinde saklanan lightuserdata'nin gosterdigi
// instance ucuyor. (zaten yanlis hatirlamiyorsam direk stack'te olan bi instance o). sonra getType() falan dedigimiz gibi gocecek program. belki bunu engellemek icin birseyler yapabiliriz.
int EventBinder::create(lua_State* L)
{
	StackChecker checker(L, "EventBinder::create", 1);

	Binder binder(L);

	if (lua_type(L, 1) != LUA_TSTRING && lua_type(L, 1) != LUA_TLIGHTUSERDATA)
		luaL_typerror(L, 1, "string");

	if (lua_type(L, 1) == LUA_TSTRING)
	{
		binder.pushInstance("Event", 0);
		lua_pushvalue(L, 1);
		lua_setfield(L, -2, "__type");

		// TODO: eger 2. parametere table ise ondaki degerleri buna transfer et.
		//Event.new("myEvent", {x=10,y=10,z=20}) diyebilmek icin
	}
	else
	{
		void* event = lua_touserdata(L, 1);
		binder.pushInstance("Event", event);
	}

	return 1;
}

int EventBinder::getType(lua_State* L)
{
	StackChecker checker(L, "EventBinder::getType", 1);

	Binder binder(L);
	Event* event = static_cast<Event*>(binder.getInstance("Event"));

	if (event == 0)
	{
		lua_getfield(L, 1, "__type");
	}
	else
	{
		lua_pushstring(L, event->type());
	}

	return 1;
}


int EventBinder::stopPropagation(lua_State* L)
{
	StackChecker checker(L, "EventBinder::stopPropagation", 0);

	Binder binder(L);
	Event* event = static_cast<Event*>(binder.getInstance("Event"));

	if (event == 0)
	{
		lua_pushboolean(L, 1);
		lua_setfield(L, 1, "__stopPropagation");
	}
	else
	{
		event->stopPropagation();
	}

	return 0;
}


int EventBinder::getTarget(lua_State* L)
{
	StackChecker checker(L, "EventBinder::getTarget", 1);

	lua_getfield(L, 1, "__target");

	return 1;
}
