#include "shaderbinder.h"
#include "Shaders.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include <luautil.h>

ShaderBinder::ShaderBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
        {"setConstant", setConstant},
        {NULL, NULL},
	};

	binder.createClass("Shader", NULL, create, destruct, functionList);

	lua_getglobal(L, "Shader");

	lua_pushinteger(L, ShaderProgram::CINT);
	lua_setfield(L, -2, "CINT");
	lua_pushinteger(L, ShaderProgram::CFLOAT);
	lua_setfield(L, -2, "CFLOAT");
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

	lua_pop(L, 1);

}

int ShaderBinder::create(lua_State* L)
{
	StackChecker checker(L, "ShaderBinder::create", 1);

    Binder binder(L);

	const char* vs = luaL_checkstring(L, 1);
	const char* ps = luaL_checkstring(L, 2);
    luaL_checktype(L, 3, LUA_TTABLE);
    luaL_checktype(L, 4, LUA_TTABLE);

	std::vector<ShaderProgram::ConstantDesc> constants;
	std::vector<ShaderProgram::DataDesc> datas;

    int n = luaL_getn(L, 3);  /* get size of table */
    for (int i=1; i<=n; i++) {
    	ShaderProgram::ConstantDesc cst={NULL,ShaderProgram::CINT,false,0,NULL};
        lua_rawgeti(L, 3, i);  /* push t[i] */
        luaL_checktype(L,-1,LUA_TTABLE); //Check table
        lua_getfield(L,-1,"name");
        cst.name=luaL_checkstring(L,-1);
        lua_getfield(L,-2,"type");
        cst.type=(ShaderProgram::ConstantType)luaL_checkinteger(L,-1);
        lua_getfield(L,-3,"vertex");
        cst.vertexShader=lua_toboolean(L,-1);
        lua_pop(L,4);
    	constants.push_back(cst);
      }

    n = luaL_getn(L, 4);  /* get size of table */
    for (int i=1; i<=n; i++) {
    	ShaderProgram::DataDesc cst={NULL,ShaderProgram::DFLOAT,0,0,0};
        lua_rawgeti(L, 4, i);  /* push t[i] */
        luaL_checktype(L,-1,LUA_TTABLE); //Check table
        lua_getfield(L,-1,"name");
        cst.name=luaL_checkstring(L,-1);
        lua_getfield(L,-2,"type");
        cst.type=(ShaderProgram::DataType)luaL_checkinteger(L,-1);
        lua_getfield(L,-3,"mult");
        cst.mult=luaL_checkinteger(L,-1);
        lua_getfield(L,-4,"slot");
        cst.slot=luaL_checkinteger(L,-1);
        lua_getfield(L,-5,"offset");
        cst.offset=luaL_checkinteger(L,-1);
        lua_pop(L,6);
    	datas.push_back(cst);
      }


	ShaderProgram::ConstantDesc clast={NULL,ShaderProgram::CINT,false,0,NULL};
	ShaderProgram::DataDesc dlast={NULL,ShaderProgram::DFLOAT,0,0,0};
	constants.push_back(clast);
	datas.push_back(dlast);
    ShaderProgram *shader=ShaderEngine::Engine->createShaderProgram(vs,ps,&(constants[0]),&(datas[0]));
    binder.pushInstance("Shader", shader);
	return 1;
}

int ShaderBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	ShaderProgram* shd = static_cast<ShaderProgram*>(ptr);
	delete shd;

	return 0;
}

int ShaderBinder::setConstant(lua_State* L)
{
	StackChecker checker(L, "ShaderBinder::setConstant", 0);

	Binder binder(L);

	ShaderProgram* shd = static_cast<ShaderProgram*>(binder.getInstance("Shader", 1));
/* TODO
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);

	bitmap->setAnchorPoint(x, y);
	*/

	return 0;
}
