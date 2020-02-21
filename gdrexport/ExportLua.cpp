#include "ExportLua.h"
#include "ExportCommon.h"
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "lfs.h"

static ExportXml *currentXml=NULL;
static ExportContext *currentContext=NULL;
static bool inited=false;

static int getProperty(lua_State* L)
{
	if (!currentXml)
		return 0;
	const char *prop=luaL_checkstring(L,1);
	lua_pushstring(L,currentXml->GetProperty(QString(prop)).toStdString().c_str());
	return 1;
}

static int setProperty(lua_State* L)
{
	if (!currentXml)
		return 0;
	const char *prop=luaL_checkstring(L,1);
	const char *val=luaL_checkstring(L,2);
	currentXml->SetProperty(QString(prop),QString(val));
	return 0;
}

static int callXml(lua_State* L)
{
	if (!currentXml)
		return 0;
	const char *xml=luaL_checkstring(L,1);
	bool res=currentXml->ProcessRuleString(xml);
	lua_pushboolean(L,res?1:0);
	return 1;
}

extern "C" int luaopen_windows_hkey(lua_State *L);

static int bindAll(lua_State* L)
{
    static const luaL_Reg functionList[] = {
        {"getProperty", getProperty},
        {"setProperty", setProperty},
        {"callXml", callXml},
        {NULL, NULL},
    };

	lua_newtable(L);
    luaL_register(L, NULL, functionList);
    lua_setglobal(L, "Export");

    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcnfunction(L, luaopen_lfs,"open_lfs");
    lua_setfield(L, -2, "lfs");

#ifdef Q_OS_WIN32
    lua_pushcnfunction(L, luaopen_windows_hkey,"open_windows_hkey");
    lua_setfield(L, -2, "hkey");
#endif

    lua_pop(L, 2);

	return 0;
}

void ExportLUA_Init(ExportContext *ctx)
{
 ctx->L = luaL_newstate();
 luaL_openlibs(ctx->L);
 lua_pushcnfunction(ctx->L, bindAll, "bindAll");
 lua_call(ctx->L, 0, 0);
}

void ExportLUA_DonePlugins(ExportContext *ctx)
{
	 if (inited)
	 {
		 ExportXml *tmp=new ExportXml();
		 tmp->SetupProperties(ctx);
		 ExportLUA_CallCode(ctx,tmp,"Export._finish()");
		 delete tmp;
	 }
}

void ExportLUA_Cleanup(ExportContext *ctx)
{
 lua_close(ctx->L);
 ctx->L = NULL;
}

bool ExportLUA_CallFile(ExportContext *ctx,ExportXml *xml,const char *fn)
{
	if (!inited)
	{
		inited=true;
		ExportLUA_CallFile(ctx,xml,"Tools/export_init.lua");
	}
	if (luaL_loadfile(ctx->L,fn))
	{
    	const char *err=lua_tostring(ctx->L, -1);
    	ExportCommon::exportError("Lua error:%s\n",err);
        lua_pop(ctx->L, 2);
		ctx->exportError=true;
        return false;
	}
	currentXml=xml;
	currentContext=ctx;
    if (lua_pcall(ctx->L, 0, 0, 0) != 0)
    {
    	currentXml=NULL;
    	const char *err=lua_tostring(ctx->L, -1);
    	ExportCommon::exportError("Lua error:%s\n",err);
        lua_pop(ctx->L, 1);
		ctx->exportError=true;
        return false;
    }
	currentXml=NULL;
	currentContext=NULL;
    return true;
}

bool ExportLUA_CallCode(ExportContext *ctx,ExportXml *xml,const char *code)
{
	if (!inited)
	{
		inited=true;
		ExportLUA_CallFile(ctx,xml,"Tools/export_init.lua");
	}
	if (luaL_loadstring(ctx->L,code))
	{
    	const char *err=lua_tostring(ctx->L, -1);
    	ExportCommon::exportError("Lua error:%s\n",err);
        lua_pop(ctx->L, 2);
		ctx->exportError=true;
        return false;
	}
	currentXml=xml;
    if (lua_pcall(ctx->L, 0, 0, 0) != 0)
    {
    	currentXml=NULL;
    	const char *err=lua_tostring(ctx->L, -1);
    	ExportCommon::exportError("Lua error:%s\n",err);
        lua_pop(ctx->L, 1);
		ctx->exportError=true;
        return false;
    }
	currentXml=NULL;
    return true;
}
