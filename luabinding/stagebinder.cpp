#include "stagebinder.h"
#include "application.h"
#include "stackchecker.h"
#include "stage.h"

StageBinder::StageBinder(lua_State* L, Application* application)
{
	StackChecker checker(L, "StageBinder::StageBinder()", 0);

	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"getOrientation", StageBinder::getOrientation},
		{"setOrientation", StageBinder::setOrientation},
		{"getClearColorBuffer", StageBinder::getClearColorBuffer},
		{"setClearColorBuffer", StageBinder::setClearColorBuffer},
		{"setBackgroundColor",  StageBinder::setBackgroundColor},
		{"getBackgroundColor",  StageBinder::getBackgroundColor},
		{NULL, NULL},
	};

	binder.createClass("Stage", "Sprite", 0, destruct, functionList);

	lua_getglobal(L, "Stage");	// get Stage metatable

	lua_pushstring(L, "portrait");
	lua_setfield(L, -2, "PORTRAIT");

	lua_pushstring(L, "portraitUpsideDown");
	lua_setfield(L, -2, "PORTRAIT_UPSIDE_DOWN");

	lua_pushstring(L, "landscapeLeft");
	lua_setfield(L, -2, "LANDSCAPE_LEFT");
	
	lua_pushstring(L, "landscapeRight");
	lua_setfield(L, -2, "LANDSCAPE_RIGHT");

	lua_pop(L, 1);

	binder.pushInstance("Stage", application->stage());
	application->stage()->ref();
	lua_setglobal(L, "stage");
}

int StageBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	Stage* stage = static_cast<Stage*>(ptr);
	stage->unref();

	return 0;
}


int StageBinder::getOrientation(lua_State* L)
{
	StackChecker checker(L, "getOrientation", 1);
	
	Binder binder(L);
	Stage* stage = static_cast<Stage*>(binder.getInstance("Stage"));

	switch (stage->application()->orientation())
	{
	case ePortrait:
	case eFixed:
		lua_getfield(L, 1, "PORTRAIT");
		break;
	case ePortraitUpsideDown:
		lua_getfield(L, 1, "PORTRAIT_UPSIDE_DOWN");
		break;
	case eLandscapeLeft:
		lua_getfield(L, 1, "LANDSCAPE_LEFT");
		break;
	case eLandscapeRight:
		lua_getfield(L, 1, "LANDSCAPE_RIGHT");
		break;
	}

	return 1;
}

int StageBinder::setOrientation(lua_State* L)
{
	StackChecker checker(L, "setOrientation", 0);

	Binder binder(L);
	Stage* stage = static_cast<Stage*>(binder.getInstance("Stage"));

	bool changed = false;
	
	lua_getfield(L, 1, "PORTRAIT");
	if (lua_equal(L, 2, -1))
	{
		stage->application()->setOrientation(ePortrait);
		changed = true;
	}
	lua_pop(L, 1);

	lua_getfield(L, 1, "PORTRAIT_UPSIDE_DOWN");
	if (lua_equal(L, 2, -1))
	{
		stage->application()->setOrientation(ePortraitUpsideDown);
		changed = true;
	}
	lua_pop(L, 1);

	lua_getfield(L, 1, "LANDSCAPE_LEFT");
	if (lua_equal(L, 2, -1))
	{
		stage->application()->setOrientation(eLandscapeLeft);
		changed = true;
	}
	lua_pop(L, 1);

	lua_getfield(L, 1, "LANDSCAPE_RIGHT");
	if (lua_equal(L, 2, -1))
	{
		stage->application()->setOrientation(eLandscapeRight);
		changed = true;
	}
	lua_pop(L, 1);

	if (changed == false)
	{
		// TODO: lua_error prints line number, file name and function name. Debug interface bize bunlari saglayabilir
		printf("Warning: bad argument #1 'setOrientation' (PORTRAIT or PORTRAIT_UPSIDE_DOWN or LANDSCAPE_LEFT or LANDSCAPE_RIGHT expected)\n");
	}
	
	return 0;
}

int StageBinder::getClearColorBuffer(lua_State* L)
{
	StackChecker checker(L, "StageBinder::getClearColorBuffer", 1);

	Binder binder(L);
	Stage* stage = static_cast<Stage*>(binder.getInstance("Stage", 1));

	bool b = stage->application()->getClearColorBuffer();
	lua_pushboolean(L, b);

	return 1;
}

int StageBinder::setClearColorBuffer(lua_State* L)
{
	StackChecker checker(L, "StageBinder::setClearColorBuffer", 0);

	Binder binder(L);
	Stage* stage = static_cast<Stage*>(binder.getInstance("Stage", 1));

	bool b = lua_toboolean(L, 2);
	stage->application()->setClearColorBuffer(b);

	return 0;
}

int StageBinder::setBackgroundColor(lua_State* L)
{
	StackChecker checker(L, "StageBinder::setBackgroundColor", 0);

	Binder binder(L);
	Stage* stage = static_cast<Stage*>(binder.getInstance("Stage", 1));

	lua_Number r = luaL_checknumber(L, 2);
	lua_Number g = luaL_checknumber(L, 3);
	lua_Number b = luaL_checknumber(L, 4);

	stage->application()->setBackgroundColor(r, g, b);

	return 0;
}

int StageBinder::getBackgroundColor(lua_State* L)
{
	StackChecker checker(L, "StageBinder::getBackgroundColor", 3);

	Binder binder(L);
	Stage* stage = static_cast<Stage*>(binder.getInstance("Stage", 1));

	float r, g, b;
	stage->application()->getBackgroundColor(&r, &g, &b);

	lua_pushnumber(L, r);
	lua_pushnumber(L, g);
	lua_pushnumber(L, b);

	return 3;
}
