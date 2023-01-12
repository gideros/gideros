#include "luaapplication.h"

#include "eventdispatcher.h"

#include "platform.h"

#include "application.h"
#include "stage.h"

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
#include "screenbinder.h"
#include "bufferbinder.h"
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
#include "debugging.h"

#include "tlsf.h"
#include "CoreRandom.cpp.inc"
#include "memcache.cpp.inc"
#include <mutex>
#include <functional>

std::deque<LuaApplication::AsyncLuaTask> LuaApplication::tasks_;
int LuaApplication::debuggerBreak=0;
std::map<int,bool> LuaApplication::breakpoints;
void (*LuaApplication::debuggerHook)(void *context,lua_State *L,lua_Debug *ar)=NULL;
void *LuaApplication::debuggerContext=NULL;
std::mutex LuaApplication::taskLock;
std::condition_variable LuaApplication::frameWake;
bool LuaApplication::taskStopping=false;

static g_id luaCbGid=g_NextId();
static lua_State *globalLuaState=NULL;

static void LuaCallback_s(int type, void *event, void *udata)
{
	lua_State* L = globalLuaState;
	if (L == NULL)
		return;

	lua_getref(L,type);
	lua_unref(L,type);
	if (lua_type(L,-1)==LUA_TFUNCTION) {
		int nret=((gapplication_LuaArgPusher)udata)(L,event);
		lua_call(L, nret, 0);
	}
	else
		lua_pop(L,1);
}

void gapplication_luaCallback(int luaFuncRef,void *data,gapplication_LuaArgPusher pusher)
{
	gevent_EnqueueEvent(luaCbGid, LuaCallback_s, luaFuncRef, data,1,(void *)pusher);
}

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
    taskLock.lock();
    std::deque<LuaApplication::AsyncLuaTask>::iterator it=LuaApplication::tasks_.begin();
    while ((it!=LuaApplication::tasks_.end())&&(it->L!=L)) it++;

    if ((it!=LuaApplication::tasks_.end())&&(it->th)) {
        bool shouldStop=taskStopping;
        taskLock.unlock();
        if (shouldStop) {
            lua_pushstring(L,"System stopping");
            lua_error(L);
        }
        //AsyncThread, just sleep
        if (lua_isboolean(L,1)||lua_isnoneornil(L,1))
    	{
    		if (lua_toboolean(L,1))
    		{
    			std::unique_lock<std::mutex> lk(taskLock);
                lua_enableThreads(L,-1);
                frameWake.wait(lk);
                lua_enableThreads(L,1);
            }
            else {
                lua_enableThreads(L,-1);
                std::this_thread::yield();
                lua_enableThreads(L,1);
            }
        }
    	else {
            long long sleep=1000*luaL_checknumber(L,1);
            lua_enableThreads(L,-1);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
            lua_enableThreads(L,1);
        }

        return 0;
    }
    taskLock.unlock();
    //AsyncTask, must be the first of the list anyhow, and shouldn't change
    it=LuaApplication::tasks_.begin();
	if ((it==LuaApplication::tasks_.end())||(it->L!=L)) {
		lua_pushstring(L,"Core.yield must be called from an async Call");
		lua_error(L);
		return 0; //Yield only applies to asyncThreads
	}
	it->sleepTime=0;
	it->autoYield=false;
	it->skipFrame=false;
	if (lua_isboolean(L,1))
	{
		if (lua_toboolean(L,1))
			it->skipFrame=true;
        else {
			it->autoYield=true;
            if (lua_toboolean(L,2))
                return 0;
        }
	}
	else
	{
		double sleep=luaL_optnumber(L,1,0);
		it->sleepTime=iclock()+sleep;
	}
	return lua_yield(L,0);
}

int LuaApplication::Core_yieldable(lua_State* L)
{
    taskLock.lock();
    std::deque<LuaApplication::AsyncLuaTask>::iterator it=LuaApplication::tasks_.begin();
    while ((it!=LuaApplication::tasks_.end())&&(it->L!=L)) it++;
    bool isThread=(it!=LuaApplication::tasks_.end());
    lua_pushboolean(L,lua_isyieldable(L));
    lua_pushboolean(L,isThread);
    lua_pushboolean(L,isThread&&(it->autoYield||it->th));
    lua_pushboolean(L,isThread&&(it->th));
    taskLock.unlock();
    return 4;
}

#include "lstate.h"
int LuaApplication::Core_asyncCall(lua_State* L)
{
	LuaApplication::AsyncLuaTask t;
    luaL_checktype(L,1,LUA_TFUNCTION);
    int nargs=lua_gettop(L);
    lua_State *T=lua_newthread(L);
	t.taskRef=luaL_ref(L,LUA_REGISTRYINDEX);
    t.th=nullptr;
	t.L=T;
	t.sleepTime=0;
	t.skipFrame=false;
	t.autoYield=true;
    t.nargs=nargs-1;
    t.terminated=false;
    t.inError=false;
    T->profilerHook=L->profilerHook;
    T->profileTableAllocs=L->profileTableAllocs;
    lua_xmove(L,T,nargs);
    taskLock.lock();
    LuaApplication::tasks_.push_back(t);
    taskLock.unlock();
    lua_rawgeti(L, LUA_REGISTRYINDEX, t.taskRef);
	return 1;
}

static int Signal_wait(lua_State *L) {
    std::condition_variable *sig=(std::condition_variable *) luaL_checkudata(L,1,"Signal");
    double dur=luaL_optnumber(L,2,0);
    bool hasPredicate=(lua_type(L,3)==LUA_TFUNCTION);
    LuaApplication::taskLock.lock();
    std::deque<LuaApplication::AsyncLuaTask>::iterator it=LuaApplication::tasks_.begin();
    while ((it!=LuaApplication::tasks_.end())&&(it->L!=L)) it++;
    if ((it==LuaApplication::tasks_.end())||(!it->th)) {
        LuaApplication::taskLock.unlock();
        lua_pushstring(L,"Only parallel threads can wait on signals");
        lua_error(L);
    }
    bool shouldStop=LuaApplication::taskStopping;
    LuaApplication::taskLock.unlock();
    if (shouldStop) {
        lua_pushstring(L,"System stopping");
        lua_error(L);
    }
    bool ret=true;
    if (sig) {
        std::unique_lock<std::mutex> lk(LuaApplication::taskLock);
        lua_enableThreads(L,-1);
        std::function<bool()> p=[=]{
    		if (LuaApplication::taskStopping) return true;
    		if (!hasPredicate) return false;
    		lua_pushvalue(L,3);
    		lua_call(L,0,1);
    		bool exit=lua_toboolean(L,-1);
    		lua_pop(L,1);
    		return exit;
        };
        if (dur==0)
            sig->wait(lk,p);
        else
            ret=sig->wait_for(lk,std::chrono::milliseconds((int)(dur*1000)),p);
        lua_enableThreads(L,1);
        if (LuaApplication::taskStopping) {
            lua_pushstring(L,"System stopping");
            lua_error(L);
        }
    }
    lua_pushboolean(L,ret);
    return 1;
}

static int Signal_notify(lua_State *L) {
    std::condition_variable *sig=(std::condition_variable *) luaL_checkudata(L,1,"Signal");
    sig->notify_all();
    return 0;
}

int LuaApplication::Core_signal(lua_State* L)
{
    std::condition_variable *sig=(std::condition_variable *)lua_newuserdata(L,sizeof(std::condition_variable));
    new (sig) std::condition_variable();
    lua_getfield(L, LUA_REGISTRYINDEX, "Signal");
    lua_setmetatable(L,-2);
    return 1;
}

void LuaApplication::runThread(lua_State *L)
{
    int nargs=0;
    taskLock.lock();
    for(auto it=tasks_.begin();it!=tasks_.end();it++) {
        if (it->L==L) nargs=it->nargs;
    }
    taskLock.unlock();
    bool err=lua_pcall_traceback(L,nargs, 0, 0);
    taskLock.lock();
    for(auto it=tasks_.begin();it!=tasks_.end();it++) {
        if (it->L==L) {
            it->inError=err;
            it->terminated=true;
        }
    }
    taskLock.unlock();
    lua_enableThreads(L,-1);
}

int LuaApplication::Core_asyncThread(lua_State* L)
{
    LuaApplication::AsyncLuaTask t;
    luaL_checktype(L,1,LUA_TFUNCTION);
    int nargs=lua_gettop(L);
    lua_State *T=lua_newthread(L);
    t.taskRef=luaL_ref(L,LUA_REGISTRYINDEX);
    t.L=T;
    t.sleepTime=0;
    t.skipFrame=false;
    t.autoYield=false;
    t.nargs=nargs-1;
    t.terminated=false;
    t.inError=false;
    lua_xmove(L,T,nargs);
    lua_rawgeti(L, LUA_REGISTRYINDEX, t.taskRef);
    lua_enableThreads(L,1);
    taskLock.lock();
    t.th=new std::thread(runThread,T);
    LuaApplication::tasks_.push_back(t);
    taskLock.unlock();
    return 1;
}

#include "luabindings.luac.c"
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
	CompositeFontBinder compositeFontBinder(L);
	TTFontBinder ttfontBinder(L);
	TextFieldBinder textFieldBinder(L);
    TexturePackFontBinder texturePackFontBinder(L);
#if TARGET_OS_TV == 0
    AccelerometerBinder accelerometerBinder(L);
#endif
	//Box2DBinder2 box2DBinder2(L);
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
    ScreenBinder screenbinder(L);
    BufferBinder bufferbinder(L);

    int lres=luaL_loadbuffer(L,(const char *)luabinding_luabindings_luac,sizeof(luabinding_luabindings_luac),"internals");
#ifdef LUA_IS_LUAU
    if (lres<0) {
    	for (int k=1;k<=-lres;k++) {
			lua_rawgeti(L,-1,k);
	        lua_call(L, 0, 0);
    	}
    	lua_pop(L,1);
    }
#else
    if (lres==0)
        lua_call(L, 0, 0);
#endif

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

    lua_pushinteger(L, GINPUT_KEY_F1);
    lua_setfield(L, -2, "F1");
    lua_pushinteger(L, GINPUT_KEY_F2);
    lua_setfield(L, -2, "F2");
    lua_pushinteger(L, GINPUT_KEY_F3);
    lua_setfield(L, -2, "F3");
    lua_pushinteger(L, GINPUT_KEY_F4);
    lua_setfield(L, -2, "F4");
    lua_pushinteger(L, GINPUT_KEY_F5);
    lua_setfield(L, -2, "F5");
    lua_pushinteger(L, GINPUT_KEY_F6);
    lua_setfield(L, -2, "F6");
    lua_pushinteger(L, GINPUT_KEY_F7);
    lua_setfield(L, -2, "F7");
    lua_pushinteger(L, GINPUT_KEY_F8);
    lua_setfield(L, -2, "F8");
    lua_pushinteger(L, GINPUT_KEY_F9);
    lua_setfield(L, -2, "F9");
    lua_pushinteger(L, GINPUT_KEY_F10);
    lua_setfield(L, -2, "F10");
    lua_pushinteger(L, GINPUT_KEY_F11);
    lua_setfield(L, -2, "F11");
    lua_pushinteger(L, GINPUT_KEY_F12);
    lua_setfield(L, -2, "F12");

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
	lua_pushinteger(L, GINPUT_KEY_DELETE);
	lua_setfield(L, -2, "DELETE");
	lua_pushinteger(L, GINPUT_KEY_INSERT);
	lua_setfield(L, -2, "INSERT");
	lua_pushinteger(L, GINPUT_KEY_END);
	lua_setfield(L, -2, "END");
	lua_pushinteger(L, GINPUT_KEY_HOME);
	lua_setfield(L, -2, "HOME");
	lua_pushinteger(L, GINPUT_KEY_PAGEUP);
	lua_setfield(L, -2, "PAGE_UP");
	lua_pushinteger(L, GINPUT_KEY_PAGEDOWN);
	lua_setfield(L, -2, "PAGE_DOWN");
	lua_pushinteger(L, GINPUT_KEY_ENTER);
	lua_setfield(L, -2, "ENTER");

    lua_pushinteger(L, GINPUT_NO_BUTTON);
    lua_setfield(L, -2, "MOUSE_NONE");
    lua_pushinteger(L, GINPUT_LEFT_BUTTON);
    lua_setfield(L, -2, "MOUSE_LEFT");
    lua_pushinteger(L, GINPUT_RIGHT_BUTTON);
    lua_setfield(L, -2, "MOUSE_RIGHT");
    lua_pushinteger(L, GINPUT_MIDDLE_BUTTON);
    lua_setfield(L, -2, "MOUSE_MIDDLE");

    lua_pushinteger(L, GINPUT_NO_MODIFIER);
    lua_setfield(L, -2, "MODIFIER_NONE");
    lua_pushinteger(L, GINPUT_SHIFT_MODIFIER);
    lua_setfield(L, -2, "MODIFIER_SHIFT");
    lua_pushinteger(L, GINPUT_CTRL_MODIFIER);
    lua_setfield(L, -2, "MODIFIER_CTRL");
    lua_pushinteger(L, GINPUT_ALT_MODIFIER);
    lua_setfield(L, -2, "MODIFIER_ALT");
    lua_pushinteger(L, GINPUT_META_MODIFIER);
    lua_setfield(L, -2, "MODIFIER_META");

	lua_setglobal(L, "KeyCode");


	// correct clock function which is wrong in iphone
	lua_getglobal(L, "os");
	lua_pushcnfunction(L, os_timer, "timer");
	lua_setfield(L, -2, "timer");
	lua_pop(L, 1);

	//coroutines helpers
    luaL_newmetatable(L,"Signal");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index"); // mt.__index = mt
    lua_pushcnfunction(L, Signal_wait,"wait");
    lua_setfield(L, -2, "wait");
    lua_pushcnfunction(L, Signal_notify,"notify");
    lua_setfield(L, -2, "notify");
    lua_pop(L, 1);

	lua_getglobal(L, "Core");
    lua_pushcnfunction(L, LuaApplication::Core_asyncCall,"Core.asyncCall");
    lua_setfield(L, -2, "asyncCall");
#ifdef __EMSCRIPTEN__
    lua_pushcnfunction(L, LuaApplication::Core_asyncCall,"Core.asyncCall");
#else
    lua_pushcnfunction(L, LuaApplication::Core_asyncThread,"Core.asyncThread");
#endif
    lua_setfield(L, -2, "asyncThread");
    lua_pushcnfunction(L, LuaApplication::Core_signal,"Core.signal");
    lua_setfield(L, -2, "signal");
    lua_pushcnfunction(L, LuaApplication::Core_yield,"Core.yield");
    lua_setfield(L, -2, "yield");
    lua_pushcnfunction(L, LuaApplication::Core_yieldable,"Core.yieldable");
    lua_setfield(L, -2, "yieldable");
    lua_pushcnfunction(L, LuaApplication::Core_frameStatistics,"Core.frameStatistics");
	lua_setfield(L, -2, "frameStatistics");
	lua_pushcnfunction(L, LuaApplication::Core_profilerStart, "Core.profilerStart");
	lua_setfield(L, -2, "profilerStart");
	lua_pushcnfunction(L, LuaApplication::Core_profilerStop,"Core.profilerStop");
	lua_setfield(L, -2, "profilerStop");
	lua_pushcnfunction(L, LuaApplication::Core_profilerReset, "Core.profilerReset");
	lua_setfield(L, -2, "profilerReset");
    lua_pushcnfunction(L, LuaApplication::Core_profilerReport, "Core.profilerReport");
    lua_setfield(L, -2, "profilerReport");
    lua_pushcnfunction(L, LuaApplication::Core_enableAllocationTracking, "Core.enableAllocationTracking");
    lua_setfield(L, -2, "enableAllocationTracking");
	lua_pushcnfunction(L, LuaApplication::Core_random, "Core.random");
	lua_setfield(L, -2, "random");
	lua_pushcnfunction(L, LuaApplication::Core_randomSeed, "Core.randomSeed");
	lua_setfield(L, -2, "randomSeed");
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
        application_->mouseDown(event2->x, event2->y, event2->button, event2->modifiers, event2->mouseType);
    }
    else if (type == GINPUT_MOUSE_MOVE_EVENT)
    {
        ginput_MouseEvent *event2 = (ginput_MouseEvent*)event;
        application_->mouseMove(event2->x, event2->y, event2->button, event2->modifiers, event2->mouseType);
    }
    else if (type == GINPUT_MOUSE_HOVER_EVENT)
    {
        ginput_MouseEvent *event2 = (ginput_MouseEvent*)event;
        application_->mouseHover(event2->x, event2->y, event2->button, event2->modifiers, event2->mouseType);
    }
    else if (type == GINPUT_MOUSE_UP_EVENT)
    {
        ginput_MouseEvent *event2 = (ginput_MouseEvent*)event;
        application_->mouseUp(event2->x, event2->y, event2->button, event2->modifiers, event2->mouseType);
    }
    else if (type == GINPUT_MOUSE_WHEEL_EVENT)
    {
        ginput_MouseEvent *event2 = (ginput_MouseEvent*)event;
        application_->mouseWheel(event2->x, event2->y, event2->wheel, event2->modifiers);
    }
    else if (type == GINPUT_MOUSE_ENTER_EVENT)
    {
        ginput_MouseEvent *event2 = (ginput_MouseEvent*)event;
        application_->mouseEnter(event2->x, event2->y, event2->button, event2->modifiers);
    }
    else if (type == GINPUT_MOUSE_LEAVE_EVENT)
    {
        ginput_MouseEvent *event2 = (ginput_MouseEvent*)event;
        application_->mouseLeave(event2->x, event2->y, event2->modifiers);
    }
    else if (type == GINPUT_KEY_DOWN_EVENT)
    {
        ginput_KeyEvent *event2 = (ginput_KeyEvent*)event;
        application_->keyDown(event2->keyCode, event2->realCode,event2->modifiers);
    }
    else if (type == GINPUT_KEY_UP_EVENT)
    {
        ginput_KeyEvent *event2 = (ginput_KeyEvent*)event;
        application_->keyUp(event2->keyCode, event2->realCode,event2->modifiers);
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
        OpenUrlEvent eventg(OpenUrlEvent::OPEN_URL,event2->url);
        application_->broadcastEvent(&eventg);
    }
    else if (type == GAPPLICATION_TEXT_INPUT_EVENT)
    {
        gapplication_TextInputEvent *event2 = (gapplication_TextInputEvent*)event;
        TextInputEvent eventg(TextInputEvent::TEXT_INPUT,event2->text,event2->context,event2->selStart,event2->selEnd);
        application_->broadcastEvent(&eventg);
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

void LuaApplication::resetStyleCache()
{
    styleCache.unitS=F_NAN;
    styleCache.unitIs=F_NAN;
}

struct LuaApplication::_StyleCache LuaApplication::styleCache;
int LuaApplication::getStyleTable(lua_State *L,int sprIndex)
{
	lua_getfield(L,sprIndex,"__style");
	bool lpop=false;
	while (lua_isnil(L,-1)) {
		lua_pop(L,1);
		lua_getfield(L, sprIndex, "__parent");
		if (lua_isnil(L,-1))
			break;
		if (lpop) lua_remove(L,-2);
		lua_getfield(L,-1,"__style");
		sprIndex=-1;
		lpop=true;
	}
	if (lpop) lua_remove(L,-2);
	return 1;
}

int LuaApplication::resolveStyle(lua_State *L,const char *key,int luaIndex)
{
    //Style table is expected to be at top of stack
	lua_pushstring(L,"__Reference");
	lua_insert(L,-2);
	lua_pushstring(L,"__Parent");
	lua_insert(L,-2);
    int ret=resolveStyleInternal(L,key,luaIndex?luaIndex-2:0,-3);
	lua_remove(L,-2);
	lua_remove(L,-2);
    return ret;
}

static char emptyValue;
void LuaApplication::cacheComputedStyle(lua_State *L, const char *key, bool empty) { //Tr,val
    if (lua_rawgetfield(L,-2,"__Cache")==LUA_TTABLE) {
        if (empty)
            lua_pushlightuserdata(L,&emptyValue);
        else
            lua_pushvalue(L,-2);
        lua_setfield(L,-2,key);
    }
    lua_pop(L,1);
}

#include "fontbasebinder.h"
#include <fontbase.h>
#include <luautil.h>
int LuaApplication::resolveStyleInternal(lua_State *L,const char *key,int luaIndex,int refIndex, int limit, bool recursed)
{
    if (limit>1000) {
        lua_pushfstringL(L,"Recursion while resolving style: %s",key);
        lua_error(L);
        return LUA_TNIL;
    }

    //Expected callStack: refstring, parentstring, table
	if (luaIndex>0) luaIndex=0;
    if ((!key)&&luaIndex) key=lua_tolstring(L,luaIndex,NULL);
    if (!key) {
        lua_pushfstringL(L,"Key is nil in resolveStyle");
        lua_error(L);
    }
    if (!lua_istable(L,-1))
    {
        lua_pushfstringL(L,"Style table doesn't exist, while looking for key: %s",key);
        lua_error(L);
    }
    lua_checkstack(L,8);

    int rtype;
    if (!recursed) { //If we're not looking up a reference
        int klen=strlen(key);
        if (((*key)=='|')||((klen>3)&&((key[klen-4]=='.')&&(
                                           ((key[klen-3]=='p')&&(key[klen-2]=='n')&&(key[klen-1]=='g'))||
                                           ((key[klen-3]=='j')&&(key[klen-2]=='p')&&(key[klen-1]=='g'))
                                           ))))
        {
            //File, return as is
            lua_pop(L,1);
            if (luaIndex)
                lua_pushvalue(L,luaIndex+1);
            else
                lua_pushlstring(L,key,klen);
            return LUA_TSTRING;
        }
        const char *kk=key;
        while ((((*kk)>='0')&&((*kk)<='9'))||((*kk)=='-')||((*kk)=='.')) kk++;
        if (kk>key) {
            //Number-String: check for a unit
            if (kk[0]==0) {
                //Not suffix, just convert to number
                lua_pop(L,1);
                lua_pushnumber(L,strtod(key,NULL));
                return LUA_TNUMBER;
            }
            else if ((kk[0]=='e')&&(kk[1]=='m')&&(kk[2]==0)) {
                Binder binder(L);
                if (resolveStyleInternal(L,"font",0,refIndex,true)==LUA_TNIL)
                {
                    lua_pushfstringL(L,"Font not found for computing: %s",key);
                    lua_error(L);
                }
                FontBase *font = static_cast<FontBase*>(binder.getInstance("FontBase", -1));
                lua_Number num=font->getLineHeight();
                lua_pop(L,1);
                lua_pushnumber(L,num*strtod(key,NULL));
                return LUA_TNUMBER;
            }
            else {
                float num=F_NAN;
                if ((kk[0]=='s')&&(kk[1]==0)) // Cached-'s'
                    num=styleCache.unitS;
                else if ((kk[0]=='i')&&(kk[1]=='s')&&(kk[2]==0)) // Cached-'is'
                    num=styleCache.unitIs;
                if (isnan(num)) {
                    char unitName[32+6]="unit.";
                    strncpy(unitName+5,kk,32);
                    unitName[32+5]=0;
                    if (resolveStyleInternal(L,unitName,0,refIndex,true)==LUA_TNIL)
                    {
                        lua_pushfstringL(L,"Unit not recognized: %s",kk);
                        lua_error(L);
                    }
                    num=lua_tonumber(L,-1);
                    if ((kk[0]=='s')&&(kk[1]==0)) // Cached-'s'
                        styleCache.unitS=num;
                    else if ((kk[0]=='i')&&(kk[1]=='s')&&(kk[2]==0)) // Cached-'is'
                        styleCache.unitIs=num;
                }
                lua_pop(L,1);
                lua_pushnumber(L,num*strtod(key,NULL));
                return LUA_TNUMBER;
            }
        }
    }

    //Check cache
    bool hasCache=false;
    if (lua_rawgetfield(L,-1,"__Cache")==LUA_TTABLE) {
        hasCache=true;
        if (luaIndex) {
            lua_pushvalue(L,luaIndex-1); //Tc,C,key
            rtype=lua_gettable(L,-2); //Tc,C,Tr
        }
        else
            rtype=lua_rawgetfield(L,-1,key); //Tc,C,Tr
        if (key[0]!='f') {
            lua_pushnil(L);
            lua_pop(L,1);
        }
        if ((rtype==LUA_TLIGHTUSERDATA)&&(lua_tolightuserdata(L,-1)==&emptyValue)) {
            lua_pop(L,3);
            lua_pushnil(L);
            return LUA_TNIL;
        }
        if (rtype!=LUA_TNIL) {
            lua_remove(L,-2);
            lua_remove(L,-2);
            return rtype;
        }
        lua_pop(L,2);
    }
    else
        lua_pop(L,1);

    //Check if current table is a reference
    lua_pushvalue(L,refIndex); //Tc,Ref
    if (lua_rawget(L,-2)==LUA_TSTRING) { //Tc,Tc[Ref]
        lua_pushvalue(L,refIndex-1+1); //Tc,Key,Parent
        rtype=lua_rawget(L,-3); //Tc,Key,Tc[Parent]
        if (rtype==LUA_TTABLE)
            rtype=resolveStyleInternal(L,NULL,-2,refIndex-2,limit+1,true); //Tc,Key,Tr
        if (rtype!=LUA_TTABLE)
        {
            lua_pushfstringL(L,"Reference doesn't resolve to a table: %s",lua_tolstring(L,-2,NULL));
            lua_error(L);
        }
        lua_remove(L,-2); //Tc,Tr
        //Lookup in reference
        if (luaIndex) {
            lua_pushvalue(L,luaIndex-1); //Tc,Tr,key
            rtype=lua_gettable(L,-2); //Tc,Tr,Tr[key]
        }
        else
            rtype=lua_getfield(L,-1,key); //Tc,Tr,Tr[key]
        lua_remove(L,-2); //Tc,Tr[key]
    }
    else {
        lua_pop(L,1); //Tc
        rtype=lua_getfield(L,-1,key); //Tc,Tc[key]
    }
    //Check Parent if nil
    if (rtype==LUA_TNIL) {
        lua_pop(L,1); //Tc
        lua_pushvalue(L,refIndex+1); //Tc,Parent
        rtype=lua_rawget(L,-2); //Tc,Tc[parent]
        if (rtype==LUA_TSTRING) {
            //Parent is a string, look it up
            lua_pushvalue(L,-2); //Tc,Key,Tc
            rtype=resolveStyleInternal(L,NULL,-2,refIndex-2,limit+1,true); //Tc,Key,Tr
            lua_remove(L,-2); //Tc,Tr
        }
        if (rtype==LUA_TNIL) {
            //Nothing, return nil
            lua_remove(L,-2);
            return rtype;
        }
        if (rtype!=LUA_TTABLE)
        {
            lua_pushfstringL(L,"Parent style isn't a table while resolving: %s",key);
            lua_error(L);
        }
        //Tc,Tc[parent] or Tr
        //Recurse in parent
        rtype=resolveStyleInternal(L,key,luaIndex?luaIndex-1:0,refIndex-1,limit+1); //Tc,Tr
    }
    while ((rtype==LUA_TSTRING)&&(recursed||!limit)) {
        //Got a string, re-run full key processing
        const char *skey=lua_tolstring(L,-1,NULL); //Tc,Key
        lua_pushvalue(L,-2); //Tc,Key,Tc
        //Recurse
        rtype=resolveStyleInternal(L,skey,-2,refIndex-2,limit+1); //Tc,Key,Tr
        lua_remove(L,-2); //Tc,Tr
    }
    if (hasCache)
        cacheComputedStyle(L,key,rtype==LUA_TNIL);
    lua_remove(L,-2); //Tc[key]
    return rtype;
}

void LuaApplication::resolveColor(lua_State *L,int spriteIdx, int colIdx, float *color, std::string &cache)
{
    int idx=colIdx;
    if (colIdx&&(lua_type(L,colIdx)==LUA_TSTRING)) {
        cache=lua_tolstring(L,colIdx,NULL);
        colIdx=0;
    }
    if (!colIdx) {
    	getStyleTable(L,spriteIdx);
        resolveStyle(L,cache.c_str(),0);
        idx=-1;
    }
    switch (lua_type(L,idx)) {
    case LUA_TVECTOR:
    {
        const float *c=lua_tovector(L,idx);
        color[0]=c[0];
        color[1]=c[1];
        color[2]=c[2];
        color[3]=c[3];
        break;
    }
    case LUA_TNUMBER:
    {
        unsigned int c = lua_tounsigned(L,idx);

        color[0]= (1.0/255)*((c >> 16) & 0xff);
        color[1]= (1.0/255)*((c >> 8) & 0xff);
        color[2]= (1.0/255)*((c >> 0) & 0xff);
        color[3]=1;
        break;
    }
    case LUA_TUSERDATA:
    {
        int64_t c=luaL_checkint64(L,idx);

        color[0]= (1.0/255)*((c >> 16) & 0xff);
        color[1]= (1.0/255)*((c >> 8) & 0xff);
        color[2]= (1.0/255)*((c >> 0) & 0xff);
        color[3]= (1.0/255)*((c >> 24) & 0xff);
        break;
    }
    case LUA_TNIL:
    {
        color[0]=0;
        color[1]=0;
        color[2]=0;
        color[3]=0;
        break;
    }
    default:
        lua_pushfstringL(L,"Cannot turn %s into a color at index %d with name '%s'",lua_typename(L,lua_type(L,idx)),colIdx,cache.c_str());
        lua_error(L);
    }

    //luaL_checkvector(L,idx);
    if (!colIdx)
        lua_pop(L,1);
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
        ::free(((size_t *)ptr)-1);
}

static size_t g_getsize(void *ptr)
{
    if (memory_pool <= ptr && ptr < memory_pool_end)
        return tlsf_block_size(ptr);
    else
    	return *(((size_t *)ptr)-1);
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
        {
            size_t *ps = (size_t *)::malloc(size+sizeof(size_t));
            *ps=size;
            p=ps+1;
        }
    }
    else
    {
        if (memory_pool <= ptr && ptr < memory_pool_end)
        {
            if (size <= 256)
                p = tlsf_realloc(memory_pool, ptr, size);

            if (p == NULL)
            {
                size_t *ps = (size_t *)::malloc(size+sizeof(size_t));
                *ps=size;
                p=ps+1;
                memcpy(p, ptr, osize);
                tlsf_free(memory_pool, ptr);
            }
        }
        else
        {
            size_t *ps=(size_t *)ptr;
            ps = (size_t *)::realloc(ps-1, size+sizeof(size_t));
            *ps=size;
            p=ps+1;
        }
    }

    return p;
}

#ifndef EMSCRIPTEN0 //memalloc has issues with emscripten, disable til I know more...
class MemCacheLua : public MemCache
{
public:
	MemCacheLua();
	void *MasterAllocateMemory(size_t Size);
	void MasterFreeMemory(void *Memory);
	size_t MasterGetSize(void *Memory);
	void *MasterResizeMemory(void *Old,size_t Size);
};

MemCacheLua::MemCacheLua()
{
    if (memory_pool == NULL)
    {
        const size_t mpsize = 1024 * 1024;
        glog_v("init_memory_pool: %dKb", mpsize / 1024);
        memory_pool = tlsf_create_with_pool(malloc(mpsize), mpsize);
        memory_pool_end = (char*)memory_pool + mpsize;
    }
    Reset();
}

void *MemCacheLua::MasterAllocateMemory(size_t Size)
{
	return g_realloc(NULL, 0, Size);
}

void MemCacheLua::MasterFreeMemory(void *Memory)
{
	return g_free(Memory);
}

void *MemCacheLua::MasterResizeMemory(void *Memory,size_t Size)
{
	return g_realloc(Memory,g_getsize(Memory),Size);
}


size_t MemCacheLua::MasterGetSize(void *Memory)
{
	return g_getsize(Memory);
}

static MemCacheLua luamem;
#endif

static void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
    (void)ud;
    (void)osize;
    void *ret=NULL;
#if EMSCRIPTEN0 //TLSF has issues with emscripten, disable til I know more...
    if (nsize == 0)
    {
    	if (ptr)
            free(ptr);
    }
    else if (ptr==NULL)
        ret=malloc(nsize);
    else
        ret=realloc(ptr, nsize);
#else
    if (nsize == 0)
    {
    	if (ptr)
            luamem.FreeMemory(ptr);
    }
    else if (ptr==NULL)
        ret=luamem.AllocateMemory(nsize);
    else
        ret=luamem.ResizeMemory(ptr, nsize);
#endif
    return ret;
}

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

    lua_pushcnfunction(L, ::callFile, "callFile");
    std::string chunkname=filename;
    const char *suffix=".gideros_merged";
    int filenamesz=chunkname.length();
    int suffixlen=strlen(suffix);
    if ((filenamesz>suffixlen)&&(!strcmp(suffix,filename+filenamesz-suffixlen)))
    	chunkname.resize(filenamesz-suffixlen);

    int lres=luaL_loadfilenamed(L, filename, chunkname.c_str());
    if (lres>0)
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
#ifdef LUA_IS_LUAU
	for (int k=1;k<=-lres;k++) {
        lua_pushvalue(L,-2);
        lua_rawgeti(L,-2,k);
#endif
        if (lua_pcall_traceback(L, 1, 0, 0))
        {
            if (exceptionsEnabled_ == true)
            {
                if (status)
                    *status = GStatus(1, lua_tostring(L, -1));
            }
            lua_pop(L, 1-lres);
            application_->deleteAutounrefPool(pool);
            return;
        }
#ifdef LUA_IS_LUAU
	}
    lua_pop(L,2);
#endif
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
	Core_profilerStop(L);
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
    taskLock.lock();
    taskStopping=true;
    frameWake.notify_all();
    while (!tasks_.empty()) {
        AsyncLuaTask t=tasks_.front();
        if (t.th) {
            taskLock.unlock();
            t.th->join();
            taskLock.lock();
        }
        tasks_.pop_front();
        luaL_unref(L,LUA_REGISTRYINDEX,t.taskRef);
    }
    taskStopping=false;
    taskLock.unlock();

    //Schedule Tasks, at least one task should be run no matter if there is enough time or not
	PluginManager& pluginManager = PluginManager::instance();
	for (size_t i = 0; i < pluginManager.plugins.size(); ++i)
        pluginManager.plugins[i].main(L, 1);

	lua_close(L);
	L = NULL;
	globalLuaState=NULL;

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

static int updateStyles(lua_State *L) {
    LuaApplication *luaApplication = static_cast<LuaApplication*>(luaL_getdata(L));
    luaApplication->resetStyleCache();

    /* perform style updating */
    lua_getglobal(L,"application");
    int npop=1;
    if (!lua_isnil(L,-1))
    {
        lua_getfield(L,-1,"__styleUpdates");
        npop++;
        if (!lua_isnil(L,-1))
        {
            lua_pushnil(L);
            lua_setfield(L,-3,"__styleUpdates");
            lua_pushnil(L);
            while (lua_next(L,-2)) {
                lua_pop(L,1); //No need for sprite itself
                lua_getfield(L,-1,"updateStyle");
                lua_pushvalue(L,-2);
                lua_call(L,1,0);
            }
        }
    }
    lua_pop(L,npop);
    return 0;
}

static int updateLayout(lua_State *L) {
    LuaApplication *luaApplication = static_cast<LuaApplication*>(luaL_getdata(L));
    Application *application = luaApplication->getApplication();
    application->stage()->validateLayout();
    return 0;
}

static int updateEffects(lua_State *L) {
    LuaApplication *luaApplication = static_cast<LuaApplication*>(luaL_getdata(L));
    Application *application = luaApplication->getApplication();
    application->stage()->validateEffects();
    return 0;
}

bool LuaApplication::hasStyleUpdate=false;
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


    if (LuaApplication::hasStyleUpdate) {
        LuaApplication::hasStyleUpdate=false;
        lua_pushcnfunction(L,updateStyles,"gideros_updateStyles");
        lua_call(L,0,0);
    }

    if (application->stage()->needLayout) {
        lua_pushcnfunction(L,updateLayout,"gideros_updateLayout");
        lua_call(L,0,0);
    }

    if (application->stage()->spriteWithEffectCount) {
        lua_pushcnfunction(L,updateEffects,"gideros_updateEffects");
        lua_call(L,0,0);
    }

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
#ifdef LUA_IS_LUAU
static void yieldHook(lua_State* L, int gc) {
    if ((gc==-1)&&lua_isyieldable(L)&&(iclock() >= yieldHookLimit)) {
        L->status=LUA_YIELD;
    }
}
#else
static void yieldHook(lua_State *L,lua_Debug *ar)
{
	//glog_i("YieldHook:%f %f\n",iclock(),yieldHookLimit);
	if (ar->event == LUA_HOOKRET)
	{
		if (LuaApplication::debuggerHook&&(LuaApplication::debuggerBreak&LUA_MASKRET))
		{
            lua_getinfo(L, "lS", ar);
			LuaApplication::debuggerHook(LuaApplication::debuggerContext,L,ar);
		}
        if (iclock() >= yieldHookLimit) {
            LuaDebugging::yieldHookMask=LUA_MASKCOUNT;
			lua_sethook(L, yieldHook, LUA_MASKCOUNT | (LuaApplication::debuggerBreak&DBG_MASKLUA), 1);
        }
	}
	else if (ar->event == LUA_HOOKCOUNT)
	{
		if (iclock() >= yieldHookLimit)
		{
			if (lua_canyield(L))
				lua_yield(L, 0);
            else {
                LuaDebugging::yieldHookMask=LUA_MASKRET | LUA_MASKCOUNT;
                lua_sethook(L, yieldHook, LUA_MASKRET | LUA_MASKCOUNT | (LuaApplication::debuggerBreak&DBG_MASKLUA), 1000);
            }
		}
	}
	else if (ar->event == LUA_HOOKLINE)
	{
		lua_getinfo(L, "l", ar);
		if (LuaApplication::debuggerHook&&(LuaApplication::debuggerBreak&LUA_MASKLINE)&&
				(!(LuaApplication::debuggerBreak&DBG_MASKBREAK)||LuaApplication::breakpoints[ar->currentline]))
		{
			lua_getinfo(L, "S", ar); //Possible match, resolve source name and let debuggerHook decide
			LuaApplication::debuggerHook(LuaApplication::debuggerContext,L,ar);
		}
		else {
			if (ar->currentline!=LuaDebugging::lastLine)
				LuaDebugging::lastLine=0;
		}
	}
	else if (ar->event == LUA_HOOKCALL)
	{
		if (LuaApplication::debuggerHook&&(LuaApplication::debuggerBreak&LUA_MASKCALL))
		{
            lua_getinfo(L, "lS", ar);
			LuaApplication::debuggerHook(LuaApplication::debuggerContext,L,ar);
		}
	}
}
#endif
bool LuaApplication::onDemandDraw(bool &now) {
    bool onDemand=application_->onDemandDraw(now);
    if (onDemand) {
        meanFrameTime_=.016;
        meanFreeTime_=.015;
    }
    return onDemand;
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

    //Schedule Tasks, at least one task should be run no matter if there is enough time or not
    taskLock.lock();
    bool hasThreads=false;
    if ((meanFrameTime_ >= 0.01)&&(meanFrameTime_<=0.1)) //If frame rate is between 10Hz and 100Hz
	{
		double taskStart = iclock();
		double timeLimit = taskStart + meanFreeTime_*0.9; //Limit ourselves t 90% of free time
		yieldHookLimit = timeLimit;
        size_t loops = 0;
        size_t ntasks=tasks_.size();
		while ((!tasks_.empty())&&(!status->error()))
		{
            AsyncLuaTask t = tasks_.front();
            if (t.th) {
                //This is a full thread task, don't consider it for resume
                hasThreads=true;
                tasks_.pop_front();
                if (!t.terminated) {
                    //Not terminated, queue it again
                    tasks_.push_back(t);
                    loops++;
                    if (loops > tasks_.size())
                        break;
                    if (ntasks) ntasks--;
                    continue;
                }
                //Check termination
                taskLock.unlock();
                t.th->join();
                if (t.inError)
                {
                    if (exceptionsEnabled_ == true)
                    {
                        if (status)
                            *status = GStatus(1, lua_tostring(t.L, -1));
                    }
                }
                //Drop any return args
                lua_settop(t.L, 0);
                luaL_unref(L, LUA_REGISTRYINDEX, t.taskRef);
                taskLock.lock();
                continue;
            }
			if ((t.sleepTime > iclock()) || (t.skipFrame))
			{
                //Skip this task: push to it the back of the queue
                tasks_.pop_front();
                tasks_.push_back(t);
                loops++;
				if (loops > tasks_.size())
					break;
                if (ntasks) ntasks--;
                continue;
			}
			loops = 0;
			int res = 0;
            taskLock.unlock();
			if (t.autoYield)
			{                
#ifdef LUA_IS_LUAU
                lua_callbacks(t.L)->interrupt=yieldHook;
                res = lua_resume(t.L,L,t.nargs);
                lua_callbacks(t.L)->interrupt=NULL;
#else
                LuaDebugging::yieldHookMask=LUA_MASKRET| LUA_MASKCOUNT;
                lua_sethook(t.L, yieldHook, LUA_MASKRET | LUA_MASKCOUNT | (LuaApplication::debuggerBreak&DBG_MASKLUA), 1000);
                res = lua_resume(t.L,t.nargs);
                LuaDebugging::yieldHookMask=0;
                lua_sethook(t.L, yieldHook, (LuaApplication::debuggerBreak&DBG_MASKLUA), 1000);
#endif
			}
			else
#ifdef LUA_IS_LUAU
                res = lua_resume(t.L,NULL,t.nargs);
#else
                res = lua_resume(t.L,t.nargs);
#endif
            taskLock.lock();
            //Reload task data after task has yielded/ended
            t = tasks_.front();
            tasks_.pop_front();
			if (res == LUA_YIELD)
            { /* Yielded: push to the back of the queue */
                t.nargs=0;
                tasks_.push_back(t);
            }
			else if (res != 0)
			{
				if (exceptionsEnabled_ == true)
				{
					lua_traceback(t.L,L);
					if (status)
						*status = GStatus(1, lua_tostring(t.L, -1));
				}
                lua_pop(t.L, 1);
				luaL_unref(L, LUA_REGISTRYINDEX, t.taskRef);
				//Task had an error, return now to report the error
				break;
			}
			else
			{
				//Drop any return args
                lua_settop(t.L, 0);
				luaL_unref(L, LUA_REGISTRYINDEX, t.taskRef);
			}
			if (ntasks) ntasks--;
			if ((ntasks==0)&&(iclock() > timeLimit)) //All tasks have been executed once and time has been exceeded
				break;
		}

		for (std::deque<LuaApplication::AsyncLuaTask>::iterator it = LuaApplication::tasks_.begin(); it != LuaApplication::tasks_.end(); ++it)
			(*it).skipFrame = false;
		taskFrameTime_ = iclock() - taskStart;
	}
	else
		taskFrameTime_ = 0;
    taskLock.unlock();
    //If we have some true threads, ensure GC is performed if possible
    if (hasThreads) lua_gc(L,LUA_GCSTEP,0);
    frameWake.notify_all();
    application_->deleteAutounrefPool(pool);
}

void LuaApplication::clearBuffers()
{
	if (!frameStartTime_)
	    frameStartTime_=iclock();
	application_->clearBuffers();
}

void drawInfoResolution(int width, int height, int scale, int lWidth,
		int lHeight, bool drawRunning, float canvasColor[3],
		float infoColor[3],int ho, int ao, float fps, float cpu);

void LuaApplication::setDrawInfo(bool enable,float r,float g,float b,float a)
{
	drawInfo_=enable;
	infoColor_[0]=r;
	infoColor_[1]=g;
	infoColor_[2]=b;
	infoColor_[3]=a;
}

void LuaApplication::renderScene(int deltaFrameCount,float *vmat,float *pmat,const std::function<void(ShaderEngine *,Matrix4 &)> &preStage)
{
	application_->renderScene(-1,vmat,pmat,preStage);

	//Compute frame timings
    double frmEnd=iclock();
    double frmLasted=frmEnd-lastFrameTime_;
    if ((frmLasted>=0.01)&&(frmLasted<0.1)) //If frame rate is between 10Hz and 100Hz
    	meanFrameTime_=meanFrameTime_*0.95+frmLasted*0.05; //Average on 20 frames
    lastFrameTime_=frmEnd;

    double freeTime=meanFrameTime_-(frmEnd-frameStartTime_-taskFrameTime_);
    if (freeTime>=0)
    	meanFreeTime_=meanFreeTime_*0.95+freeTime*0.05; //Average on 20 frames
	//glog_i("FrameTimes:last:%f mean:%f task:%f free:%f\n",frmLasted,meanFrameTime_,taskFrameTime_,meanFreeTime_);

	frameStartTime_=0;

	float canvasColor[3]={1,1,1}; //Dummy, not used anyway
	if (drawInfo_)
		drawInfoResolution(getHardwareWidth(), getHardwareHeight(), 100, getLogicalWidth(), getLogicalHeight(),
				true, canvasColor, infoColor_, (int) hardwareOrientation(), (int) orientation(),
				1.0/meanFrameTime_,1-(meanFreeTime_/meanFrameTime_));
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


void LuaApplication::setResolution(int width, int height,bool keepBuffers)
{
	width_ = width;
	height_ = height;

	application_->setResolution(width_, height_, keepBuffers);
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

	lua_pushcnfunction(L, ::broadcastEvent, "broadcastEvent");
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
	drawInfo_=false;

#if defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define ARCH_X64 1
#else
#define ARCH_X64 0
#endif

#ifdef LUA_IS_LUAU
    L = lua_newstate(l_alloc, NULL);
    //L = luaL_newstate();
#else
    if (ARCH_X64 && lua_isjit())
        L = luaL_newstate();
    else
        L = lua_newstate(l_alloc, NULL);
#endif

    lua_pushlightuserdata(L, &key_tickFunction);
    lua_pushcnfunction(L, ::tick, "gideros_tick");
    lua_rawset(L, LUA_REGISTRYINDEX);

    lua_pushlightuserdata(L, &key_enterFrameFunction);
	lua_pushcnfunction(L, ::enterFrame, "gideros_enterFrame");
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

    resetStyleCache();

    lua_setprintfunc(L, printFunc_, printData_);
    luaL_setdata(L, this);

	luaL_openlibs(L);

	//	lua_sethook(L, testHook, LUA_MASKLINE, 0);
	lua_pushcnfunction(L, bindAll, "gideros_bindAll");
	lua_pushlightuserdata(L, application_);
	lua_call(L, 1, 0);

	Rnd::Initialize(iclock()*0xFFFF);
    LuaDebugging::L=L;
    LuaDebugging::yieldHookMask=0;
#ifndef LUA_IS_LUAU
    LuaDebugging::hook=yieldHook;
    lua_sethook(L, yieldHook, (LuaApplication::debuggerBreak&DBG_MASKLUA), 1);
#endif
    globalLuaState=L;

    Core_profilerReset(L);
    if (LuaDebugging::profiling)
    	Core_profilerStart(L);
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

//PROFILER
struct ProfileInfo {
	std::string fid;
	std::string name;
	double time;
	int count;
	double entered;
	int enterCount;
    std::string callret;
	std::map<std::string,double> cTime;
	std::map<std::string,int> cCount;
};
static std::map<std::string,ProfileInfo*> proFuncs;
static std::map<Closure*,ProfileInfo*> proLookup;
static ProfileInfo *profilerGetInfo(Closure *cl)
{
	ProfileInfo*p=proLookup[cl];
	if (!p)
	{
		std::string fid;
		char fmt[255];
#ifdef LUA_IS_LUAU
        if (cl->isC)
#else
        if (cl->c.isC)
#endif
		{
#ifdef LUA_IS_LUAU
            if (cl->c.debugname)
                sprintf(fmt,"=[C] %p(%s)",cl->c.f,cl->c.debugname);
#else
            if (cl->c.name)
				sprintf(fmt,"=[C] %p(%s)",cl->c.f,cl->c.name);
#endif
            else
				sprintf(fmt,"=[C] %p",cl->c.f);
		}
		else
#ifdef LUA_IS_LUAU
            sprintf(fmt,"%s:%d:%p",getstr(cl->l.p->source),cl->l.p->abslineinfo?cl->l.p->abslineinfo[0]:0,cl->l.p);
#else
            sprintf(fmt,"%s:%d:%p",getstr(cl->l.p->source),cl->l.p->linedefined,cl->l.p);
#endif
		fid=fmt;
		p=proFuncs[fid];
		if (!p) {
			p=(ProfileInfo *)new ProfileInfo;
			proFuncs[fid]=p;
			proLookup[cl]=p;
			p->fid=fid;
			p->time=0;
			p->count=0;
			p->enterCount=0;
		}
	}
	return p;
}

static void profilerHook(lua_State *L,int enter)
{
	double time=iclock();
	Closure *cl=curr_func(L);
	ProfileInfo *p=profilerGetInfo(cl);
	if (enter)
	{
		if (!(p->enterCount++))
		{
			p->entered=time;
			if (p->name.empty())
			{
				lua_Debug ar;                
				ar.name=NULL;
#ifdef LUA_IS_LUAU
                lua_getinfo(L,0,"n",&ar); //Check the '1' value
                if ((cl->isC)&&(cl->c.debugname))
                    p->name=cl->c.debugname;
                else {
                    if (ar.name)
                        p->name=ar.name;
                    else
                        p->name="Unknown";
                }
#else
				ar.i_ci = cast_int(L->ci - L->base_ci);
				lua_getinfo(L,"n",&ar);
				if ((cl->c.isC)&&(cl->c.name))
					p->name=cl->c.name;
				else {
					if (ar.name)
						p->name=ar.name;
					else
						p->name="Unknown";
                }
#endif
			}
			if (L->ci>L->base_ci)
			{
				CallInfo *ci=L->ci;
				ci--;
			    if(ttisfunction(ci->func))
			    {
			    	Closure *ccl=ci_func(ci);
			    	ProfileInfo *cp=profilerGetInfo(ccl);
                    p->callret=cp->fid;
			    }
			}
		}
	}
	else
	{
		int rcalls=1;
#ifndef LUA_IS_LUAU
		if (f_isLua(L->ci)) {  /* Lua function? */
		    rcalls+=L->ci->tailcalls;
		}
#endif
		while (p&&(rcalls--)) {
			if (p->enterCount)
			{
				double ctime=0;
                if (!(--p->enterCount))
					ctime=time-p->entered;
				p->time+=ctime;
				p->count++;
                ProfileInfo *np=NULL;
				if (!(p->callret.empty()))
                {
                    p->cCount[p->callret]=p->cCount[p->callret]+1;
                    p->cTime[p->callret]=p->cTime[p->callret]+ctime;
                    np=proFuncs[p->callret];
                    if (!(p->enterCount))
                        p->callret.clear();
                }
				p=np;
			}
		}
	}
}

int LuaApplication::Core_profilerStart(lua_State *L)
{
	if (!L->profilerHook) //Don't set if already set, since it could be used by the debugger
		L->profilerHook=profilerHook;
	return 0;
}

int LuaApplication::Core_profilerStop(lua_State *L)
{
    L->profilerHook=NULL;
	return 0;
}

int LuaApplication::Core_profilerReport(lua_State *L)
{
	lua_newtable(L);
	for (std::map<std::string,ProfileInfo*>::iterator it=proFuncs.begin();it!=proFuncs.end();it++)
	{
		ProfileInfo *p=it->second;
		lua_newtable(L);
		lua_pushinteger(L,p->count);
		lua_setfield(L,-2,"count");
		lua_pushnumber(L,p->time);
		lua_setfield(L,-2,"time");
		lua_pushstring(L,p->name.c_str());
		lua_setfield(L,-2,"name");

		lua_newtable(L);
		for (std::map<std::string,int>::iterator it2=p->cCount.begin();it2!=p->cCount.end();it2++)
		{
			lua_newtable(L);
			lua_pushinteger(L,it2->second);
			lua_setfield(L,-2,"count");
			lua_pushnumber(L,p->cTime[it2->first]);
			lua_setfield(L,-2,"time");
			lua_setfield(L,-2,it2->first.c_str());
		}
		lua_setfield(L,-2,"callers");

		lua_setfield(L,-2,it->first.c_str());
	}
	return 1;
}

int LuaApplication::Core_profilerReset(lua_State *L)
{
    G_UNUSED(L);
	proLookup.clear();
	for (std::map<std::string,ProfileInfo*>::iterator it=proFuncs.begin();it!=proFuncs.end();it++)
		delete it->second;
	proFuncs.clear();
	return 0;
}

int LuaApplication::Core_enableAllocationTracking(lua_State *L)
{
    lua_gc(L, LUA_GCCOLLECT, 0);
    lua_gc(L, LUA_GCCOLLECT, 0);
    bool en=lua_toboolean(L,1);
    L->profileTableAllocs=en;
    lua_getglobal(L,"__tableAllocationProfiler__");
    if (!en) {
        lua_pushnil(L);
        lua_setglobal(L,"__tableAllocationProfiler__");
    }
    return 1;
}

int LuaApplication::Core_random(lua_State *L)
{
	int gen=luaL_optnumber(L,1,0);
	switch (gen)
	{
	default:
		if (lua_isnoneornil(L,2))
		{
			double val=Rnd::MT19937::ExtractDouble();
			lua_pushnumber(L,val);
			return 1;
		}
		uint64_t uival=Rnd::MT19937::ExtractU32();
		int a=luaL_checkinteger(L,2);
		if (lua_isnoneornil(L,3))
		{
			lua_pushinteger(L,1+(((uival*a)>>32)&0xFFFFFFFF));
		}
		else
		{
			int b=luaL_checkinteger(L,3);
			lua_pushinteger(L,a+(((uival*(b+1-a))>>32)&0xFFFFFFFF));
		}
		return 1;
	break;
	}
	return 1;
}

int LuaApplication::Core_randomSeed(lua_State *L)
{
	int gen=luaL_optnumber(L,1,0);
	int seed=luaL_optinteger(L,2,iclock()*0xFFFF);
	switch (gen)
	{
	default: lua_pushinteger(L,Rnd::MT19937::GetSeed()); Rnd::MT19937::Initialize(seed); break;
	}
	return 1;
}



//
