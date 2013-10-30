#include "timerbinder.h"
#include "timer.h"
#include "stackchecker.h"
#include "keys.h"
#include "luautil.h"
#include "platformutil.h"
#include "timerevent.h"
#include "luaapplication.h"
#include <application.h>
#include <timercontainer.h>
#include <glog.h>

TimerBinder::TimerBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"start", TimerBinder::start},
		{"stop", TimerBinder::stop},
		{"reset", TimerBinder::reset},
		{"pause", TimerBinder::pause},

		{"getDelay", TimerBinder::getDelay},
		{"getCurrentCount", TimerBinder::getCurrentCount},
		{"getRepeatCount", TimerBinder::getRepeatCount},
		{"isRunning", TimerBinder::getRunning},

		{"setDelay", TimerBinder::setDelay},
		{"setRepeatCount", TimerBinder::setRepeatCount},

		{"pauseAll", TimerBinder::pauseAllTimers},
		{"resumeAll", TimerBinder::resumeAllTimers},
		{"stopAll", TimerBinder::stopAllTimers},

		{NULL, NULL},
	};

	binder.createClass("Timer", "EventDispatcher", create, destruct, functionList);
}

class TimerListener : public EventDispatcher
{
public:
    TimerListener(lua_State *L, Timer *timer) :
        L(L),
        timer(timer)
    {
    }

    ~TimerListener()
    {
    }

    void timerCompleteEvent(TimerEvent *event)
    {
        luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_timers);
        lua_pushnil(L);
        luaL_rawsetptr(L, -2, timer);
        lua_pop(L, 1);
    }

    lua_State *L;
    Timer *timer;
};

static char key_timer = ' ';

int TimerBinder::create(lua_State* L)
{
    StackChecker checker(L, "TimerBinder::create", 1);

	double delay = luaL_checknumber(L, 1);
	int repeatCount = luaL_optinteger(L, 2, 0);

    LuaApplication *application = (LuaApplication*)luaL_getdata(L);

	Binder binder(L);
    Timer *timer = new Timer(application->getApplication(), delay, repeatCount);
	binder.pushInstance("Timer", timer);

    lua_State *mainL = application->getLuaState();

    TimerListener *timerListener = new TimerListener(mainL, timer);
    timer->setData(&key_timer, timerListener);
    timerListener->unref();
    timer->addEventListener(TimerEvent::TIMER_COMPLETE, timerListener, &TimerListener::timerCompleteEvent);

	return 1;
}

int TimerBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	Timer* timer = static_cast<Timer*>(ptr);
	timer->unref();

	return 0;
}

int TimerBinder::start(lua_State* L)
{
    StackChecker checker(L, "TimerBinder::start", 0);

	Binder binder(L);
	Timer* timer = static_cast<Timer*>(binder.getInstance("Timer", 1));
	timer->start();

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_timers);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, timer);
    lua_pop(L, 1);

	return 0;
}

int TimerBinder::stop(lua_State* L)
{
	StackChecker checker(L, "TimerBinder::stop()", 0);

	Binder binder(L);
	Timer* timer = static_cast<Timer*>(binder.getInstance("Timer", 1));
	timer->stop();

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_timers);
    lua_pushnil(L);
    luaL_rawsetptr(L, -2, timer);
    lua_pop(L, 1);

	return 0;
}


int TimerBinder::reset(lua_State* L)
{
	StackChecker checker(L, "TimerBinder::reset()", 0);

	Binder binder(L);
	Timer* timer = static_cast<Timer*>(binder.getInstance("Timer", 1));
	timer->reset();

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_timers);
    lua_pushnil(L);
    luaL_rawsetptr(L, -2, timer);
    lua_pop(L, 1);

	return 0;
}

int TimerBinder::pause(lua_State* L)
{
    StackChecker checker(L, "TimerBinder::pause()", 0);

    Binder binder(L);
    Timer* timer = static_cast<Timer*>(binder.getInstance("Timer", 1));
    timer->pause();

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_timers);
    lua_pushnil(L);
    luaL_rawsetptr(L, -2, timer);
    lua_pop(L, 1);

    return 0;
}

int TimerBinder::getDelay(lua_State* L)
{
	StackChecker checker(L, "TimerBinder::getDelay", 1);
	
	Binder binder(L);
	Timer* timer = static_cast<Timer*>(binder.getInstance("Timer"));

	lua_pushnumber(L, timer->delay());

	return 1;
}

int TimerBinder::getCurrentCount(lua_State* L)
{
	StackChecker checker(L, "TimerBinder::getCurrentCount", 1);
	
	Binder binder(L);
	Timer* timer = static_cast<Timer*>(binder.getInstance("Timer"));

	lua_pushinteger(L, timer->currentCount());

	return 1;
}

int TimerBinder::getRepeatCount(lua_State* L)
{
	StackChecker checker(L, "TimerBinder::getRepeatCount", 1);
	
	Binder binder(L);
	Timer* timer = static_cast<Timer*>(binder.getInstance("Timer"));

	lua_pushinteger(L, timer->repeatCount());

	return 1;
}

int TimerBinder::getRunning(lua_State* L)
{
	StackChecker checker(L, "TimerBinder::getRunning", 1);
	
	Binder binder(L);
	Timer* timer = static_cast<Timer*>(binder.getInstance("Timer"));

	lua_pushboolean(L, timer->running());

	return 1;
}

int TimerBinder::setDelay(lua_State* L)
{
	StackChecker checker(L, "TimerBinder::setDelay", 0);
	
	Binder binder(L);
	Timer* timer = static_cast<Timer*>(binder.getInstance("Timer"));

	double delay = luaL_checknumber(L, 2);
	timer->setDelay(delay);

	return 0;
}

int TimerBinder::setRepeatCount(lua_State* L)
{
	StackChecker checker(L, "TimerBinder::setRepeatCount", 0);
	
	Binder binder(L);
	Timer* timer = static_cast<Timer*>(binder.getInstance("Timer"));

	int repeatCount = luaL_checkinteger(L, 2);
	timer->setRepeatCount(repeatCount);

	return 0;
}

int TimerBinder::pauseAllTimers(lua_State* L)
{
    LuaApplication *application = (LuaApplication*)luaL_getdata(L);
    TimerContainer *timerContainer = application->getApplication()->getTimerContainer();

    timerContainer->pauseAllTimers();

    return 0;
}

int TimerBinder::resumeAllTimers(lua_State* L)
{
    LuaApplication *application = (LuaApplication*)luaL_getdata(L);
    TimerContainer *timerContainer = application->getApplication()->getTimerContainer();

    timerContainer->resumeAllTimers();

    return 0;
}

int TimerBinder::stopAllTimers(lua_State* L)
{
    LuaApplication *application = (LuaApplication*)luaL_getdata(L);
    TimerContainer *timerContainer = application->getApplication()->getTimerContainer();

    timerContainer->removeAllTimers();

    lua_newtable(L);
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_timers);

	return 0;
}
