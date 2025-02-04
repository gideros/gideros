#ifndef LUAAPPLICATION_H
#define LUAAPPLICATION_H

//extern "C"
//{
//#include "lua.h"
//#include "lauxlib.h"
//#include "lualib.h"
//}
#include "lua.hpp"

class Application;
class Ticker;

#include <exception>
#include <string>
#include <vector>
#include <set>
#include <gideros_p.h>
#include <deque>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <gglobal.h>

struct Touch;

#include "orientation.h"
class Event;

#include <gstatus.h>
#include <functional>
#include "Shaders.h"
#include "Matrices.h"

/*
class LuaException : public std::exception
{
public:
	enum Type
	{
		eLoadError,
		eRuntimeErrorAfterLoad,
		eRuntimeError
	};
	
	LuaException(Type type, const char* message) : type_(type), message_(message) {}
	~LuaException() throw() {}

	virtual const char* what() const throw()
	{
		return message_.c_str();
	}

	Type type() const
	{
		return type_;
	}

private:
	Type type_;
	std::string message_;
};
*/

typedef int (*gapplication_LuaArgPusher)(struct lua_State *L,void *returnData);
void gapplication_luaCallback(int luaFuncRef,void *data,gapplication_LuaArgPusher pusher);

struct ProjectProperties {
	ProjectProperties() {
		scaleMode = 0;
		logicalWidth = 320;
		logicalHeight = 480;
		orientation = 0;
		fps = 60;
		retinaDisplay = 0;
		autorotation = 0;
		mouseToTouch = 1;
		touchToMouse = 1;
		mouseTouchOrder = 0;
		windowWidth=0;
		windowHeight=0;
	}
	void load(const std::vector<char> &data, bool skipFirst);

	int scaleMode;
	int logicalWidth;
	int logicalHeight;
	std::vector<std::pair<std::string, float> > imageScales;
	int orientation;
	int fps;
	int retinaDisplay;
	int autorotation;
	int mouseToTouch;
	int touchToMouse;
	int mouseTouchOrder;
	int windowWidth;
	int windowHeight;
	std::string app_name;
    int version_code;
    std::string version;
    int build_number;
    unsigned int build_timestamp;
    static ProjectProperties current;
};

class LuaApplication : public LuaApplicationBase
{
public:
    static bool hasStyleUpdate;
    LuaApplication(void);
	~LuaApplication(void);

	void enableExceptions();
	void disableExceptions();
	
	lua_PrintFunc getPrintFunc(void);
    void setPrintFunc(lua_PrintFunc printFunc, void *data = NULL);

	virtual void initialize();

    void loadFile(const char* filename, GStatus *status);

    void tick(GStatus *status);
    void enterFrame(GStatus *status);
	void clearBuffers(int deltaFrameCount=-1);
	void renderScene(int deltaFrameCount = -1,float *vmat=NULL, float *pmat=NULL,const std::function<void(ShaderEngine *,Matrix4 &)> &preStage=nullptr);
    bool onDemandDraw(bool &now);

    virtual void deinitialize();

	bool isInitialized() const;

    void setPlayerMode(bool isPlayer);
    bool isPlayerMode();
    bool isPlayer_;

	void setHardwareOrientation(Orientation orientation);
	void setResolution(int width, int height,bool keepBuffers=false);
	void setDrawInfo(bool enable,float r,float g,float b,float a);

//	void broadcastApplicationDidFinishLaunching();
//	void broadcastApplicationWillTerminate();
//  void broadcastMemoryWarning();

    void broadcastEvent(Event* event, GStatus *status);

	lua_State* luaState() const
	{
		return L;
	}

//	void orientationChange(Orientation orientation);

	Application* getApplication() const
	{
		return application_;
	}

	float getPhysicsScale() const
	{
		return physicsScale_;
	}

	void setPhysicsScale(float physicsScale)
	{
		physicsScale_ = physicsScale;
	}

    void setScale(float scale);

	void setLogicalDimensions(int width, int height);
	void setLogicalScaleMode(LogicalScaleMode mode);

	int getLogicalWidth() const;
	int getLogicalHeight() const;
	int getHardwareWidth() const;
	int getHardwareHeight() const;

	void setImageScales(const std::vector<std::pair<std::string, float> >& imageScales);
	const std::vector<std::pair<std::string, float> >& getImageScales() const;

	void setOrientation(Orientation orientation);
	Orientation orientation() const;
	Orientation hardwareOrientation() const
	{
	     return orientation_;
	}

	virtual void addTicker(Ticker* ticker); //Virtual calls are accessible to .DLL plugins
	virtual void removeTicker(Ticker* ticker);

	float getLogicalTranslateX() const;
	float getLogicalTranslateY() const;
	float getLogicalScaleX() const;
	float getLogicalScaleY() const;

    virtual void setError(const char* error);
    virtual bool isErrorSet() const;
    virtual const char* getError() const;
    virtual void clearError();

    lua_State *getLuaState() const;

    struct AsyncLuaTask {
    	lua_State *L;
    	int taskRef;
    	double sleepTime;
    	bool skipFrame;
        bool autoYield;
        bool inError;
        bool terminated;
    	int nargs;
        std::thread *th;
        bool profilerYielded;
        double profilerYieldStart;
    };
    static std::deque<AsyncLuaTask> tasks_;
    static std::mutex taskLock;
    static std::condition_variable frameWake;
    static bool taskStopping;
    static double meanFrameTime_; //Average frame duration
    static double meanFreeTime_; //Average time available for async tasks
    static unsigned long frameCounter_; //Global frame counter
    static void runThread(lua_State *L);
    static int Core_asyncCall(lua_State *L);
    static int Core_asyncThread(lua_State *L);
    static int Core_setAutoYield(lua_State *L);
    static int Core_yield(lua_State *L);
    static int Core_yieldable(lua_State *L);
    static int Core_stopping(lua_State *L);
    static int Core_signal(lua_State *L);
    static int Core_getScriptPath(lua_State *L);
    static int Core_frameStatistics(lua_State *L);
    static int Core_enableAllocationTracking(lua_State *L);
    static int Core_profilerStart(lua_State *L);
    static int Core_profilerStop(lua_State *L);
    static int Core_profilerReset(lua_State *L);
    static int Core_profilerReport(lua_State *L);
    static int Core_random(lua_State *L);
    static int Core_randomSeed(lua_State *L);
    static int getStyleTable(lua_State *L,int sprIndex);
    static int resolveStyle(lua_State *L,const char *key,int luaIndex);
    static int resolveStyleInternal(lua_State *L,const char *key,int luaIndex, int limit=0, bool recursed=false, bool allowPrimary=false);
    static void cacheComputedStyle(lua_State *L, const char *key, bool empty);
    static void resolveColor(lua_State *L,int spriteIdx, int colIdx, float *color, std::string &cache);
    void resetStyleCache();
    void applyStyles();
    //Debugger support
#define DBG_MASKBREAK	(1<<7) //Check breakpoints: Not a lua debug flag
#define DBG_MASKSUB		(1<<6) //Ignore subcalls: Not a lua debug flag
#define DBG_MASKLUA		0x1F
    static int debuggerBreak;
    static std::map<int,bool> breakpoints;
    static void (*debuggerHook)(void *context,lua_State *L,lua_Debug *ar);
    static void *debuggerContext;
    static size_t token__parent,token__style,token__Reference,token__Parent,token__Cache;
    static size_t token_application,token__styleUpdates,token_updateStyle;
private:
    static struct _StyleCache {
        float unitS;
        float unitIs;
    } styleCache;
	float physicsScale_;
	bool drawInfo_;
	float infoColor_[4];

	static const char* fileNameFunc_s(const char* filename, void* data);
	const char* fileNameFunc(const char* filename);

	lua_State* L;
	Application* application_;
	lua_PrintFunc printFunc_;
    void *printData_;
	bool exceptionsEnabled_;
	Orientation orientation_;
	int width_, height_;

    float scale_;

    std::string error_;

    static void callback_s(int type, void *event, void *udata);
    void callback(int type, void *event);

    double frameStartTime_; //Time at which that frame processing started
    double lastFrameTime_; //Total duration of last frame
    double taskFrameTime_; //Total time consumed by async tasks
};


#endif
