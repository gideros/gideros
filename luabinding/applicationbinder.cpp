#include "applicationbinder.h"
#include <platform.h>
#include "stackchecker.h"
#include "luaapplication.h"
#include "application.h"
#include "gstatus.h"
#include <ginput.h>
#include <gtexture.h>
#include <gapplication.h>
#include <luautil.h>
#include <gideros.h>
#include <algorithm>
#include <sstream>

#define PORTRAIT "portrait"
#define PORTRAIT_UPSIDE_DOWN "portraitUpsideDown"
#define LANDSCAPE_LEFT "landscapeLeft"
#define LANDSCAPE_RIGHT "landscapeRight"

#define NO_SCALE "noScale"
#define CENTER "center"
#define PIXEL_PERFECT "pixelPerfect"
#define LETTERBOX "letterbox"
#define CROP "crop"
#define STRETCH "stretch"
#define FIT_WIDTH "fitWidth"
#define FIT_HEIGHT "fitHeight"

#ifdef WINSTORE
#undef max
#undef min
#endif


ApplicationBinder::ApplicationBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
		{"openUrl", ApplicationBinder::openUrl},
		{"canOpenUrl", ApplicationBinder::canOpenUrl},
		{"getLogicalWidth", ApplicationBinder::getLogicalWidth},
		{"getLogicalHeight", ApplicationBinder::getLogicalHeight},
		{"getDeviceWidth", ApplicationBinder::getDeviceWidth},
		{"getDeviceHeight", ApplicationBinder::getDeviceHeight},
		{"vibrate", ApplicationBinder::vibrate},
		{"getLocale", ApplicationBinder::getLocale},
		{"getLanguage", ApplicationBinder::getLanguage},
		{"setKeepAwake", ApplicationBinder::setKeepAwake},
		{"setKeyboardVisibility", ApplicationBinder::setKeyboardVisibility},
		{"getLogicalTranslateX", ApplicationBinder::getLogicalTranslateX},
		{"getLogicalTranslateY", ApplicationBinder::getLogicalTranslateY},
		{"getLogicalScaleX", ApplicationBinder::getLogicalScaleX},
		{"getLogicalScaleY", ApplicationBinder::getLogicalScaleY},
		{"getDeviceInfo", ApplicationBinder::getDeviceInfo},
		{"getContentWidth", ApplicationBinder::getContentWidth},
		{"getContentHeight", ApplicationBinder::getContentHeight},
		{"setBackgroundColor", ApplicationBinder::setBackgroundColor},
		{"getBackgroundColor", ApplicationBinder::getBackgroundColor},
		{"setOrientation", ApplicationBinder::setOrientation},
		{"getOrientation", ApplicationBinder::getOrientation},
        {"setScaleMode", ApplicationBinder::setScaleMode},
        {"getScaleMode", ApplicationBinder::getScaleMode},
        {"setLogicalDimensions", ApplicationBinder::setLogicalDimensions},
        {"setFps", ApplicationBinder::setFps},
        {"getFps", ApplicationBinder::getFps},
        {"exit", ApplicationBinder::exit},
        {"isPlayerMode", ApplicationBinder::isPlayerMode},
        {"getApiVersion", ApplicationBinder::getApiVersion},
        {"getTextureMemoryUsage", ApplicationBinder::getTextureMemoryUsage},
        {"getScreenDensity", ApplicationBinder::getScreenDensity},
        {"getDeviceOrientation", ApplicationBinder::getDeviceOrientation},
        {"configureFrustum", ApplicationBinder::configureFrustum},
        {"setWindowSize", ApplicationBinder::setWindowSize},
        {"setFullScreen", ApplicationBinder::setFullScreen},
        {"getDeviceName", ApplicationBinder::getDeviceName},
        {"set", ApplicationBinder::set},
        {"get", ApplicationBinder::get},
        {NULL, NULL},
	};

	binder.createClass("Application", NULL, 0, 0, functionList);

	lua_getglobal(L, "Application");

	lua_pushstring(L, PORTRAIT);
	lua_setfield(L, -2, "PORTRAIT");

	lua_pushstring(L, PORTRAIT_UPSIDE_DOWN);
	lua_setfield(L, -2, "PORTRAIT_UPSIDE_DOWN");

	lua_pushstring(L, LANDSCAPE_LEFT);
	lua_setfield(L, -2, "LANDSCAPE_LEFT");

	lua_pushstring(L, LANDSCAPE_RIGHT);
	lua_setfield(L, -2, "LANDSCAPE_RIGHT");

	lua_pop(L, 1);

	binder.pushInstance("Application", NULL);
	lua_setglobal(L, "application");
}

int ApplicationBinder::openUrl(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	const char* url = luaL_checkstring(L, 2);
	::openUrl(url);
	return 0;
}

int ApplicationBinder::canOpenUrl(lua_State* L)
{
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    const char* url = luaL_checkstring(L, 2);
    lua_pushboolean(L, ::canOpenUrl(url));
    return 1;
}

int ApplicationBinder::isPlayerMode(lua_State* L)
{
    Binder binder(L);
    (void)binder.getInstance("Application", 1);
    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
    lua_pushboolean(L, application->isPlayerMode());
    return 1;
}


int ApplicationBinder::getLogicalWidth(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	lua_pushnumber(L, application->getLogicalWidth());

	return 1;
}

int ApplicationBinder::getLogicalHeight(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	lua_pushnumber(L, application->getLogicalHeight());

	return 1;
}

int ApplicationBinder::getDeviceWidth(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	Orientation orientation = application->orientation();

	if ((orientation == eLandscapeLeft || orientation == eLandscapeRight) &&
	    (application->hardwareOrientation()==eFixed))
		lua_pushnumber(L, application->getHardwareHeight());
	else
		lua_pushnumber(L, application->getHardwareWidth());

	return 1;
}

int ApplicationBinder::getDeviceHeight(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	Orientation orientation = application->orientation();

	if ((orientation == eLandscapeLeft || orientation == eLandscapeRight) &&
	    (application->hardwareOrientation()==eFixed))
		lua_pushnumber(L, application->getHardwareWidth());
	else
		lua_pushnumber(L, application->getHardwareHeight());

	return 1;
}

int ApplicationBinder::vibrate(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

    int ms = 100;
    if(!lua_isnoneornil(L,2))
        ms = lua_tonumber(L, 2);

    ::vibrate(ms);

	return 0;
}


int ApplicationBinder::getLocale(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	lua_pushstring(L, ::getLocale().c_str());

	return 1;
}

int ApplicationBinder::getLanguage(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	lua_pushstring(L, ::getLanguage().c_str());

	return 1;
}

int ApplicationBinder::setKeepAwake(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	::setKeepAwake(lua_toboolean(L, 2) != 0);

	return 0;
}

int ApplicationBinder::setKeyboardVisibility(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	lua_pushboolean(L,::setKeyboardVisibility(lua_toboolean(L, 2) != 0));

	return 1;
}

int ApplicationBinder::getLogicalTranslateX(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	lua_pushnumber(L, application->getLogicalTranslateX());

	return 1;
}

int ApplicationBinder::getLogicalTranslateY(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	lua_pushnumber(L, application->getLogicalTranslateY());

	return 1;
}

int ApplicationBinder::getLogicalScaleX(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	lua_pushnumber(L, application->getLogicalScaleX());

	return 1;
}

int ApplicationBinder::getLogicalScaleY(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	lua_pushnumber(L, application->getLogicalScaleY());

	return 1;
}

int ApplicationBinder::getDeviceInfo(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	std::vector<std::string> deviceInfo = ::getDeviceInfo();

	for (size_t i = 0; i < deviceInfo.size(); ++i)
		lua_pushstring(L, deviceInfo[i].c_str());

	return deviceInfo.size();
}

int ApplicationBinder::getContentWidth(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	Orientation orientation = application->orientation();

	if (orientation == eLandscapeLeft || orientation == eLandscapeRight)
		lua_pushnumber(L, application->getLogicalHeight());
	else
		lua_pushnumber(L, application->getLogicalWidth());

	return 1;
}

int ApplicationBinder::getContentHeight(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	Orientation orientation = application->orientation();

	if (orientation == eLandscapeLeft || orientation == eLandscapeRight)
		lua_pushnumber(L, application->getLogicalWidth());
	else
		lua_pushnumber(L, application->getLogicalHeight());

	return 1;
}

int ApplicationBinder::setBackgroundColor(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	unsigned int color = luaL_checkinteger(L, 2);

	int r = (color >> 16) & 0xff;
	int g = (color >> 8) & 0xff;
	int b = color& 0xff;

	application->getApplication()->setBackgroundColor(r/255.f, g/255.f, b/255.f);

	return 0;
}

int ApplicationBinder::getBackgroundColor(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	float r, g, b;
	application->getApplication()->getBackgroundColor(&r, &g, &b);

	int ir = std::min((int)(r * 256), 255);
	int ig = std::min((int)(g * 256), 255);
	int ib = std::min((int)(b * 256), 255);

	lua_pushinteger(L, (ir << 16) | (ig << 8) | ib);

	return 1;
}

int ApplicationBinder::setOrientation(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	const char* orientation = luaL_checkstring(L, 2);

	if (strcmp(orientation, PORTRAIT) == 0)
	{
		application->getApplication()->setOrientation(ePortrait);
	}
	else if (strcmp(orientation, PORTRAIT_UPSIDE_DOWN) == 0)
	{
		application->getApplication()->setOrientation(ePortraitUpsideDown);
	}
	else if (strcmp(orientation, LANDSCAPE_LEFT) == 0)
	{
		application->getApplication()->setOrientation(eLandscapeLeft);
	}
	else if (strcmp(orientation, LANDSCAPE_RIGHT) == 0)
	{
		application->getApplication()->setOrientation(eLandscapeRight);
	}
	else
	{
		GStatus status(2008, "orientation");	// Parameter %s must be one of the accepted values.
		return luaL_error(L, status.errorString());
	}

	return 0;
}

int ApplicationBinder::getOrientation(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	switch (application->getApplication()->orientation())
	{
	case ePortrait:
		lua_pushstring(L, PORTRAIT);
		break;
	case ePortraitUpsideDown:
		lua_pushstring(L, PORTRAIT_UPSIDE_DOWN);
		break;
	case eLandscapeLeft:
		lua_pushstring(L, LANDSCAPE_LEFT);
		break;
	case eLandscapeRight:
		lua_pushstring(L, LANDSCAPE_RIGHT);
		break;
	}

	return 1;
}

int ApplicationBinder::setScaleMode(lua_State *L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    const char* scaleMode = luaL_checkstring(L, 2);

    if (strcmp(scaleMode, NO_SCALE) == 0)
	{
        application->getApplication()->setLogicalScaleMode(eNoScale);
    }
    else if (strcmp(scaleMode, CENTER) == 0)
	{
        application->getApplication()->setLogicalScaleMode(eCenter);
    }
    else if (strcmp(scaleMode, PIXEL_PERFECT) == 0)
	{
        application->getApplication()->setLogicalScaleMode(ePixelPerfect);
    }
    else if (strcmp(scaleMode, LETTERBOX) == 0)
	{
        application->getApplication()->setLogicalScaleMode(eLetterBox);
    }
    else if (strcmp(scaleMode, CROP) == 0)
	{
        application->getApplication()->setLogicalScaleMode(eCrop);
    }
    else if (strcmp(scaleMode, STRETCH) == 0)
	{
        application->getApplication()->setLogicalScaleMode(eStretch);
    }
    else if (strcmp(scaleMode, FIT_WIDTH) == 0)
	{
        application->getApplication()->setLogicalScaleMode(eFitWidth);
    }
    else if (strcmp(scaleMode, FIT_HEIGHT) == 0)
	{
        application->getApplication()->setLogicalScaleMode(eFitHeight);
    }
	else
	{
        GStatus status(2008, "scaleMode");	// Parameter %s must be one of the accepted values.
        return luaL_error(L, status.errorString());
	}

	return 0;
}

int ApplicationBinder::getScaleMode(lua_State* L)
{
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    switch (application->getApplication()->getLogicalScaleMode())
    {
    case eNoScale:
        lua_pushstring(L, NO_SCALE);
        break;
    case eCenter:
        lua_pushstring(L, CENTER);
        break;
    case ePixelPerfect:
        lua_pushstring(L, PIXEL_PERFECT);
        break;
    case eLetterBox:
        lua_pushstring(L, LETTERBOX);
        break;
    case eCrop:
        lua_pushstring(L, CROP);
        break;
    case eStretch:
        lua_pushstring(L, STRETCH);
        break;
    case eFitWidth:
        lua_pushstring(L, FIT_WIDTH);
        break;
    case eFitHeight:
        lua_pushstring(L, FIT_HEIGHT);
        break;
    }

    return 1;
}

int ApplicationBinder::setLogicalDimensions(lua_State* L)
{
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    int width = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);

    application->getApplication()->setLogicalDimensions(width, height);

    return 0;
}

extern "C" {
int g_getFps();
void g_setFps(int fps);
}

int ApplicationBinder::getFps(lua_State* L)
{
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    lua_pushinteger(L, g_getFps());

    return 1;
}

int ApplicationBinder::setFps(lua_State* L)
{
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    int fps = luaL_checkinteger(L, 2);

    if (fps != 30 && fps != 60 && fps != -30 && fps != -60)
    {
        GStatus status(2008, "fps");	// Parameter %s must be one of the accepted values.
        return luaL_error(L, status.errorString());
    }

    g_setFps(fps);

    return 0;
}

int ApplicationBinder::exit(lua_State *L)
{
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    extern void g_exit();
    g_exit();

    return 0;
}

int ApplicationBinder::getApiVersion(lua_State* L)
{
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    lua_pushliteral(L, GIDEROS_VERSION);

    return 1;
}

int ApplicationBinder::getTextureMemoryUsage(lua_State* L)
{
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    lua_pushnumber(L, gtexture_getMemoryUsage() / 1024.0);

    return 1;
}

int ApplicationBinder::getScreenDensity(lua_State* L)
{
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    int dpi = gapplication_getScreenDensity();

    if (dpi == -1)
        lua_pushnil(L);
    else
        lua_pushinteger(L, dpi);

    return 1;
}

int ApplicationBinder::getDeviceOrientation(lua_State *L)
{
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    LuaApplication *application = static_cast<LuaApplication*>(luaL_getdata(L));

    switch (application->getApplication()->getDeviceOrientation())
    {
    case ePortrait:
        lua_pushstring(L, PORTRAIT);
        break;
    case ePortraitUpsideDown:
        lua_pushstring(L, PORTRAIT_UPSIDE_DOWN);
        break;
    case eLandscapeLeft:
        lua_pushstring(L, LANDSCAPE_LEFT);
        break;
    case eLandscapeRight:
        lua_pushstring(L, LANDSCAPE_RIGHT);
        break;
    }

    return 1;
}

int ApplicationBinder::configureFrustum(lua_State* L)
{
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    lua_Number fov = luaL_checknumber(L, 2);
    if (fov<0) fov=0;
    if (fov>180) fov=180;

    lua_Number farplane=0;
    if (!lua_isnoneornil(L, 3))
    	farplane = luaL_checknumber(L, 3);
    lua_Number nearplane=0;
    if (!lua_isnoneornil(L, 4))
    	nearplane = luaL_checknumber(L, 4);
    application->getApplication()->configureFrustum(fov,farplane,nearplane);

    return 0;
}


int ApplicationBinder::setWindowSize(lua_State* L){
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    int width = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);

    ::setWindowSize(width, height);

    return 0;
}

int ApplicationBinder::setFullScreen(lua_State* L){
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    bool fullScreen = lua_toboolean(L, 2);

    ::setFullScreen(fullScreen);
    return 0;
}

int ApplicationBinder::getDeviceName(lua_State *L)
{
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    lua_pushstring(L, ::getDeviceName().c_str());

    return 1;
}


int ApplicationBinder::set(lua_State *L)
{
    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    const char* what = luaL_checkstring(L, 2);

    std::stringstream arg;
    if ( g_checkStringProperty(true,what)){
        std::string arg4 = luaL_checkstring(L, 3);
        arg << arg4;
    }else{
        int arg1 = luaL_optnumber(L, 3, 0);
        int arg2 = luaL_optnumber(L, 4, 0);
        int arg3 = luaL_optnumber(L, 5, 0);
        arg << arg1;
        arg << "|";
        arg << arg2;
        arg << "|";
        arg << arg3;
    }

    g_setProperty(what, arg.str().c_str());


    return 0;
}

int ApplicationBinder::get(lua_State *L)
{

    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    const char* what = luaL_checkstring(L, 2);

    std::stringstream arg;
    if ( g_checkStringProperty(false,what)){
        std::string arg4 = luaL_checkstring(L, 3);
        arg << arg4;
    }else{
        int arg1 = luaL_optnumber(L, 3, 0);
        int arg2 = luaL_optnumber(L, 4, 0);
        int arg3 = luaL_optnumber(L, 5, 0);
        arg << arg1;
        arg << "|";
        arg << arg2;
        arg << "|";
        arg << arg3;
    }

    const char* propertyGet = g_getProperty(what,arg.str().c_str());
    std::string stringProp = propertyGet;

    char *returnedProperty = (char*)malloc((stringProp.length() + 1)*sizeof(char));

    strcpy(returnedProperty, propertyGet);
    std::string firstChar(returnedProperty,returnedProperty+1);

    unsigned int index = 0;

    if  (strcmp(firstChar.c_str(), "s") == 0){
        std::string resultChar(returnedProperty + 1);
        lua_pushstring(L, resultChar.c_str());
        index = 1;
    }else{

        const char* arrayProperty[10] = {""};
        arrayProperty[index] = strtok(returnedProperty,"|");

        while(arrayProperty[index] != NULL)
        {
            lua_pushnumber(L, atoi(arrayProperty[index]));
            ++index;
            arrayProperty[index] = strtok(NULL, "|");

        }
    }
    free(returnedProperty);

    return index;


}
