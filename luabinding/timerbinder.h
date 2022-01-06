#ifndef TIMERBINDER_H
#define TIMERBINDER_H

#include "binder.h"

class TimerBinder
{
public:
	TimerBinder(lua_State* L);

private:
	static int create(lua_State* L);
	static int destruct(void *p);

	static int start(lua_State* L);
	static int stop(lua_State* L);
	static int reset(lua_State* L);
	static int pause(lua_State* L);

	static int getDelay(lua_State* L);
	static int getCurrentCount(lua_State* L);
	static int getRepeatCount(lua_State* L);
	static int getRunning(lua_State* L);

	static int setDelay(lua_State* L);
	static int setRepeatCount(lua_State* L);

	static int pauseAllTimers(lua_State* L);
	static int resumeAllTimers(lua_State* L);
	static int stopAllTimers(lua_State* L);
};

#endif
