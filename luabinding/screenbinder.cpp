#include "screenbinder.h"
#include <screen.h>
#include "luaapplication.h"
#include <luautil.h>
#include <string.h>
#include <transform.h>

ScreenBinder::ScreenBinder(lua_State *L)
{
    Binder binder(L);

    static const luaL_Reg functionList[] = {
        {"clear", clear},
        {"setContent", setContent},
        {"setSize",setSize},
        {"getSize",getSize},
        {"setPosition",setPosition},
        {"getPosition",getPosition},
        {"setState",setState},
        {"getState",getState},
        {"getMaxSize",getMaxSize},
        {"getId",getId},
        {NULL, NULL},
    };

    binder.createClass("Screen", NULL, create, destruct, functionList);
	lua_getglobal(L, "Screen");	// get Stage metatable

	lua_pushinteger(L, Screen::NORMAL);
	lua_setfield(L, -2, "STATE_NORMAL");
	lua_pushinteger(L, Screen::MINIMIZED);
	lua_setfield(L, -2, "STATE_MINIMIZED");
	lua_pushinteger(L, Screen::MAXIMIZED);
	lua_setfield(L, -2, "STATE_MAXIMIZED");
	lua_pushinteger(L, Screen::FULLSCREEN);
	lua_setfield(L, -2, "STATE_FULLSCREEN");
	lua_pushinteger(L, Screen::ACTIVE);
	lua_setfield(L, -2, "STATE_ACTIVE");

	lua_pop(L,1);
}


int ScreenBinder::create(lua_State *L)
{
    LuaApplication *application = static_cast<LuaApplication*>(luaL_getdata(L));

    Binder binder(L);

    int id = luaL_checkinteger(L, 1);
    Screen *sc=NULL;
    if (ScreenManager::manager)
    	sc=ScreenManager::manager->openScreen(application->getApplication(),id);
    if (sc)
    	binder.pushInstance("Screen", sc);
    else
    	lua_pushnil(L);
    return 1;
}

int ScreenBinder::destruct(lua_State *L)
{
    void* ptr = *(void**)lua_touserdata(L, 1);
    GReferenced* refptr = static_cast<GReferenced*>(ptr);
    refptr->unref();
    if (ScreenManager::manager)
    	ScreenManager::manager->screenDestroyed();

    return 0;
}

int ScreenBinder::clear(lua_State *L)
{
    Binder binder(L);

    Screen *renderTarget = static_cast<Screen*>(binder.getInstance("Screen", 1));
    unsigned int color = luaL_checkinteger(L, 2);
    float alpha = luaL_optnumber(L, 3, 1.0);

    renderTarget->clear(color, alpha);

    return 0;
}

int ScreenBinder::setContent(lua_State* L)
{
	Binder binder(L);
	Screen* shape = static_cast<Screen*>(binder.getInstance("Screen"));
	Sprite* s = static_cast<Sprite*>(binder.getInstance("Sprite", 2));
	shape->setContent(s);

	return 0;
}


int ScreenBinder::setSize(lua_State *L)
{
    Binder binder(L);
    Screen *screen = static_cast<Screen*>(binder.getInstance("Screen", 1));
    lua_Number x=luaL_checknumber(L,2);
    lua_Number y=luaL_checknumber(L,3);
    screen->setSize(x,y);
    return 0;
}

int ScreenBinder::getSize(lua_State *L)
{
    Binder binder(L);
    Screen *screen = static_cast<Screen*>(binder.getInstance("Screen", 1));
    int x,y;
    screen->getSize(x,y);
    lua_pushinteger(L,x);
    lua_pushinteger(L,y);
    return 2;
}

int ScreenBinder::setState(lua_State *L)
{
    Binder binder(L);
    Screen *screen = static_cast<Screen*>(binder.getInstance("Screen", 1));
    lua_Number s=luaL_checknumber(L,2);
    screen->setState(s);
    return 0;
}

int ScreenBinder::getState(lua_State *L)
{
    Binder binder(L);
    Screen *screen = static_cast<Screen*>(binder.getInstance("Screen", 1));
    int s=screen->getState();
    lua_pushinteger(L,s);
    return 1;
}

int ScreenBinder::setPosition(lua_State *L)
{
    Binder binder(L);
    Screen *screen = static_cast<Screen*>(binder.getInstance("Screen", 1));
    lua_Number x=luaL_checknumber(L,2);
    lua_Number y=luaL_checknumber(L,3);
    screen->setPosition(x,y);
    return 0;
}

int ScreenBinder::getPosition(lua_State *L)
{
    Binder binder(L);
    Screen *screen = static_cast<Screen*>(binder.getInstance("Screen", 1));
    int x,y;
    screen->getPosition(x,y);
    lua_pushinteger(L,x);
    lua_pushinteger(L,y);
    return 2;
}

int ScreenBinder::getMaxSize(lua_State *L)
{
    Binder binder(L);
    Screen *screen = static_cast<Screen*>(binder.getInstance("Screen", 1));
    int x,y;
    screen->getMaxSize(x,y);
    lua_pushinteger(L,x);
    lua_pushinteger(L,y);
    return 2;
}

int ScreenBinder::getId(lua_State *L)
{
    Binder binder(L);
    Screen *screen = static_cast<Screen*>(binder.getInstance("Screen", 1));
    lua_pushinteger(L,screen->getId());
    return 1;
}
