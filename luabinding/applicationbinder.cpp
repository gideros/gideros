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
		{"getAppId", ApplicationBinder::getAppId},
		{"getDeviceSafeArea", ApplicationBinder::getDeviceSafeArea},
		{"setKeepAwake", ApplicationBinder::setKeepAwake},
		{"setKeyboardVisibility", ApplicationBinder::setKeyboardVisibility},
		{"getKeyboardModifiers", ApplicationBinder::getKeyboardModifiers},
		{"getLogicalTranslateX", ApplicationBinder::getLogicalTranslateX},
		{"getLogicalTranslateY", ApplicationBinder::getLogicalTranslateY},
		{"getLogicalScaleX", ApplicationBinder::getLogicalScaleX},
		{"getLogicalScaleY", ApplicationBinder::getLogicalScaleY},
		{"getLogicalBounds", ApplicationBinder::getLogicalBounds},
		{"getNativePath", ApplicationBinder::getNativePath},
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
        {"requestPermissions", ApplicationBinder::requestPermissions},
        {"checkPermission", ApplicationBinder::checkPermission},
		{"setTextInput", ApplicationBinder::setTextInput},
		{"setClipboard", ApplicationBinder::setClipboard},
		{"getClipboard", ApplicationBinder::getClipboard},
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
#define CONSTANT(value,name)	lua_pushstring(L, value); lua_setfield(L, -2, name);
	CONSTANT(CENTER,"CENTER");
	CONSTANT(CROP,"CROP");
	CONSTANT(FIT_HEIGHT,"FIT_HEIGHT");
	CONSTANT(FIT_WIDTH,"FIT_WIDTH");
	CONSTANT(LETTERBOX,"LETTERBOX");
	CONSTANT(NO_SCALE,"NO_SCALE");
	CONSTANT(PIXEL_PERFECT,"PIXEL_PERFECT");
	CONSTANT(STRETCH,"STRETCH");
#undef CONSTANT
#define CONSTANT(value,name)	lua_pushinteger(L, value); lua_setfield(L, -2, name);
	CONSTANT(0,"TEXTINPUT_CLASS_NONE");
	CONSTANT(1,"TEXTINPUT_CLASS_TEXT");
	CONSTANT(2,"TEXTINPUT_CLASS_NUMBER");
	CONSTANT(3,"TEXTINPUT_CLASS_PHONE");
	CONSTANT(4,"TEXTINPUT_CLASS_DATE");
	CONSTANT(0x10,"TEXTINPUT_TVARIANT_URI");
	CONSTANT(0x20,"TEXTINPUT_TVARIANT_EMAIL");
	CONSTANT(0x80,"TEXTINPUT_TVARIANT_PASSWORD");
	CONSTANT(0x10,"TEXTINPUT_DVARIANT_DATE");
	CONSTANT(0x20,"TEXTINPUT_DVARIANT_TIME");
	CONSTANT(0x10,"TEXTINPUT_NVARIANT_PASSWORD");
	CONSTANT(0x1000,"TEXTINPUT_TFLAG_CAPCHARACTERS");
	CONSTANT(0x2000,"TEXTINPUT_TFLAG_CAPWORDS");
	CONSTANT(0x4000,"TEXTINPUT_TFLAG_CAPSENTENCES");
	CONSTANT(0x8000,"TEXTINPUT_TFLAG_AUTOCORRECT");
	CONSTANT(0x20000,"TEXTINPUT_TFLAG_MULTILINE");
	CONSTANT(0x1000,"TEXTINPUT_NFLAG_SIGNED");
	CONSTANT(0x2000,"TEXTINPUT_NFLAG_DECIMAL");
#undef CONSTANT
	lua_pop(L, 1);

	binder.pushInstance("Application", NULL);
	lua_setglobal(L, "application");
}

int ApplicationBinder::getNativePath(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);
	const char *path = luaL_checkstring(L, 2);
    lua_pushstring(L,g_pathForFile(path));
	return 1;
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

int ApplicationBinder::getAppId(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	lua_pushstring(L, ::getAppId().c_str());

	return 1;
}

int ApplicationBinder::getDeviceSafeArea(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));
	bool logical=lua_toboolean(L,2);

	int idsl=0,idst=0,idsr=0,idsb=0;
	float dsl=0,dst=0,dsr=0,dsb=0;
	getSafeDisplayArea(idsl,idst,idsr,idsb);
	dsl=idsl; dst=idst; dsr=idsr; dsb=idsb;
	int dsw=application->getHardwareWidth();
	int dsh=application->getHardwareHeight();
	Orientation lor = application->orientation();
	Orientation hor = application->hardwareOrientation();
	Orientation dor = application->getApplication()->getDeviceOrientation();

	if ((lor == eLandscapeLeft || lor == eLandscapeRight) && (hor==eFixed))
		std::swap(dsw,dsh);

	float lsx=application->getLogicalScaleX();
	float lsy=application->getLogicalScaleY();
	float lox=application->getLogicalTranslateX();
	float loy=application->getLogicalTranslateY();

	Orientation rot=logical?hor:dor;
	int l;
	switch (rot) {
	case eLandscapeRight: l=dsl; dsl=dst; dst=dsr; dsr=dsb; dsb=l; break;
	case eLandscapeLeft: l=dsl; dsl=dsb; dsb=dsr; dsr=dst; dst=l; break;
	case ePortraitUpsideDown: l=dsl; dsl=dsr; dsr=l; l=dst; dst=dsb; dsb=l; break;
	default: break;
	}

	if (logical) {
		switch (lor) {
		case eLandscapeLeft: l=dsl; dsl=dst; dst=dsr; dsr=dsb; dsb=l; l=dsw; dsw=dsh; dsh=l; break;
		case eLandscapeRight: l=dsl; dsl=dsb; dsb=dsr; dsr=dst; dst=l; l=dsw; dsw=dsh; dsh=l; break;
		case ePortraitUpsideDown: l=dsl; dsl=dsr; dsr=l; l=dst; dst=dsb; dsb=l; break;
		default: break;
		}
		dsl=(dsl-lox)/lsx;
		dst=(dst-loy)/lsy;
		dsr=(-dsr-lox+dsw)/lsx;
		dsb=(-dsb-loy+dsh)/lsy;
	}
	else
	{
		dsr=dsw-dsr;
		dsb=dsh-dsb;
	}

	lua_pushnumber(L,dsl);
	lua_pushnumber(L,dst);
	lua_pushnumber(L,dsr);
	lua_pushnumber(L,dsb);

	return 4;
}

int ApplicationBinder::getLogicalBounds(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	int dsw=application->getHardwareWidth();
	int dsh=application->getHardwareHeight();
	Orientation lor = application->orientation();

	if ((application->hardwareOrientation()!=eFixed)&&(lor == eLandscapeLeft || lor == eLandscapeRight))
		std::swap(dsw,dsh);

	float lsx=application->getLogicalScaleX();
	float lsy=application->getLogicalScaleY();
	float lox=application->getLogicalTranslateX();
	float loy=application->getLogicalTranslateY();

	lua_pushnumber(L,-lox/lsx);
	lua_pushnumber(L,-loy/lsy);
	lua_pushnumber(L,(-lox+dsw)/lsx);
	lua_pushnumber(L,(-loy+dsh)/lsy);

	return 4;
}

int ApplicationBinder::setKeepAwake(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	::setKeepAwake(lua_toboolean(L, 2) != 0);

	return 0;
}

static int getsetClipboardLuaCb(lua_State *L,void *result) {
	gapplication_ClipboardResponseCb *r=(gapplication_ClipboardResponseCb *)result;
	lua_pushboolean(L,r->result);
	lua_pushstring(L,r->data);
	lua_pushstring(L,r->mimeType);
    return 3;
}

extern "C"
void gapplication_clipboardCallback(int luaFuncRef,int result,const char *data, const char *type)
{
	gapplication_ClipboardResponseCb *r=(gapplication_ClipboardResponseCb *)gevent_CreateEventStruct2(
			sizeof(gapplication_ClipboardResponseCb),
			offsetof(gapplication_ClipboardResponseCb,data),data,
			offsetof(gapplication_ClipboardResponseCb,mimeType),type);
	r->result=result>0;
	gapplication_luaCallback(luaFuncRef,r,getsetClipboardLuaCb);
}

int ApplicationBinder::setClipboard(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	const char *cdata=luaL_optstring(L,2,NULL);
	std::string data=cdata?cdata:"";
	std::string type=luaL_optstring(L,3,cdata?"text/plain":"");
	int fref=LUA_NOREF;
	if (lua_isfunction(L,4)){
		lua_pushvalue(L,4);
        fref=lua_ref(L,-1);
		lua_pop(L,1);
	}

	int ret=::setClipboard(data,type,fref);
	if (fref==LUA_NOREF) {
		lua_pushboolean(L,ret>0);
		return 1;
	}
	else {
		if (ret!=0)
			gapplication_clipboardCallback(fref,ret>0,NULL,NULL);
	}
	return 0;
}

int ApplicationBinder::getClipboard(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	std::string data;
	std::string type=luaL_optstring(L,2,"text/plain");
	int fref=LUA_NOREF;
    if (lua_isfunction(L,3)){
        lua_pushvalue(L,3);
        fref=lua_ref(L,-1);
		lua_pop(L,1);
	}
	int ret=::getClipboard(data,type,fref);
	if (fref==LUA_NOREF) {
		if (ret>0)
		{
			lua_pushstring(L,data.c_str());
			lua_pushstring(L,type.c_str());
			return 2;
		}
		else
			return 0;
	}
	else {
		if (ret!=0)
			gapplication_clipboardCallback(fref,ret>0,data.c_str(),type.c_str());
	}
	return 0;
}

int ApplicationBinder::setKeyboardVisibility(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	lua_pushboolean(L,::setKeyboardVisibility(lua_toboolean(L, 2) != 0));

	return 1;
}

int ApplicationBinder::setTextInput(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	lua_pushboolean(L,::setTextInput(luaL_checkinteger(L,2),
			luaL_optstring(L,3,""),
			luaL_optinteger(L,4,0),
			luaL_optinteger(L,5,luaL_optinteger(L,4,0)),
			luaL_optstring(L,6,""),
			luaL_optstring(L,7,""),
			luaL_optstring(L,8,"")
			));

	return 1;
}

int ApplicationBinder::getKeyboardModifiers(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	lua_pushinteger(L,::getKeyboardModifiers());

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

	float alpha=luaL_optnumber(L,3,1.0);

	application->getApplication()->setBackgroundColor(r/255.f, g/255.f, b/255.f,alpha);

	return 0;
}

int ApplicationBinder::getBackgroundColor(lua_State* L)
{
	Binder binder(L);
	(void)binder.getInstance("Application", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	float r, g, b,a;
	application->getApplication()->getBackgroundColor(&r, &g, &b, &a);

	int ir = std::min((int)(r * 256), 255);
	int ig = std::min((int)(g * 256), 255);
	int ib = std::min((int)(b * 256), 255);

	lua_pushinteger(L, (ir << 16) | (ig << 8) | ib);
	lua_pushnumber(L,a);

	return 2;
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
        luaL_error(L, "%s", status.errorString());
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
	case eFixed:
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
        luaL_error(L, "%s", status.errorString());
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
        luaL_error(L, "%s", status.errorString());
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
    lua_pushliteral(L, __DATE__ " " __TIME__);
#ifdef GIDEROS_GIT_HASH
    lua_pushliteral(L, GIDEROS_GIT_HASH);
#else
    lua_pushliteral(L,"undefined");
#endif
    return 3;
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
	case eFixed:
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

int ApplicationBinder::checkPermission(lua_State *L)
{

    Binder binder(L);
    (void)binder.getInstance("Application", 1);

    const char* what = luaL_checkstring(L, 2);
    bool res=false;
#ifdef __ANDROID__
    res=::gapplication_checkPermission(what);
#endif
    lua_pushboolean(L,res);
    return 1;
}

int ApplicationBinder::requestPermissions(lua_State *L)
{

    Binder binder(L);
    (void)binder.getInstance("Application", 1);
    std::vector<std::string> perms;

    if (lua_istable(L,2)) {
    	size_t n=lua_objlen(L,2);
    	for (size_t i=1;i<=n;i++)
    	{
    		lua_rawgeti(L,2,i);
        	perms.push_back(luaL_checkstring(L,-1));
        	lua_pop(L,1);
    	}
    }
    else
    	perms.push_back(luaL_checkstring(L,2));
#ifdef __ANDROID__
    ::gapplication_requestPermissions(perms);
#endif
    return 0;
}
