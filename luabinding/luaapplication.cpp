#include "luaapplication.h"

#include "eventdispatcher.h"

#include "platform.h"

#include "application.h"

#include "luautil.h"
#include "stackchecker.h"

#include "binder.h"
#include "eventbinder.h"
#include "eventdispatcherbinder.h"
#include "enterframeevent.h"
#include "spritebinder.h"
#include "matrixbinder.h"
#include "texturebasebinder.h"
#include "texturebinder.h"
#include "texturepackbinder.h"
#include "bitmapdatabinder.h"
#include "bitmapbinder.h"
#include "stagebinder.h"
#include "timerbinder.h"
#include "fontbasebinder.h"
#include "fontbinder.h"
#include "ttfontbinder.h"
#include "textfieldbinder.h"
#include "accelerometerbinder.h"
#include "box2dbinder2.h"
#include "dibbinder.h"
#include "applicationbinder.h"
#include "tilemapbinder.h"
#include "shapebinder.h"
#include "movieclipbinder.h"
#include "urlloaderbinder.h"
#include "geolocationbinder.h"
#include "gyroscopebinder.h"
#include "timer.h"
#include "alertdialogbinder.h"
#include "textinputdialogbinder.h"
#include "meshbinder.h"
#include "audiobinder.h"
#include "rendertargetbinder.h"
#include "stageorientationevent.h"
#include "shaderbinder.h"
#include "path2dbinder.h"
#include "viewportbinder.h"
#include "pixelbinder.h"
#include "particlesbinder.h"

#include "keys.h"

#include "ogl.h"

#include <algorithm>

#include <gfile.h>
#include <gfile_p.h>

#include <pluginmanager.h>

#include <keycode.h>

#include <glog.h>

#include <gevent.h>
#include <ginput.h>
#include <gapplication.h>

#include "tlsf.h"

std::deque<LuaApplication::AsyncLuaTask> LuaApplication::tasks_;

const char* LuaApplication::fileNameFunc_s(const char* filename, void* data)
{
	LuaApplication* that = static_cast<LuaApplication*>(data);
	return that->fileNameFunc(filename);
}

const char* LuaApplication::fileNameFunc(const char* filename)
{
	return g_pathForFile(filename);
}

/*
static const char* sound_lua = 
"function Sound:play(startTime, loops)"						"\n"
"	return SoundChannel.new(self, startTime, loops)"		"\n"
"end"														"\n"
;

static const char* texturepack_lua = 
"function TexturePack:getBitmapData(index)"					"\n"
"	local x, y, width, height"								"\n"
"	x, y, width, height = self:getLocation(index)"			"\n"
""															"\n"
"	if x == nil then"										"\n"
"		return nil"											"\n"
"	end"													"\n"
""															"\n"
"	return BitmapData.new(self, x, y, width, height)"		"\n"
"end"														"\n"
;

static const char* class_lua = 
"function class(b)"											"\n"
"	local c = {}"											"\n"
"	setmetatable(c, b)"										"\n"
"	c.__index = c"											"\n"
""															"\n"
"	c.new = function(...)"									"\n"
"		local s0 = b.new(...)"								"\n"
"		local s1 = {}"										"\n"
"		setmetatable(s1, c)"								"\n"
""															"\n"
"		for k,v in pairs(s0) do"							"\n"
"			s1[k] = v"										"\n"
"		end"												"\n"
""															"\n"
"		return s1"											"\n"
"	end"													"\n"
""															"\n"
"	return c"												"\n"
"end"														"\n"
;

static const char* class_v2_lua = 
"function class_v2(b)"										"\n"
"	local c = {}"											"\n"
"	setmetatable(c, b)"										"\n"
"	c.__index = c"											"\n"
""															"\n"
"	c.new = function(...)"									"\n"
"		local s0 = b.new(...)"								"\n"
"		setmetatable(s0, c)"								"\n"
"		s0.__index = s0"									"\n"
""															"\n"
"		local s1 = {}"										"\n"
"		setmetatable(s1, s0)"								"\n"
""															"\n"
"		return s1"											"\n"
"	end"													"\n"
""															"\n"
"	return c"												"\n"
"end"														"\n"
;
*/
static int environTable(lua_State* L)
{
	StackChecker checker(L, "environTable", 1);

	static char k = ' ';		// todo: maybe we address the table with this pointer

	lua_pushlightuserdata(L, &k);
	lua_rawget(L, LUA_REGISTRYINDEX);
	
	if (lua_isnil(L, -1) == 1)
	{
		lua_pop(L, 1);			// pop nil

		lua_pushlightuserdata(L, &k);
		lua_newtable(L);
		lua_rawset(L, LUA_REGISTRYINDEX);

		lua_pushlightuserdata(L, &k);
		lua_rawget(L, LUA_REGISTRYINDEX);
		
		//luaL_newweaktable(L);
		//lua_setfield(L, -2, "bridges");

		lua_newtable(L);
		lua_setfield(L, -2, "timers");

		lua_newtable(L);
		lua_setfield(L, -2, "soundchannels");
	}

	return 1;
}

int setEnvironTable(lua_State* L)
{
	return 0;

	StackChecker checker(L, "setEnvironTable", 0);

	environTable(L);
	lua_replace(L, LUA_ENVIRONINDEX);

	return 0;
}

static int os_timer(lua_State* L)
{
	lua_pushnumber(L, iclock());
	return 1;
}

static int instance_of(lua_State* L){
    if(g_isInstanceOf(L, luaL_checkstring(L, 2), 1))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

static int get_class(lua_State* L){
    lua_getfield(L, 1, "__classname");
    if (!lua_isstring(L, -1)){
        lua_pop(L, 1);
        lua_pushstring(L, "Object");
    }
    return 1;
}

static int get_base(lua_State* L){
    lua_getfield(L, 1, "__basename");
    if (!lua_isstring(L, -1)){
        lua_pop(L, 1);
        lua_pushstring(L, "Object");
    }
    return 1;
}

void registerModules(lua_State* L);

double LuaApplication::meanFrameTime_; //Average frame duration
double LuaApplication::meanFreeTime_; //Average time available for async tasks
unsigned long LuaApplication::frameCounter_; //Global frame counter

int LuaApplication::Core_frameStatistics(lua_State* L)
{
	lua_newtable(L);
	lua_pushnumber(L,meanFrameTime_);
	lua_setfield(L,-2,"meanFrameTime");
	lua_pushnumber(L,meanFreeTime_);
	lua_setfield(L,-2,"meanFreeTime");
	lua_pushinteger(L,frameCounter_);
	lua_setfield(L,-2,"frameCounter");
	return 1;
}

int LuaApplication::Core_yield(lua_State* L)
{
	for (std::deque<LuaApplication::AsyncLuaTask>::iterator it=LuaApplication::tasks_.begin();it!=LuaApplication::tasks_.end();++it)
		if ((*it).L==L)
		{
			if (lua_isboolean(L,1))
			{
				if (lua_toboolean(L,1))
				{
					(*it).skipFrame=true;
					(*it).autoYield=false;
				}
				else
				{
					(*it).autoYield=true;
				}
			}
			else
			{
				double sleep=luaL_optnumber(L,1,0);
				(*it).sleepTime=iclock()+sleep;
			}
		}
	return lua_yield(L,0);
}


int LuaApplication::Core_asyncCall(lua_State* L)
{
	LuaApplication::AsyncLuaTask t;
	lua_State *T=lua_newthread(L);
	lua_pushvalue(L,-1);
	t.taskRef=luaL_ref(L,LUA_REGISTRYINDEX);
	t.L=T;
	t.sleepTime=0;
	t.autoYield=true;
	int nargs=lua_gettop(L);
	luaL_loadstring(T,"local function _start_(fn,...) coroutine.yield() fn(...) end return _start_(...)");
	lua_xmove(L,T,nargs);
	if (lua_resume(T,nargs)!=LUA_YIELD)
	{
		lua_xmove(T,L,1);
		lua_error(L);
	}
	else
		LuaApplication::tasks_.push_back(t);
	return 1;
}


static int bindAll(lua_State* L)
{
	Application* application = static_cast<Application*>(lua_touserdata(L, 1));
	lua_pop(L, 1);

	StackChecker checker(L, "bindAll", 0);

	setEnvironTable(L);

	{
		lua_newtable(L);
		luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_touches);

		lua_newtable(L);
		luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_eventClosures);

		lua_newtable(L);
		luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_events);

		luaL_newweaktable(L);
		luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_b2);

		lua_newtable(L);
		luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_timers);
	}

    static const luaL_Reg functionList[] = {
        {"isInstanceOf", instance_of},
        {"getClass", get_class},
        {"getBaseClass", get_base},
        {NULL, NULL},
    };

    luaL_newmetatable(L, "Object");
    luaL_register(L, NULL, functionList);
    lua_setglobal(L, "Object");
	
	EventBinder eventBinder(L);
	EventDispatcherBinder eventDispatcherBinder(L);
	TimerBinder timerBinder(L);
	MatrixBinder matrixBinder(L);
	SpriteBinder spriteBinder(L);
	TextureBaseBinder textureBaseBinder(L);
	TextureBinder textureBinder(L);
	TexturePackBinder texturePackBinder(L);
	BitmapDataBinder bitmapDataBinder(L);
	BitmapBinder bitmapBinder(L);
	StageBinder stageBinder(L, application);
	FontBaseBinder fontBaseBinder(L);
	FontBinder fontBinder(L);
	TTFontBinder ttfontBinder(L);
	TextFieldBinder textFieldBinder(L);
#if TARGET_OS_TV == 0
    AccelerometerBinder accelerometerBinder(L);
#endif
	Box2DBinder2 box2DBinder2(L);
	DibBinder dibBinder(L);
	TileMapBinder tileMapBinder(L);
	ApplicationBinder applicationBinder(L);
	ShapeBinder shapeBinder(L);
	MovieClipBinder movieClipBinder(L);
    UrlLoaderBinder urlLoaderBinder(L);
#if TARGET_OS_TV == 0
	GeolocationBinder geolocationBinder(L);
	GyroscopeBinder gyroscopeBinder(L);
#endif
    AlertDialogBinder alertDialogBinder(L);
    TextInputDialogBinder textInputDialogBinder(L);
    MeshBinder meshBinder(L);
    AudioBinder audioBinder(L);
    RenderTargetBinder renderTargetBinder(L);
    ShaderBinder shaderBinder(L);
    Path2DBinder path2DBinder(L);
    ViewportBinder viewportBinder(L);
    PixelBinder pixelbinder(L);
    ParticlesBinder particlesbinder(L);

	PluginManager& pluginManager = PluginManager::instance();
	for (size_t i = 0; i < pluginManager.plugins.size(); ++i)
        pluginManager.plugins[i].main(L, 0);

	lua_getglobal(L, "Event");
	lua_getfield(L, -1, "new");
	lua_pushlightuserdata(L, NULL);
	lua_call(L, 1, 1);
	lua_remove(L, -2);
	luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_Event);

	lua_getglobal(L, "Event");
	lua_getfield(L, -1, "new");
	lua_pushlightuserdata(L, NULL);
	lua_call(L, 1, 1);
	lua_remove(L, -2);
	luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_EnterFrameEvent);

	lua_getglobal(L, "Event");
	lua_getfield(L, -1, "new");
	lua_pushlightuserdata(L, NULL);
	lua_call(L, 1, 1);
	lua_remove(L, -2);
	luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_MouseEvent);

	lua_getglobal(L, "Event");
	lua_getfield(L, -1, "new");
	lua_pushlightuserdata(L, NULL);
	lua_call(L, 1, 1);
	lua_remove(L, -2);
	luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_TouchEvent);


	lua_getglobal(L, "Event");
	lua_getfield(L, -1, "new");
	lua_pushlightuserdata(L, NULL);
	lua_call(L, 1, 1);
	lua_remove(L, -2);
	luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_TimerEvent);

	lua_getglobal(L, "Event");
	lua_getfield(L, -1, "new");
	lua_pushlightuserdata(L, NULL);
	lua_call(L, 1, 1);
	lua_remove(L, -2);
	luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_KeyboardEvent);

    lua_getglobal(L, "Event");
    lua_getfield(L, -1, "new");
    lua_pushlightuserdata(L, NULL);
    lua_call(L, 1, 1);
    lua_remove(L, -2);
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &key_CompleteEvent);

#include "property.c.in"
#include "texturepack.c.in"
#include "compatibility.c.in"

	lua_newtable(L);

    lua_pushinteger(L, GINPUT_KEY_BACK);
	lua_setfield(L, -2, "BACK");
    lua_pushinteger(L, GINPUT_KEY_SEARCH);
	lua_setfield(L, -2, "SEARCH");
    lua_pushinteger(L, GINPUT_KEY_MENU);
	lua_setfield(L, -2, "MENU");
    lua_pushinteger(L, GINPUT_KEY_CENTER);
	lua_setfield(L, -2, "CENTER");
    lua_pushinteger(L, GINPUT_KEY_SELECT);
	lua_setfield(L, -2, "SELECT");
    lua_pushinteger(L, GINPUT_KEY_START);
	lua_setfield(L, -2, "START");
    lua_pushinteger(L, GINPUT_KEY_L1);
	lua_setfield(L, -2, "L1");
    lua_pushinteger(L, GINPUT_KEY_R1);
	lua_setfield(L, -2, "R1");

    lua_pushinteger(L, GINPUT_KEY_LEFT);
	lua_setfield(L, -2, "LEFT");
    lua_pushinteger(L, GINPUT_KEY_UP);
	lua_setfield(L, -2, "UP");
    lua_pushinteger(L, GINPUT_KEY_RIGHT);
	lua_setfield(L, -2, "RIGHT");
    lua_pushinteger(L, GINPUT_KEY_DOWN);
	lua_setfield(L, -2, "DOWN");

    lua_pushinteger(L, GINPUT_KEY_A);
    lua_setfield(L, -2, "A");
    lua_pushinteger(L, GINPUT_KEY_B);
    lua_setfield(L, -2, "B");
    lua_pushinteger(L, GINPUT_KEY_C);
    lua_setfield(L, -2, "C");
    lua_pushinteger(L, GINPUT_KEY_D);
    lua_setfield(L, -2, "D");
    lua_pushinteger(L, GINPUT_KEY_E);
    lua_setfield(L, -2, "E");
    lua_pushinteger(L, GINPUT_KEY_F);
    lua_setfield(L, -2, "F");
    lua_pushinteger(L, GINPUT_KEY_G);
    lua_setfield(L, -2, "G");
    lua_pushinteger(L, GINPUT_KEY_H);
    lua_setfield(L, -2, "H");
    lua_pushinteger(L, GINPUT_KEY_I);
    lua_setfield(L, -2, "I");
    lua_pushinteger(L, GINPUT_KEY_J);
    lua_setfield(L, -2, "J");
    lua_pushinteger(L, GINPUT_KEY_K);
    lua_setfield(L, -2, "K");
    lua_pushinteger(L, GINPUT_KEY_L);
    lua_setfield(L, -2, "L");
    lua_pushinteger(L, GINPUT_KEY_M);
    lua_setfield(L, -2, "M");
    lua_pushinteger(L, GINPUT_KEY_N);
    lua_setfield(L, -2, "N");
    lua_pushinteger(L, GINPUT_KEY_O);
    lua_setfield(L, -2, "O");
    lua_pushinteger(L, GINPUT_KEY_P);
    lua_setfield(L, -2, "P");
    lua_pushinteger(L, GINPUT_KEY_Q);
    lua_setfield(L, -2, "Q");
    lua_pushinteger(L, GINPUT_KEY_R);
    lua_setfield(L, -2, "R");
    lua_pushinteger(L, GINPUT_KEY_S);
    lua_setfield(L, -2, "S");
    lua_pushinteger(L, GINPUT_KEY_T);
    lua_setfield(L, -2, "T");
    lua_pushinteger(L, GINPUT_KEY_U);
    lua_setfield(L, -2, "U");
    lua_pushinteger(L, GINPUT_KEY_V);
    lua_setfield(L, -2, "V");
    lua_pushinteger(L, GINPUT_KEY_W);
    lua_setfield(L, -2, "W");
    lua_pushinteger(L, GINPUT_KEY_X);
    lua_setfield(L, -2, "X");
    lua_pushinteger(L, GINPUT_KEY_Y);
    lua_setfield(L, -2, "Y");
    lua_pushinteger(L, GINPUT_KEY_Z);
    lua_setfield(L, -2, "Z");

    lua_pushinteger(L, GINPUT_KEY_0);
    lua_setfield(L, -2, "NUM_0");
    lua_pushinteger(L, GINPUT_KEY_1);
    lua_setfield(L, -2, "NUM_1");
    lua_pushinteger(L, GINPUT_KEY_2);
    lua_setfield(L, -2, "NUM_2");
    lua_pushinteger(L, GINPUT_KEY_3);
    lua_setfield(L, -2, "NUM_3");
    lua_pushinteger(L, GINPUT_KEY_4);
    lua_setfield(L, -2, "NUM_4");
    lua_pushinteger(L, GINPUT_KEY_5);
    lua_setfield(L, -2, "NUM_5");
    lua_pushinteger(L, GINPUT_KEY_6);
    lua_setfield(L, -2, "NUM_6");
    lua_pushinteger(L, GINPUT_KEY_7);
    lua_setfield(L, -2, "NUM_7");
    lua_pushinteger(L, GINPUT_KEY_8);
    lua_setfield(L, -2, "NUM_8");
    lua_pushinteger(L, GINPUT_KEY_9);
    lua_setfield(L, -2, "NUM_9");

	lua_pushinteger(L, GINPUT_KEY_SHIFT);
	lua_setfield(L, -2, "SHIFT");
	lua_pushinteger(L, GINPUT_KEY_SPACE);
	lua_setfield(L, -2, "SPACE");
	lua_pushinteger(L, GINPUT_KEY_BACKSPACE);
	lua_setfield(L, -2, "BACKSPACE");
	lua_pushinteger(L, GINPUT_KEY_CTRL);
	lua_setfield(L, -2, "CTRL");
	lua_pushinteger(L, GINPUT_KEY_ALT);
	lua_setfield(L, -2, "ALT");
	lua_pushinteger(L, GINPUT_KEY_ESC);
	lua_setfield(L, -2, "ESC");
	lua_pushinteger(L, GINPUT_KEY_TAB);
	lua_setfield(L, -2, "TAB");

    lua_pushinteger(L, GINPUT_NO_BUTTON);
    lua_setfield(L, -2, "MOUSE_NONE");
    lua_pushinteger(L, GINPUT_LEFT_BUTTON);
    lua_setfield(L, -2, "MOUSE_LEFT");
    lua_pushinteger(L, GINPUT_RIGHT_BUTTON);
    lua_setfield(L, -2, "MOUSE_RIGHT");
    lua_pushinteger(L, GINPUT_MIDDLE_BUTTON);
    lua_setfield(L, -2, "MOUSE_MIDDLE");

	lua_setglobal(L, "KeyCode");


	// correct clock function which is wrong in iphone
	lua_getglobal(L, "os");
	lua_pushcfunction(L, os_timer);
	lua_setfield(L, -2, "timer");
	lua_pop(L, 1);

	//coroutines helpers
	lua_getglobal(L, "Core");
	lua_pushcfunction(L, LuaApplication::Core_asyncCall);
	lua_setfield(L, -2, "asyncCall");
	lua_pushcfunction(L, LuaApplication::Core_yield);
	lua_setfield(L, -2, "yield");
	lua_pushcfunction(L, LuaApplication::Core_frameStatistics);
	lua_setfield(L, -2, "frameStatistics");
	lua_pop(L, 1);

	// register collectgarbagelater
//	lua_pushcfunction(L, ::collectgarbagelater);
//	lua_setglobal(L, "collectgarbagelater");

	registerModules(L);

	return 0;
}

LuaApplication::LuaApplication(void)
{
    L = luaL_newstate();
	printFunc_ = lua_getprintfunc(L);
    printData_ = NULL;
	lua_close(L);
	L = NULL;

	application_ = 0;

    isPlayer_ = true;

	exceptionsEnabled_ = true;

	orientation_ = ePortrait;

	width_ = 320;
	height_ = 480;
	scale_ = 1;

#if TARGET_OS_TV == 0
    ginput_addCallback(callback_s, this);
#endif
    gapplication_addCallback(callback_s, this);
}

void LuaApplication::callback_s(int type, void *event, void *udata)
{
    static_cast<LuaApplication*>(udata)->callback(type, event);
}

void LuaApplication::callback(int type, void *event)
{
    if (type == GINPUT_MOUSE_DOWN_EVENT)
    {
        ginput_MouseEvent *event2 = (ginput_MouseEvent*)event;
        application_->mouseDown(event2->x, event2->y, event2->button);
    }
    else if (type == GINPUT_MOUSE_MOVE_EVENT)
    {
        ginput_MouseEvent *event2 = (ginput_MouseEvent*)event;
        application_->mouseMove(event2->x, event2->y, event2->button);
    }
    else if (type == GINPUT_MOUSE_HOVER_EVENT)
    {
        ginput_MouseEvent *event2 = (ginput_MouseEvent*)event;
        application_->mouseHover(event2->x, event2->y, event2->button);
    }
    else if (type == GINPUT_MOUSE_UP_EVENT)
    {
        ginput_MouseEvent *event2 = (ginput_MouseEvent*)event;
        application_->mouseUp(event2->x, event2->y, event2->button);
    }
    else if (type == GINPUT_MOUSE_WHEEL_EVENT)
    {
        ginput_MouseEvent *event2 = (ginput_MouseEvent*)event;
        application_->mouseWheel(event2->x, event2->y, event2->wheel);
    }
    else if (type == GINPUT_KEY_DOWN_EVENT)
    {
        ginput_KeyEvent *event2 = (ginput_KeyEvent*)event;
        application_->keyDown(event2->keyCode, event2->realCode);
    }
    else if (type == GINPUT_KEY_UP_EVENT)
    {
        ginput_KeyEvent *event2 = (ginput_KeyEvent*)event;
        application_->keyUp(event2->keyCode, event2->realCode);
    }
    else if (type == GINPUT_KEY_CHAR_EVENT)
    {
        ginput_KeyEvent *event2 = (ginput_KeyEvent*)event;
        application_->keyChar(event2->charCode);
    }
    else if (type == GINPUT_TOUCH_BEGIN_EVENT)
    {
        ginput_TouchEvent *event2 = (ginput_TouchEvent*)event;
        application_->touchesBegin(event2);
    }
    else if (type == GINPUT_TOUCH_MOVE_EVENT)
    {
        ginput_TouchEvent *event2 = (ginput_TouchEvent*)event;
        application_->touchesMove(event2);
    }
    else if (type == GINPUT_TOUCH_END_EVENT)
    {
        ginput_TouchEvent *event2 = (ginput_TouchEvent*)event;
        application_->touchesEnd(event2);
    }
    else if (type == GINPUT_TOUCH_CANCEL_EVENT)
    {
        ginput_TouchEvent *event2 = (ginput_TouchEvent*)event;
        application_->touchesCancel(event2);
    }
    else if (type == GAPPLICATION_PAUSE_EVENT)
    {
        PluginManager& pluginManager = PluginManager::instance();
        for (size_t i = 0; i < pluginManager.plugins.size(); ++i)
            if (pluginManager.plugins[i].foreground)
                pluginManager.plugins[i].suspend(L);

        TimerContainer *timerContainer = application_->getTimerContainer();
        timerContainer->suspend();

        Event event(Event::APPLICATION_SUSPEND);
        application_->broadcastEvent(&event);
    }
    else if (type == GAPPLICATION_RESUME_EVENT)
    {

        TimerContainer *timerContainer = application_->getTimerContainer();
        timerContainer->resume();

        PluginManager& pluginManager = PluginManager::instance();
        for (size_t i = 0; i < pluginManager.plugins.size(); ++i)
            if (pluginManager.plugins[i].foreground)
                pluginManager.plugins[i].resume(L);

        Event event(Event::APPLICATION_RESUME);
        application_->broadcastEvent(&event);
    }
    else if (type == GAPPLICATION_BACKGROUND_EVENT)
    {
        PluginManager& pluginManager = PluginManager::instance();
        for (size_t i = 0; i < pluginManager.plugins.size(); ++i)
            if (pluginManager.plugins[i].foreground)
                pluginManager.plugins[i].background(L);

        Event event(Event::APPLICATION_BACKGROUND);
        application_->broadcastEvent(&event);
    }
    else if (type == GAPPLICATION_FOREGROUND_EVENT)
    {
        PluginManager& pluginManager = PluginManager::instance();
        for (size_t i = 0; i < pluginManager.plugins.size(); ++i)
            if (pluginManager.plugins[i].foreground)
                pluginManager.plugins[i].foreground(L);

        Event event(Event::APPLICATION_FOREGROUND);
        application_->broadcastEvent(&event);
    }
    else if (type == GAPPLICATION_OPEN_URL_EVENT)
    {
        gapplication_OpenUrlEvent *event2 = (gapplication_OpenUrlEvent*)event;

        PluginManager& pluginManager = PluginManager::instance();
        for (size_t i = 0; i < pluginManager.plugins.size(); ++i)
            if (pluginManager.plugins[i].openUrl)
                pluginManager.plugins[i].openUrl(L, event2->url);
    }
    else if (type == GAPPLICATION_MEMORY_LOW_EVENT)
    {
        Event event(Event::MEMORY_WARNING);
        application_->broadcastEvent(&event);

        lua_gc(L, LUA_GCCOLLECT, 0);
        lua_gc(L, LUA_GCCOLLECT, 0);
    }
    else if (type == GAPPLICATION_START_EVENT)
    {
        Event event(Event::APPLICATION_START);
        application_->broadcastEvent(&event);
    }
    else if (type == GAPPLICATION_EXIT_EVENT)
    {
        Event event(Event::APPLICATION_EXIT);
        application_->broadcastEvent(&event);
    }
    else if (type == GAPPLICATION_ORIENTATION_CHANGE_EVENT)
    {
        gapplication_OrientationChangeEvent *event2 = (gapplication_OrientationChangeEvent*)event;
        StageOrientationEvent event(StageOrientationEvent::ORIENTATION_CHANGE, event2->orientation);
        application_->broadcastEvent(&event);
    }
}

/*
static std::map<std::pair<std::string, int>, double> memmap;
static double lastmem = 0;

static void testHook(lua_State* L, lua_Debug* ar)
{
	double m = lua_gc(L, LUA_GCCOUNT, 0) + lua_gc(L, LUA_GCCOUNTB, 0) / 1024.0;
	double deltam = m - lastmem;
	lastmem = m;

	lua_getinfo(L, "S", ar);
//	printf("%s:%d %g\n", ar->source, ar->currentline, deltam);
	memmap[std::make_pair(std::string(ar->source), ar->currentline)] += deltam;
}


static void maxmem()
{
	std::vector<std::pair<double, std::pair<std::string, int> > > v;

	std::map<std::pair<std::string, int>, double>::iterator iter;
	for (iter = memmap.begin(); iter != memmap.end(); ++iter)
		v.push_back(std::make_pair(iter->second, iter->first));

	std::sort(v.begin(), v.end());
	std::reverse(v.begin(), v.end());

	for (std::size_t i = 0; i < v.size(); ++i)
	{
		printf("%s:%d %g\n", v[i].second.first.c_str(), v[i].second.second, v[i].first);
		if (i == 5)
			break;
	}

	memmap.clear();
}
*/


static tlsf_t memory_pool = NULL;
static void *memory_pool_end = NULL;

static void g_free(void *ptr)
{
    if (memory_pool <= ptr && ptr < memory_pool_end)
        tlsf_free(memory_pool, ptr);
    else
        ::free(ptr);
}

static void *g_realloc(void *ptr, size_t osize, size_t size)
{
    void* p = NULL;

    if (ptr && size == 0)
    {
        g_free(ptr);
    }
    else if (ptr == NULL)
    {
        if (size <= 256)
            p = tlsf_malloc(memory_pool, size);

        if (p == NULL)
            p = ::malloc(size);
    }
    else
    {
        if (memory_pool <= ptr && ptr < memory_pool_end)
        {
            if (size <= 256)
                p = tlsf_realloc(memory_pool, ptr, size);

            if (p == NULL)
            {
                p = ::malloc(size);
                memcpy(p, ptr, osize);
                tlsf_free(memory_pool, ptr);
            }
        }
        else
        {
            p = ::realloc(ptr, size);
        }
    }

    return p;
}

#if EMSCRIPTEN //TLSF has issues with emscripten, disable til I know more...
static void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
    (void)ud;
    (void)osize;
    if (nsize == 0)
    {
        free(ptr);
        return NULL;
    }
    else
        return realloc(ptr, nsize);
}
#else
static void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
    if (memory_pool == NULL)
    {
        const size_t mpsize = 1024 * 1024;
        glog_v("init_memory_pool: %dKb", mpsize / 1024);
        memory_pool = tlsf_create_with_pool(malloc(mpsize), mpsize);
        memory_pool_end = (char*)memory_pool + mpsize;
    }

    if (nsize == 0)
    {
        g_free(ptr);
        return NULL;
    }
    else
        return g_realloc(ptr, osize, nsize);
}
#endif

//int renderScene(lua_State* L);
//int mouseDown(lua_State* L);
//int mouseMove(lua_State* L);
//int mouseUp(lua_State* L);
//int touchesBegan(lua_State* L);
//int touchesMoved(lua_State* L);
//int touchesEnded(lua_State* L);
//int touchesCancelled(lua_State* L);



static int callFile(lua_State* L)
{
	StackChecker checker(L, "callFile", -1);

	setEnvironTable(L);

	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_events);
	luaL_nullifytable(L, -1);
	lua_pop(L, 1);

	lua_call(L, 0, 0);

	return 0;
}

void LuaApplication::loadFile(const char* filename, GStatus *status)
{
    StackChecker checker(L, "loadFile", 0);

    void *pool = application_->createAutounrefPool();

    lua_pushcfunction(L, ::callFile);

    if (luaL_loadfile(L, filename))
	{
		if (exceptionsEnabled_ == true)
		{
            if (status)
                *status = GStatus(1, lua_tostring(L, -1));
		}
        lua_pop(L, 2);
        application_->deleteAutounrefPool(pool);
        return;
	}

    if (lua_pcall_traceback(L, 1, 0, 0))
	{
		if (exceptionsEnabled_ == true)
		{
            if (status)
                *status = GStatus(1, lua_tostring(L, -1));
		}
        lua_pop(L, 1);
        application_->deleteAutounrefPool(pool);
        return;
    }
    application_->deleteAutounrefPool(pool);
}


LuaApplication::~LuaApplication(void)
{
//	Referenced::emptyPool();
#if TARGET_OS_TV == 0
    ginput_removeCallback(callback_s, this);
#endif
    gapplication_removeCallback(callback_s, this);
}

void LuaApplication::deinitialize()
{
	/*
		Application icindeki stage'in iki tane sahibi var.
		1-Application
		2-Lua

		stage en son unref yapanin (dolayisiyla silinmesine neden olanin) Lua olmasi onemli.
		bu nedenle ilk once 
		1-application_->releaseView(); diyoruz (bu stage_->unref() yapiyor) sonra
		2-lua_close(L) diyoruz
	*/

//	Referenced::emptyPool();

//	SoundContainer::instance().stopAllSounds();

	application_->releaseView();

	//Release all async tasks
	for (std::deque<LuaApplication::AsyncLuaTask>::iterator it=LuaApplication::tasks_.begin();it!=LuaApplication::tasks_.end();++it)
		luaL_unref(L,LUA_REGISTRYINDEX,(*it).taskRef);
	tasks_.clear();

	PluginManager& pluginManager = PluginManager::instance();
	for (size_t i = 0; i < pluginManager.plugins.size(); ++i)
        pluginManager.plugins[i].main(L, 1);

	lua_close(L);
	L = NULL;

	delete application_;
    application_ = NULL;

    clearError();
//	Referenced::emptyPool();
}


#ifndef abs_index

/* convert a stack index to positive */
#define abs_index(L, i)		((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : \
	lua_gettop(L) + (i) + 1)

#endif

static int tick(lua_State *L)
{
    gevent_Tick();
    return 0;
}

static int enterFrame(lua_State* L)
{
    StackChecker checker(L, "enterFrame", 0);

    LuaApplication *luaApplication = static_cast<LuaApplication*>(luaL_getdata(L));
    Application *application = luaApplication->getApplication();

	setEnvironTable(L);

	luaL_rawgetptr(L, LUA_REGISTRYINDEX, &key_events);
	luaL_nullifytable(L, -1);
	lua_pop(L, 1);

    gevent_Tick();

    PluginManager& pluginManager = PluginManager::instance();
    for (size_t i = 0; i < pluginManager.plugins.size(); ++i)
        if (pluginManager.plugins[i].enterFrame)
            pluginManager.plugins[i].enterFrame(L);

    application->enterFrame();

	return 0;
}

void LuaApplication::tick(GStatus *status)
{
    void *pool = application_->createAutounrefPool();

    lua_pushlightuserdata(L, &key_tickFunction);
    lua_rawget(L, LUA_REGISTRYINDEX);

    if (lua_pcall_traceback(L, 0, 0, 0))
    {
        if (exceptionsEnabled_ == true)
        {
            if (status)
                *status = GStatus(1, lua_tostring(L, -1));
        }
        lua_pop(L, 1);
    }

    application_->deleteAutounrefPool(pool);
}

static double yieldHookLimit;

static void yieldHook(lua_State *L,lua_Debug *ar)
{
	//glog_i("YieldHook:%f %f\n",iclock(),yieldHookLimit);
	if (ar->event == LUA_HOOKRET)
	{
		if (iclock() >= yieldHookLimit)
			lua_sethook(L, yieldHook, LUA_MASKCOUNT, 1);
	}
	else if (ar->event == LUA_HOOKCOUNT)
	{
		if (iclock() >= yieldHookLimit)
		{
			if (lua_canyield(L))
				lua_yield(L, 0);
			else
				lua_sethook(L, yieldHook, LUA_MASKRET | LUA_MASKCOUNT, 1000);
		}
	}
}

void LuaApplication::enterFrame(GStatus *status)
{
	if (!frameStartTime_)
	    frameStartTime_=iclock();

	frameCounter_++;

    void *pool = application_->createAutounrefPool();

	StackChecker checker(L, "enterFrame", 0);

    lua_pushlightuserdata(L, &key_enterFrameFunction);
	lua_rawget(L, LUA_REGISTRYINDEX);

    if (lua_pcall_traceback(L, 0, 0, 0))
	{
		if (exceptionsEnabled_ == true)
		{
            if (status)
                *status = GStatus(1, lua_tostring(L, -1));
        }
        lua_pop(L, 1);
    }

    //Schedule Tasks, at least one task should be runn no matter if there is enough time or not
	if ((meanFrameTime_ >= 0.01)&&(meanFrameTime_<=0.1)) //If frame rate is between 10Hz and 100Hz
	{
		double taskStart = iclock();
		double timeLimit = taskStart + meanFreeTime_*0.9; //Limit ourselves t 90% of free time
		yieldHookLimit = timeLimit;
		int loops = 0;
		while (!tasks_.empty())
		{
			AsyncLuaTask t = tasks_.front();
			tasks_.pop_front();
			tasks_.push_back(t);
			if ((t.sleepTime > iclock()) || (t.skipFrame))
			{
				loops++;
				if (loops > tasks_.size())
					break;
				continue;
			}
			loops = 0;
			int res = 0;
			if (t.autoYield)
			{
				lua_sethook(t.L, yieldHook, LUA_MASKRET | LUA_MASKCOUNT, 1000);
				res = lua_resume(t.L, 0);
				lua_sethook(t.L, yieldHook, 0, 1000);
			}
			else
				res = lua_resume(t.L, 0);
			if (res == LUA_YIELD)
			{ /* Yielded: Do nothing */
			}
			else if (res != 0)
			{
				tasks_.pop_back(); //Error: Dequeue
				if (exceptionsEnabled_ == true)
				{
					if (status)
						*status = GStatus(1, lua_tostring(t.L, -1));
				}
				lua_pop(t.L, 1);
				luaL_unref(L, LUA_REGISTRYINDEX, t.taskRef);
				break;
			}
			else
			{
				tasks_.pop_back(); //Ended: Dequeue
				//Drop any return args
				lua_settop(t.L, 0);
				luaL_unref(L, LUA_REGISTRYINDEX, t.taskRef);
			}
			if (iclock() > timeLimit)
				break;
		}

		for (std::deque<LuaApplication::AsyncLuaTask>::iterator it = LuaApplication::tasks_.begin(); it != LuaApplication::tasks_.end(); ++it)
			(*it).skipFrame = false;
		taskFrameTime_ = iclock() - taskStart;
	}
	else
		taskFrameTime_ = 0;
    application_->deleteAutounrefPool(pool);
}

void LuaApplication::clearBuffers()
{
	if (!frameStartTime_)
	    frameStartTime_=iclock();
	application_->clearBuffers();
}

void LuaApplication::renderScene(int deltaFrameCount)
{
	application_->renderScene();

	//Compute frame timings
    double frmEnd=iclock();
    double frmLasted=frmEnd-lastFrameTime_;
    if ((frmLasted>=0.01)&&(frmLasted<0.1)) //If frame rate is between 10Hz and 100Hz
    	meanFrameTime_=meanFrameTime_*0.8+frmLasted*0.2; //Average on 5 frames
    lastFrameTime_=frmEnd;

    double freeTime=meanFrameTime_-(frmEnd-frameStartTime_-taskFrameTime_);
    if (freeTime>=0)
    	meanFreeTime_=meanFreeTime_*0.8+freeTime*0.2; //Average on 5 frames
	//glog_i("FrameTimes:last:%f mean:%f task:%f free:%f\n",frmLasted,meanFrameTime_,taskFrameTime_,meanFreeTime_);

	frameStartTime_=0;
}

void LuaApplication::setPlayerMode(bool isPlayer)
{
    isPlayer_ = isPlayer;
}


bool LuaApplication::isPlayerMode()
{
    return isPlayer_;
}

lua_PrintFunc LuaApplication::getPrintFunc(void)
{
	return printFunc_;
}

void LuaApplication::setPrintFunc(lua_PrintFunc func, void *data)
{
    printFunc_ = func;
    printData_ = data;
	if (L)
        lua_setprintfunc(L, printFunc_, printData_);
}

void LuaApplication::enableExceptions()
{
	exceptionsEnabled_ = true;
}

void LuaApplication::disableExceptions()
{
	exceptionsEnabled_ = false;
}


void LuaApplication::setHardwareOrientation(Orientation orientation)
{
	orientation_ = orientation;
	application_->setHardwareOrientation(orientation_);
}


void LuaApplication::setResolution(int width, int height)
{
	width_ = width;
	height_ = height;

	application_->setResolution(width_, height_);
}


struct Touches_p
{
	Touches_p(const std::vector<Touch*>& touches, const std::vector<Touch*>& allTouches) :
		touches(touches),
		allTouches(allTouches)
	{

	}
	
	const std::vector<Touch*>& touches;
	const std::vector<Touch*>& allTouches;
};


/*
void LuaApplication::broadcastApplicationDidFinishLaunching()
{
	Event event(Event::APPLICATION_DID_FINISH_LAUNCHING);
	broadcastEvent(&event);
}

void LuaApplication::broadcastApplicationWillTerminate()
{
	Event event(Event::APPLICATION_WILL_TERMINATE);
	broadcastEvent(&event);
}

void LuaApplication::broadcastMemoryWarning()
{
	Event event(Event::MEMORY_WARNING);
	broadcastEvent(&event);
}
*/


static int broadcastEvent(lua_State* L)
{
    Event* event = static_cast<Event*>(lua_touserdata(L, 1));
    lua_pop(L, 1);				// TODO: event system requires an empty stack, preferibly correct this silly requirement

	setEnvironTable(L);

	EventDispatcher::broadcastEvent(event);

	return 0;
}

void LuaApplication::broadcastEvent(Event* event, GStatus *status)
{
    void *pool = application_->createAutounrefPool();

	lua_pushcfunction(L, ::broadcastEvent);
	lua_pushlightuserdata(L, event);

    if (lua_pcall_traceback(L, 1, 0, 0))
	{
		if (exceptionsEnabled_ == true)
		{
            if (status)
                *status = GStatus(1, lua_tostring(L, -1));
		}
        lua_pop(L, 1);
	}

    application_->deleteAutounrefPool(pool);
}

/*
static int orientationChange(lua_State* L)
{
	Application* application = static_cast<Application*>(lua_touserdata(L, 1));
	Orientation orientation = static_cast<Orientation>(lua_tointeger(L, 2));
	lua_pop(L, 2);				// TODO: event system requires an empty stack, preferibly correct this silly requirement

	setEnvironTable(L);

	application->orientationChange(orientation);

	return 0;
}

void LuaApplication::orientationChange(Orientation orientation)
{
	lua_pushcfunction(L, ::orientationChange);
	lua_pushlightuserdata(L, application_);
	lua_pushinteger(L, orientation);

	if (lua_pcall_traceback(L, 2, 0, 0))
	{
		if (exceptionsEnabled_ == true)
		{
			LuaException exception(LuaException::eRuntimeError, lua_tostring(L, -1));
			lua_pop(L, lua_gettop(L)); // we always clean the stack when there is an error
			throw exception;
		}
		else
		{
			lua_pop(L, lua_gettop(L)); // we always clean the stack when there is an error
		}
	}
}
*/
bool LuaApplication::isInitialized() const
{
	return application_ != NULL;
}

void LuaApplication::initialize()
{
    clearError();

	physicsScale_ = 30;

	application_ = new Application;

	application_->setHardwareOrientation(orientation_);
	application_->setResolution(width_, height_);
	application_->setScale(scale_);
	meanFrameTime_=0;
	meanFreeTime_=0;
	frameCounter_=0;

#if defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define ARCH_X64 1
#else
#define ARCH_X64 0
#endif

    if (ARCH_X64 && lua_isjit())
        L = luaL_newstate();
    else
        L = lua_newstate(l_alloc, NULL);

    lua_pushlightuserdata(L, &key_tickFunction);
    lua_pushcfunction(L, ::tick);
    lua_rawset(L, LUA_REGISTRYINDEX);

    lua_pushlightuserdata(L, &key_enterFrameFunction);
	lua_pushcfunction(L, ::enterFrame);
	lua_rawset(L, LUA_REGISTRYINDEX);

#if 0
	if (L == NULL)
	{
		fprintf(stderr, "cannot create Lua state\n");
		return;
	}
#endif

	application_->initView();
	if (ShaderEngine::Engine)
		ShaderEngine::Engine->reset();


    lua_setprintfunc(L, printFunc_, printData_);
    luaL_setdata(L, this);

	luaL_openlibs(L);

	//	lua_sethook(L, testHook, LUA_MASKLINE, 0);

	lua_pushcfunction(L, bindAll);
	lua_pushlightuserdata(L, application_);
	lua_call(L, 1, 0);
}

void LuaApplication::setScale(float scale)
{
	scale_ = scale;
	application_->setScale(scale_);
}

void LuaApplication::setLogicalDimensions(int width, int height)
{
	application_->setLogicalDimensions(width, height);
}

void LuaApplication::setLogicalScaleMode(LogicalScaleMode mode)
{
	application_->setLogicalScaleMode(mode);
}

int LuaApplication::getLogicalWidth() const
{
	return application_->getLogicalWidth();
}
int LuaApplication::getLogicalHeight() const
{
	return application_->getLogicalHeight();
}
int LuaApplication::getHardwareWidth() const
{
	return application_->getHardwareWidth();
}
int LuaApplication::getHardwareHeight() const
{
	return application_->getHardwareHeight();
}

void LuaApplication::setImageScales(const std::vector<std::pair<std::string, float> >& imageScales)
{
	application_->setImageScales(imageScales);
}

const std::vector<std::pair<std::string, float> >& LuaApplication::getImageScales() const
{
	return application_->getImageScales();
}

void LuaApplication::setOrientation(Orientation orientation)
{
	application_->setOrientation(orientation);
}

Orientation LuaApplication::orientation() const
{
	return application_->orientation();
}

void LuaApplication::addTicker(Ticker* ticker)
{
	application_->addTicker(ticker);
}

void LuaApplication::removeTicker(Ticker* ticker)
{
	application_->removeTicker(ticker);
}

float LuaApplication::getLogicalTranslateX() const
{
	return application_->getLogicalTranslateX();
}

float LuaApplication::getLogicalTranslateY() const
{
	return application_->getLogicalTranslateY();
}

float LuaApplication::getLogicalScaleX() const
{
	return application_->getLogicalScaleX();
}

float LuaApplication::getLogicalScaleY() const
{
	return application_->getLogicalScaleY();
}

void LuaApplication::setError(const char* error)
{
    error_ = error;
}

bool LuaApplication::isErrorSet() const
{
    return !error_.empty();
}

const char* LuaApplication::getError() const
{
    return error_.c_str();
}

void LuaApplication::clearError()
{
    error_.clear();
}

lua_State *LuaApplication::getLuaState() const
{
    return L;
}
