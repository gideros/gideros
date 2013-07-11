#include "movieclipbinder.h"
#include "movieclip2.h"
#include "stackchecker.h"
#include "stringid.h"
#include "luaapplication.h"

MovieClipBinder::MovieClipBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"play", play},
		{"stop", stop},
		{"gotoAndPlay", gotoAndPlay},
		{"gotoAndStop", gotoAndStop},
		{"setStopAction", setStopAction},
		{"setGotoAction", setGotoAction},
		{"clearAction", clearAction},
		{NULL, NULL},
	};

	binder.createClass("MovieClip", "Sprite", create, destruct, functionList);
}


struct AutoUnref
{
public:
	AutoUnref(GReferenced* refptr) : refptr_(refptr)
	{
	}

	~AutoUnref()
	{
		if (refptr_)
			refptr_->unref();
	}

	void release()
	{
		refptr_ = NULL;
	}

private:
	GReferenced* refptr_;
};

#ifndef abs_index

/* convert a stack index to positive */
#define abs_index(L, i)		((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : \
	lua_gettop(L) + (i) + 1)

#endif

int MovieClipBinder::create(lua_State* L)
{
	StackChecker checker(L, "MovieClipBinder::create", 1);

    LuaApplication* application = static_cast<LuaApplication*>(lua_getdata(L));

    Binder binder(L);

    if (lua_type(L, 1) != LUA_TTABLE)
        return luaL_typerror(L, 3, "table");

    if (lua_objlen(L, 1) == 0)
        luaL_error(L, GStatus(2102).errorString());     // Error #2102 Timeline array doesn't contain any elements.

    MovieClip* movieclip = new MovieClip(application->getApplication());	// box movieclip to unref

	AutoUnref autounref(movieclip);

	int len = lua_objlen(L, 1);
	for (int i = 1; i <= len; ++i)
	{
		lua_rawgeti(L, 1, i);

        if (lua_type(L, -1) != LUA_TTABLE)
            luaL_error(L, GStatus(2103).errorString());     // Error #2102 Timeline element is not a table

		lua_rawgeti(L, -1, 1);
		int start = luaL_checkinteger(L, -1);
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 2);
		int end = luaL_checkinteger(L, -1);
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 3);
		Sprite* sprite = static_cast<Sprite*>(binder.getInstance("Sprite", -1));
		lua_pop(L, 1);

		std::vector<Parameter> parameters;

		lua_rawgeti(L, -1, 4);
		if (lua_istable(L, -1))
		{
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
				
				const char* param = luaL_checkstring(L, -2);
				TweenType tweenType = eEaseLinear;
				
				lua_Number start, end;
				if (lua_istable(L, -1))
				{
					lua_rawgeti(L, -1, 1);
					start = luaL_checknumber(L, -1);
					lua_pop(L, 1);

					lua_rawgeti(L, -1, 2);
					end = luaL_checknumber(L, -1);
					lua_pop(L, 1);

					lua_rawgeti(L, -1, 3);
					if (!lua_isnil(L, -1))
						tweenType = (TweenType)StringId::instance().id(luaL_checkstring(L, -1));
					lua_pop(L, 1);
				}
				else
				{
					start = end = luaL_checkinteger(L, -1);
				}

				parameters.push_back(Parameter(param, start, end, tweenType));

				/* removes 'value'; keeps 'key' for next iteration */
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		movieclip->addFrame(start, end, sprite, parameters);

		lua_pop(L, 1);
	}

	autounref.release();

	movieclip->finalize();

	binder.pushInstance("MovieClip", movieclip);

	return 1;
}

int MovieClipBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	MovieClip* movieclip = static_cast<MovieClip*>(ptr);
	movieclip->unref();

	return 0;
}

int MovieClipBinder::play(lua_State* L)
{
	StackChecker checker(L, "MovieClipBinder::play", 0);

	Binder binder(L);
	MovieClip* movieclip = static_cast<MovieClip*>(binder.getInstance("MovieClip", 1));	

	movieclip->play();

	return 0;
}

int MovieClipBinder::stop(lua_State* L)
{
	StackChecker checker(L, "MovieClipBinder::stop", 0);

	Binder binder(L);
	MovieClip* movieclip = static_cast<MovieClip*>(binder.getInstance("MovieClip", 1));	

	movieclip->stop();

	return 0;
}

int MovieClipBinder::gotoAndPlay(lua_State* L)
{
	StackChecker checker(L, "MovieClipBinder::gotoAndPlay", 0);

	Binder binder(L);
	MovieClip* movieclip = static_cast<MovieClip*>(binder.getInstance("MovieClip", 1));	

	int frame = luaL_checkinteger(L, 2);
	movieclip->gotoAndPlay(frame);

	return 0;
}

int MovieClipBinder::gotoAndStop(lua_State* L)
{
	StackChecker checker(L, "MovieClipBinder::gotoAndStop", 0);

	Binder binder(L);
	MovieClip* movieclip = static_cast<MovieClip*>(binder.getInstance("MovieClip", 1));	

	int frame = luaL_checkinteger(L, 2);
	movieclip->gotoAndStop(frame);

	return 0;
}

int MovieClipBinder::setStopAction(lua_State* L)
{
	StackChecker checker(L, "MovieClipBinder::setStopAction", 0);

	Binder binder(L);
	MovieClip* movieclip = static_cast<MovieClip*>(binder.getInstance("MovieClip", 1));

	int frame = luaL_checkinteger(L, 2);

	movieclip->setStopAction(frame);

	return 0;
}

int MovieClipBinder::setGotoAction(lua_State* L)
{
	StackChecker checker(L, "MovieClipBinder::setGotoAction", 0);

	Binder binder(L);
	MovieClip* movieclip = static_cast<MovieClip*>(binder.getInstance("MovieClip", 1));

	int frame = luaL_checkinteger(L, 2);
	int destframe = luaL_checkinteger(L, 3);

	movieclip->setGotoAction(frame, destframe);

	return 0;
}

int MovieClipBinder::clearAction(lua_State* L)
{
	StackChecker checker(L, "MovieClipBinder::clearAction", 0);

	Binder binder(L);
	MovieClip* movieclip = static_cast<MovieClip*>(binder.getInstance("MovieClip", 1));

	int frame = luaL_checkinteger(L, 2);

	movieclip->clearAction(frame);

	return 0;
}
