#ifndef MOVIECLIPBINDER_H
#define MOVIECLIPBINDER_H

#include "binder.h"

class MovieClipBinder
{
public:
	MovieClipBinder(lua_State* L);

private:
	static int create(lua_State* L);
	static int destruct(lua_State* L);

	static int play(lua_State* L);
	static int stop(lua_State* L);
	static int gotoAndPlay(lua_State* L);
	static int gotoAndStop(lua_State* L);
	static int setStopAction(lua_State* L);
	static int setGotoAction(lua_State* L);
	static int setReverseAction(lua_State* L);
	static int clearAction(lua_State* L);
	static int getFrame(lua_State* L);
};

#endif
