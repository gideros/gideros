#include "shaderbinder.h"
#include "Shaders.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include <luautil.h>
#include <stdlib.h>
#include <map>

class ShaderParser
{
public:
    static std::vector<std::string> parseQualifiers(std::string str, std::string kw);
    static std::map<std::string, ShaderProgram::ConstantType> ConstantTypeMap;
    static std::map<std::string, ShaderProgram::SystemConstant> SystemConstantMap;
    static std::map<std::string, ShaderProgram::DataType> DataTypeMap;
    static std::map<std::string, int> VectorSizeMap;
    static const char* defaultVertexShader;
    static const char* defaultFragmentShader;
};

std::vector<std::string> ShaderParser::parseQualifiers(std::string str, std::string kw) {
    std::vector<std::string> vec;
    size_t len = str.length();
    size_t l = kw.length();
    size_t pos = 0;
    size_t end;
    char prev, next;
    std::string name, type, size;
    size_t p, p1, p2;
    while(true) {
        pos = str.find(kw, pos);
        if (pos == std::string::npos || pos + l > len) break;

        // check char before keyword
        if (pos > 0) {
            prev = str.at(pos-1);
            if (!isspace(prev) && prev != ';') {pos = pos + l; continue;}
        }

        // check char after keyword
        pos += l;
        next = str.at(pos);
        if(!isspace(next)) continue;
        end = str.find(";", pos); // find end of declaration
        if (end == std::string::npos) break;

        // reset p1, p2 and size
        p1 = pos;
        p2 = end;
        size = "";

        // find variable name
        for (p = end-1; p >= pos; --p) if (!isspace(str.at(p))) {p2 = p; break;};
        if (str.at(p2) == ']') { // array size
            p1 = p2 - 1;
            for (p = p2-1; p >= pos; --p) if (str.at(p) == '[') {p2 = p; break;};
            size = str.substr(p2+1, p1 - p2);
            for (p = p2-1; p >= pos; --p) if (!isspace(str.at(p))) {p2 = p; break;};
        }

        for (p = p2; p >= pos; --p) if (isspace(str.at(p))) {p1 = p+1; break;};
        name = str.substr(p1, p2 - p1 + 1);

        // find variable type
        for (p = p1-1; p >= pos; --p) if (!isspace(str.at(p))) {p2 = p; break;};
        if (str.at(p2) == ']') { // array size
            p1 = p2 - 1;
            for (p = p2-1; p >= pos; --p) if (str.at(p) == '[') {p2 = p; break;};
            size = str.substr(p2+1, p1 - p2);
            for (p = p2-1; p >= pos; --p) if (!isspace(str.at(p))) {p2 = p; break;};
        }
        for (p = p2; p >= pos; --p) if (isspace(str.at(p))) {p1 = p+1; break;};
        type = str.substr(p1, p2 - p1 + 1);

        pos = end;

        vec.push_back(type);
        vec.push_back(name);
        vec.push_back(size);
    }
    return vec;
}

std::map<std::string, ShaderProgram::ConstantType> ShaderParser::ConstantTypeMap = {
            {"int", ShaderProgram::CINT},
            {"float", ShaderProgram::CFLOAT},
            {"vec2", ShaderProgram::CFLOAT2},
            {"vec3", ShaderProgram::CFLOAT3},
            {"vec4", ShaderProgram::CFLOAT4},
            {"mat4", ShaderProgram::CMATRIX},
            {"sampler2D", ShaderProgram::CTEXTURE},
};

std::map<std::string, ShaderProgram::SystemConstant> ShaderParser::SystemConstantMap = {
    {"SYS_NONE", ShaderProgram::SysConst_None},
    {"SYS_WVP", ShaderProgram::SysConst_WorldViewProjectionMatrix},
    {"SYS_COLOR", ShaderProgram::SysConst_Color},
    {"SYS_WIT", ShaderProgram::SysConst_WorldInverseTransposeMatrix},
    {"SYS_WORLD", ShaderProgram::SysConst_WorldMatrix},
    {"SYS_TEXTUREINFO", ShaderProgram::SysConst_TextureInfo},
    {"SYS_PARTICLESIZE", ShaderProgram::SysConst_ParticleSize},
    {"SYS_WIT3", ShaderProgram::SysConst_WorldInverseTransposeMatrix3},
    {"SYS_TIMER", ShaderProgram::SysConst_Timer},
};

std::map<std::string, ShaderProgram::DataType> ShaderParser::DataTypeMap = {
    //{"byte", ShaderProgram::DBYTE},
    {"byte", ShaderProgram::DUBYTE},
    {"unsigned byte", ShaderProgram::DUBYTE},
    {"short", ShaderProgram::DSHORT},
    {"unsigned short", ShaderProgram::DUSHORT},
    {"int", ShaderProgram::DINT},
    {"float", ShaderProgram::DFLOAT},
    {"vec2", ShaderProgram::DFLOAT},
    {"vec3", ShaderProgram::DFLOAT},
    {"vec4", ShaderProgram::DFLOAT},
};

std::map<std::string, int> ShaderParser::VectorSizeMap = {
    {"vec2", 2},
    {"vec3", 3},
    {"vec4", 4},
};

const char* ShaderParser::defaultVertexShader = "\
attribute highp vec3 vVertex;\
attribute mediump vec2 vTexCoord;\
uniform highp mat4 SYS_WVP;\
varying mediump vec2 fTexCoord;\
void main() {\
    vec4 vertex = vec4(vVertex,1.0);\
    gl_Position = SYS_WVP*vertex;\
    fTexCoord = vTexCoord;\
}\
";

const char* ShaderParser::defaultFragmentShader = "\
varying mediump vec2 fTexCoord;\
void main() {\
    if ((mod(5.5*(fTexCoord.x+1.0), 1.0) < 0.5)\
    ^^ (mod(5.5*(fTexCoord.y+1.0), 1.0) < 0.5))\
        gl_FragColor = vec4(0.0,0.0,0.0,1.0);\
    else gl_FragColor = vec4(1.0,1.0,1.0,1.0);\
}\
";

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

    const char* vs = lua_isnil(L, 1) ? ShaderParser::defaultVertexShader : luaL_checkstring(L, 1);
    const char* ps = lua_isnil(L, 2) ? ShaderParser::defaultFragmentShader : luaL_checkstring(L, 2);
    int flags = ShaderProgram::Flag_FromCode;

    std::vector<ShaderProgram::ConstantDesc> constants;
    std::vector<ShaderProgram::DataDesc> datas;

    if (lua_gettop(L) == 2) {
        std::vector<std::string> vec;
        std::string vartype, varname, varsize;

        vec = ShaderParser::parseQualifiers(std::string(ps), "uniform");
        for (size_t i = 0; i < vec.size(); i += 3) {
            vartype = vec.at(i);
            varname = vec.at(i+1);
            varsize = vec.at(i+2);

            if (!ShaderParser::ConstantTypeMap.count(vartype))
                luaL_error(L, ("Fragment shader: unknown type '" + vartype + "'").c_str());
            if (varsize.size() > 0) {
                for (size_t j = 0; j < varsize.size(); j++)
                    if (!isdigit(varsize.at(j)) && !isspace(varsize.at(j)))
                        luaL_error(L, ("Fragment shader: invalid array size '" + varsize + "'").c_str());
            } else varsize = "1";

            ShaderProgram::ConstantDesc cst = {
                varname,
                ShaderParser::ConstantTypeMap[vartype],
                atoi(varsize.c_str()),
                ShaderParser::SystemConstantMap.count(varname) ? ShaderParser::SystemConstantMap[varname]: ShaderProgram::SysConst_None,
                false,
                0,
                NULL
            };
            constants.push_back(cst);
        }

        vec = ShaderParser::parseQualifiers(std::string(vs), "uniform");
        for (size_t i = 0; i < vec.size(); i += 3) {
            vartype = vec.at(i);
            varname = vec.at(i+1);
            varsize = vec.at(i+2);

            if (!ShaderParser::ConstantTypeMap.count(vartype))
                luaL_error(L, ("Vertex shader: unknown type '" + vartype + "'").c_str());
            if (varsize.size() > 0) {
                for (size_t j = 0; j < varsize.size(); j++)
                    if (!isdigit(varsize.at(j)) && !isspace(varsize.at(j)))
                        luaL_error(L, ("Vertex shader: invalid array size '" + varsize + "'").c_str());
            } else varsize = "1";

            ShaderProgram::ConstantDesc cst = {
                varname,
                ShaderParser::ConstantTypeMap[vartype],
                atoi(varsize.c_str()),
                ShaderParser::SystemConstantMap.count(varname) ? ShaderParser::SystemConstantMap[varname]: ShaderProgram::SysConst_None,
                true,
                0,
                NULL
            };
            constants.push_back(cst);
        }

        vec = ShaderParser::parseQualifiers(std::string(vs), "attribute");
        ShaderProgram::DataDesc cst;
        cst = {"vVertex",ShaderProgram::DFLOAT,0,0,0}; datas.push_back(cst);
        cst = {"vColor",ShaderProgram::DFLOAT,0,0,0}; datas.push_back(cst);
        cst = {"vTexCoord",ShaderProgram::DFLOAT,0,0,0}; datas.push_back(cst);
        for (size_t i = 0; i < vec.size(); i += 3) {
            vartype = vec.at(i);
            varname = vec.at(i+1);
            varsize = vec.at(i+2);

            if (varname == "vVertex" || varname == "vColor" || varname == "vTexCoord") continue;

            if (!ShaderParser::DataTypeMap.count(vartype))
                luaL_error(L, ("Vertex shader: unknown type '" + vartype + "'").c_str());
            if (varsize.size() > 0) {
                for (size_t j = 0; j < varsize.size(); j++)
                    if (!isdigit(varsize.at(j)) && !isspace(varsize.at(j)))
                        luaL_error(L, ("Vertex shader: invalid array size '" + varsize + "'").c_str());
            } else varsize = "1";

            ShaderProgram::DataDesc cst = {
                varname,
                ShaderParser::DataTypeMap[vartype],
                (unsigned char) (atoi(varsize.c_str()) * (ShaderParser::VectorSizeMap.count(vartype) ? ShaderParser::VectorSizeMap[vartype] : 1)),
                (unsigned char) (i / 3 + 3),
                0
            };
            datas.push_back(cst);
        }
    } else {
        flags = luaL_checkinteger(L,3);
        luaL_checktype(L, 4, LUA_TTABLE);
        luaL_checktype(L, 5, LUA_TTABLE);

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
