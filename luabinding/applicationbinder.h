#ifndef APPLICATIONBINDER_H
#define APPLICATIONBINDER_H

#include "binder.h"

class ApplicationBinder
{
public:
	ApplicationBinder(lua_State* L);

private:
	static int openUrl(lua_State* L);
	static int canOpenUrl(lua_State* L);
	static int getLogicalWidth(lua_State* L);
	static int getLogicalHeight(lua_State* L);
	static int getDeviceWidth(lua_State* L);
	static int getDeviceHeight(lua_State* L);
	static int vibrate(lua_State* L);
	static int getLocale(lua_State* L);
	static int getLanguage(lua_State* L);
	static int setKeepAwake(lua_State* L);
	static int setKeyboardVisibility(lua_State* L);
	static int getLogicalTranslateX(lua_State* L);
	static int getLogicalTranslateY(lua_State* L);
	static int getLogicalScaleX(lua_State* L);
	static int getLogicalScaleY(lua_State* L);
	static int getDeviceInfo(lua_State* L);
	static int getContentWidth(lua_State* L);
	static int getContentHeight(lua_State* L);
	static int setBackgroundColor(lua_State* L);
	static int getBackgroundColor(lua_State* L);
	static int setOrientation(lua_State* L);
	static int getOrientation(lua_State* L);
	static int setScaleMode(lua_State* L);
    static int getScaleMode(lua_State* L);
    static int setLogicalDimensions(lua_State* L);
    static int getFps(lua_State* L);
    static int setFps(lua_State* L);
    static int exit(lua_State* L);
    static int getApiVersion(lua_State* L);
    static int getTextureMemoryUsage(lua_State* L);
    static int getScreenDensity(lua_State* L);
    static int getDeviceOrientation(lua_State *L);
    static int configureFrustum(lua_State* L);
    static int isPlayerMode(lua_State *L);
    static int setWindowSize(lua_State *L);
    static int setFullScreen(lua_State *L);
    static int getDeviceName(lua_State *L);
    static int set(lua_State *L);
    static int get(lua_State *L);
};

#endif
