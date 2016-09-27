#include "shaderbinder.h"
#include "Shaders.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include <luautil.h>
#include <stdlib.h>

ShaderBinder::ShaderBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
	        {"setConstant", setConstant},
	        {"isValid", isValid},
	    	{"getEngineVersion",getEngineVersion},
	    	{"getShaderLanguage",getShaderLanguage},
        {NULL, NULL},
	};

	binder.createClass("Shader", NULL, create, destruct, functionList);

	lua_getglobal(L, "Shader");

	lua_pushinteger(L, ShaderProgram::CINT);
	lua_setfield(L, -2, "CINT");
	lua_pushinteger(L, ShaderProgram::CFLOAT);
	lua_setfield(L, -2, "CFLOAT");
	lua_pushinteger(L, ShaderProgram::CFLOAT2);
	lua_setfield(L, -2, "CFLOAT2");
	lua_pushinteger(L, ShaderProgram::CFLOAT3);
	lua_setfield(L, -2, "CFLOAT3");
	lua_pushinteger(L, ShaderProgram::CFLOAT4);
	lua_setfield(L, -2, "CFLOAT4");
	lua_pushinteger(L, ShaderProgram::CTEXTURE);
	lua_setfield(L, -2, "CTEXTURE");
	lua_pushinteger(L, ShaderProgram::CMATRIX);
	lua_setfield(L, -2, "CMATRIX");

	lua_pushinteger(L, ShaderProgram::DBYTE);
	lua_setfield(L, -2, "DBYTE");
	lua_pushinteger(L, ShaderProgram::DUBYTE);
	lua_setfield(L, -2, "DUBYTE");
	lua_pushinteger(L, ShaderProgram::DSHORT);
	lua_setfield(L, -2, "DSHORT");
	lua_pushinteger(L, ShaderProgram::DUSHORT);
	lua_setfield(L, -2, "DUSHORT");
	lua_pushinteger(L, ShaderProgram::DINT);
	lua_setfield(L, -2, "DINT");
	lua_pushinteger(L, ShaderProgram::DFLOAT);
	lua_setfield(L, -2, "DFLOAT");

	lua_pushinteger(L, ShaderProgram::SysConst_None);
	lua_setfield(L, -2, "SYS_NONE");
	lua_pushinteger(L, ShaderProgram::SysConst_WorldViewProjectionMatrix);
	lua_setfield(L, -2, "SYS_WVP");
	lua_pushinteger(L, ShaderProgram::SysConst_Color);
	lua_setfield(L, -2, "SYS_COLOR");
	lua_pushinteger(L, ShaderProgram::SysConst_WorldMatrix);
	lua_setfield(L, -2, "SYS_WORLD");
	lua_pushinteger(L, ShaderProgram::SysConst_WorldInverseTransposeMatrix);
	lua_setfield(L, -2, "SYS_WIT");
	lua_pushinteger(L, ShaderProgram::SysConst_WorldInverseTransposeMatrix3);
	lua_setfield(L, -2, "SYS_WIT3");
	lua_pushinteger(L, ShaderProgram::SysConst_TextureInfo);
	lua_setfield(L, -2, "SYS_TEXTUREINFO");
	lua_pushinteger(L, ShaderProgram::SysConst_ParticleSize);
	lua_setfield(L, -2, "SYS_PARTICLESIZE");
	lua_pushinteger(L, ShaderProgram::SysConst_Timer);
	lua_setfield(L, -2, "SYS_TIMER");

	lua_pushinteger(L, ShaderProgram::Flag_None);
	lua_setfield(L, -2, "FLAG_NONE");
	lua_pushinteger(L, ShaderProgram::Flag_NoDefaultHeader);
	lua_setfield(L, -2, "FLAG_NO_DEFAULT_HEADER");
	lua_pushinteger(L, ShaderProgram::Flag_FromCode);
	lua_setfield(L, -2, "FLAG_FROM_CODE");

	lua_pop(L, 1);

}

int ShaderBinder::create(lua_State* L)
{
	StackChecker checker(L, "ShaderBinder::create", 1);

    Binder binder(L);

	const char* vs = luaL_checkstring(L, 1);
	const char* ps = luaL_checkstring(L, 2);
	int flags=luaL_checkinteger(L,3);
    luaL_checktype(L, 4, LUA_TTABLE);
    luaL_checktype(L, 5, LUA_TTABLE);

	std::vector<ShaderProgram::ConstantDesc> constants;
	std::vector<ShaderProgram::DataDesc> datas;

    int n = luaL_getn(L, 4);  /* get size of table */
    for (int i=1; i<=n; i++) {
    	ShaderProgram::ConstantDesc cst={"",ShaderProgram::CINT,1,ShaderProgram::SysConst_None,false,0,NULL};
        lua_rawgeti(L, 4, i);  /* push t[i] */
        luaL_checktype(L,-1,LUA_TTABLE); //Check table
        lua_getfield(L,-1,"name");
        cst.name=luaL_checkstring(L,-1);
        lua_getfield(L,-2,"type");
        cst.type=(ShaderProgram::ConstantType)luaL_checkinteger(L,-1);
        lua_getfield(L,-3,"vertex");
        cst.vertexShader=lua_toboolean(L,-1);
        lua_getfield(L,-4,"sys");
        cst.sys=(ShaderProgram::SystemConstant) luaL_optinteger(L,-1,0);
        lua_getfield(L,-5,"mult");
        cst.mult=luaL_optinteger(L,-1,1);
        lua_pop(L,6);
    	constants.push_back(cst);
      }

    n = luaL_getn(L, 5);  /* get size of table */
    for (int i=1; i<=n; i++) {
    	ShaderProgram::DataDesc cst={"",ShaderProgram::DFLOAT,0,0,0};
        lua_rawgeti(L, 5, i);  /* push t[i] */
        luaL_checktype(L,-1,LUA_TTABLE); //Check table
        lua_getfield(L,-1,"name");
        cst.name=luaL_checkstring(L,-1);
        lua_getfield(L,-2,"type");
        cst.type=(ShaderProgram::DataType)luaL_checkinteger(L,-1);
        lua_getfield(L,-3,"mult");
        cst.mult=luaL_checkinteger(L,-1);
        lua_getfield(L,-4,"slot");
        cst.slot=luaL_optinteger(L,-1,0);
        lua_getfield(L,-5,"offset");
        cst.offset=luaL_optinteger(L,-1,0);
        lua_pop(L,6);
    	datas.push_back(cst);
      }


	ShaderProgram::ConstantDesc clast={"",ShaderProgram::CINT,1,ShaderProgram::SysConst_None,false,0,NULL};
	ShaderProgram::DataDesc dlast={"",ShaderProgram::DFLOAT,0,0,0};
	constants.push_back(clast);
	datas.push_back(dlast);
    ShaderProgram *shader=ShaderEngine::Engine->createShaderProgram(vs,ps,flags,&(constants[0]),&(datas[0]));
    if (!shader->isValid())
    {
    	lua_pushstring(L,shader->compilationLog());
    	lua_error(L);
    }
    binder.pushInstance("Shader", shader);
	return 1;
}

int ShaderBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	ShaderProgram* shd = static_cast<ShaderProgram*>(ptr);
	shd->Release();

	return 0;
}

int ShaderBinder::getEngineVersion(lua_State* L)
{
	StackChecker checker(L, "ShaderBinder::getEngineVersion", 1);
	lua_pushstring(L,ShaderEngine::Engine->getVersion());
	return 1;
}

int ShaderBinder::getShaderLanguage(lua_State* L)
{
	StackChecker checker(L, "ShaderBinder::getShaderLanguage", 1);
	lua_pushstring(L,ShaderEngine::Engine->getShaderLanguage());
	return 1;
}

int ShaderBinder::setConstant(lua_State* L)
{
	StackChecker checker(L, "ShaderBinder::setConstant", 0);

	Binder binder(L);

	ShaderProgram* shd = static_cast<ShaderProgram*>(binder.getInstance("Shader", 1));

   // virtual void setConstant(int index,ConstantType type,const void *ptr);

	int idx=-1;
	if (lua_isstring(L,2))
	{
		idx=shd->getConstantByName(luaL_checkstring(L,2));
	}
	else
		idx = luaL_checknumber(L, 2);

	if (idx<0)
	{
		lua_pushstring(L,"Shader has no constant of that name/index");
		lua_error(L);
	}
	ShaderProgram::ConstantType type = (ShaderProgram::ConstantType) luaL_checkinteger(L, 3);
	int mult = luaL_checknumber(L, 4);
	int cm=1;
	switch (type)
	{
	case ShaderProgram::CFLOAT2: cm=2; break;
	case ShaderProgram::CFLOAT3: cm=3; break;
	case ShaderProgram::CFLOAT4: cm=4; break;
	case ShaderProgram::CMATRIX: cm=16; break;
	default: cm=1;
	}

	cm*=mult;
	switch (type)
	{
	case ShaderProgram::CINT:
	{
		int *m=(int *) malloc(sizeof(int)*cm);
		if (lua_istable(L,5))
		{
			for (int k=0;k<cm;k++)
			{
				lua_rawgeti(L, 5, k+1);
				m[k]=luaL_checkinteger(L,-1);
				lua_pop(L,1);
			}
		}
		else
		{
			for (int k=0;k<cm;k++)
				m[k]=luaL_checkinteger(L,5+k);
		}
		shd->setConstant(idx,type,mult,m);
		free(m);
		break;
	}
	case ShaderProgram::CFLOAT:
	case ShaderProgram::CFLOAT2:
	case ShaderProgram::CFLOAT3:
	case ShaderProgram::CFLOAT4:
	case ShaderProgram::CMATRIX:
	{
		float *m=(float *) malloc(sizeof(float)*cm);
		if (lua_istable(L,5))
		{
			for (int k=0;k<cm;k++)
			{
				lua_rawgeti(L, 5, k+1);
				m[k]=luaL_checknumber(L,-1);
				lua_pop(L,1);
			}
		}
		else
		{
			for (int k=0;k<cm;k++)
				m[k]=luaL_checknumber(L,5+k);
		}
		shd->setConstant(idx,type,mult,m);
		free(m);
		break;
	}
	case ShaderProgram::CTEXTURE:
		break;
	}

	return 0;
}

int ShaderBinder::isValid(lua_State* L)
{
	StackChecker checker(L, "ShaderBinder::isValid", 2);

	Binder binder(L);

	ShaderProgram* shd = static_cast<ShaderProgram*>(binder.getInstance("Shader", 1));
	lua_pushboolean(L,shd->isValid());
	lua_pushstring(L,shd->compilationLog());

	return 2;
}
