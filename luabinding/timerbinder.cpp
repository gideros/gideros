#include "timerbinder.h"
#include "timer.h"
#include "stackchecker.h"
#include "keys.h"
#include "luautil.h"
#include "platformutil.h"

TimerBinder::TimerBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"start", TimerBinder::start},
		{"stop", TimerBinder::stop},
		{"reset", TimerBinder::reset},

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

#ifndef abs_index

/* convert a stack index to positive */
#define abs_index(L, i)		((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : \
	lua_gettop(L) + (i) + 1)

#endif


static void clearNotRunningTimers(lua_State* L)
{
	lua_getfield(L, LUA_ENVIRONINDEX, "timers");

	int t = abs_index(L, -1);

	/* table is in the stack at index 't' */
	lua_pushnil(L);  /* first key */
	while (lua_next(L, t) != 0) {
	  /* uses 'key' (at index -2) and 'value' (at index -1) */
#if 0
	  printf("%s - %s\n",
			 lua_typename(L, lua_type(L, -2)),
			 lua_typename(L, lua_type(L, -1)));
#endif

	  Timer* timer = static_cast<Timer*>(lua_touserdata(L, -2));

	  if (timer->running())
	  {
		  lua_pushvalue(L, -2);
		  lua_pushnil(L);
		  lua_settable(L, t);		// table[key] = nil
	  }

	  /* removes 'value'; keeps 'key' for next iteration */
	  lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

int TimerBinder::create(lua_State* L)
{
//	printf("TimerBinder::create()\n");
	StackChecker checker(L, "Timer", 1);

	double delay = luaL_checknumber(L, 1);
	int repeatCount = luaL_optinteger(L, 2, 0);

	Binder binder(L);
	Timer* timer = new Timer(delay, repeatCount);
	binder.pushInstance("Timer", timer);

	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_timers);
	lua_pushvalue(L, -2);
	luaL_rawsetptr(L, -2, timer);
	lua_pop(L, 1);

#if 0
	clearNotRunningTimers(L);
#endif

	return 1;
}

int TimerBinder::destruct(lua_State* L)
{
	//debuglog("TimerBinder::destruct()");

	void* ptr = *(void**)lua_touserdata(L, 1);
	Timer* timer = static_cast<Timer*>(ptr);
	timer->unref();

	return 0;
}

int TimerBinder::start(lua_State* L)
{
	StackChecker checker(L, "TimerBinder::start()", 0);

	Binder binder(L);
	Timer* timer = static_cast<Timer*>(binder.getInstance("Timer", 1));
	timer->start();

#if 0
	lua_getfield(L, LUA_ENVIRONINDEX, "timers");
	lua_pushlightuserdata(L, timer);
	lua_pushvalue(L, 1);
	lua_settable(L, -3);
	lua_pop(L, 1);
#endif

	return 0;
}

int TimerBinder::stop(lua_State* L)
{
	StackChecker checker(L, "TimerBinder::stop()", 0);

	Binder binder(L);
	Timer* timer = static_cast<Timer*>(binder.getInstance("Timer", 1));
	timer->stop();

#if 0
	lua_getfield(L, LUA_ENVIRONINDEX, "timers");
	lua_pushlightuserdata(L, timer);
	lua_pushnil(L);
	lua_settable(L, -3);
	lua_pop(L, 1);
#endif

	return 0;
}


int TimerBinder::reset(lua_State* L)
{
	StackChecker checker(L, "TimerBinder::reset()", 0);

	Binder binder(L);
	Timer* timer = static_cast<Timer*>(binder.getInstance("Timer", 1));
	timer->reset();

#if 0
	lua_getfield(L, LUA_ENVIRONINDEX, "timers");
	lua_pushlightuserdata(L, timer);
	lua_pushnil(L);
	lua_settable(L, -3);
	lua_pop(L, 1);
#endif

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
	Timer::pauseAllTimers();
	return 0;
}

int TimerBinder::resumeAllTimers(lua_State* L)
{
	Timer::resumeAllTimers();
	return 0;
}

int TimerBinder::stopAllTimers(lua_State* L)
{
	Timer::stopAllTimers();
#if 0
	clearNotRunningTimers(L);
#endif
	return 0;
}
