--!NEEDS:luac_codegen.lua
--!NEEDS:luau_codegen.lua

local codegen=codegen_l
if not string.dump then codegen=codegen_u end

local OPTYPE_MAP={
	["hF44*hF4"]="hF4",
	["hF33*hF3"]="hF3",
	["hF1*hF2"]="hF2",
	["hF1*hF3"]="hF3",
	["hF1*hF4"]="hF4",
	["hF1+hF2"]="hF2",
	["hF1+hF3"]="hF3",
	["hF1+hF4"]="hF4",
	["hF1-hF2"]="hF2",
	["hF1-hF3"]="hF3",
	["hF1-hF4"]="hF4",
	["hF1/hF2"]="hF2",
	["hF1/hF3"]="hF3",
	["hF1/hF4"]="hF4",
	["hF1%hF2"]="hF2",
	["hF1%hF3"]="hF3",
	["hF1%hF4"]="hF4",
}

local GFUNC_MAP={
	["discard"]={type="func", value="discard"},
	["texture2D"]={type="func", value="texture2D", rtype="lF4"},
	["shadow2D"]={type="func", value="shadow2D", rtype="hF4"},
	["sin"]={type="func", value="sin", rtype="1", acount=1},
	["cos"]={type="func", value="cos", rtype="1", acount=1},
	["floor"]={type="func", value="floor", rtype="1", acount=1},
	["dot"]={type="func", value="dot", rtype="hF1", acount=2},
	["mix"]={type="func", value="mix", rtype="1", acount=3},
	["tan"]={type="func", value="tan", rtype="1", acount=1},
	["asin"]={type="func", value="asin", rtype="1", acount=1},
	["acos"]={type="func", value="acos", rtype="1", acount=1},
	["atan"]={type="func", value="atan", rtype="1", acount=1},
	["atan2"]={type="func", value="atan2", rtype="1", acount=2},
	["abs"]={type="func", value="abs", rtype="1", acount=1},
	["sign"]={type="func", value="sign", rtype="1", acount=1},
	["ceil"]={type="func", value="ceil", rtype="1", acount=1},
	["fract"]={type="func", value="fract", rtype="1", acount=1},
	["clamp"]={type="func", value="clamp", rtype="1", acount=3},
	["step"]={type="func", value="step", rtype="2", acount=3},
	["smoothstep"]={type="func", value="smoothstep", rtype="2", acount=3},
	["length"]={type="func", value="length", rtype="hF1", acount=1},
	["distance"]={type="func", value="distance", rtype="hF1", acount=2},
	["cross"]={type="func", value="cross", rtype="1", acount=2},
	["normalize"]={type="func", value="normalize", rtype="1", acount=1},
	["reflect"]={type="func", value="reflect", rtype="1", acount=2},
	["dFdx"]={type="func", value="dFdx", rtype="1", acount=1},
	["dFdy"]={type="func", value="dFdy", rtype="1", acount=1},
	["inversesqrt"]={type="func", value="inversesqrt", rtype="1", acount=1},
	["max"]={type="func", value="max", rtype="1", acount=2},
	["min"]={type="func", value="min", rtype="1", acount=2},
	["pow"]={type="func", value="pow", rtype="1", acount=2},
	["mod"]={type="func", value="mod", rtype="1", acount=2},
	["InstanceID"]={type="cvar", value="gl_InstanceID", vtype="hI1"},
	["FragCoord"]={type="cvar", value="gl_FragCoord", vtype="hF4"},
}

local platform=application:getDeviceInfo()
local isHTML = platform and platform=="Web"
local glversion=Shader.getEngineVersion()
local isES3=(glversion=="GLES3")
local isWebGL2 = (Shader.getProperties().version or ""):find("WebGL 2")

local function populateGMap(gmap,tmap,funcs,const)
	for k,v in pairs(tmap) do
		local si=v:find(" ")
		if si then v=v:sub(si+1) end
		gmap[k]={type="func", value=v, rtype=k}
	end
	for k,v in pairs(GFUNC_MAP) do gmap[k]=v end
	if funcs then
		for k,v in ipairs(funcs) do
			gmap[v.name]={type="func", value=v.name, rtype=v.rtype, acount=v.acount, fdef=v}
		end
	end
	if const then
		for k,v in ipairs(const) do 
			gmap[v.name]={type="cvar", value=v.value, vtype=v.type}
		end
	end
end

local function shType(v)
	local p
	if v.type==Shader.CFLOAT then p="hF1"
	elseif v.type==Shader.CFLOAT2 then p="hF2"
	elseif v.type==Shader.CFLOAT3 then p="hF3"
	elseif v.type==Shader.CFLOAT4 then p="hF4"
	elseif v.type==Shader.CINT then p="hI1"
	elseif v.type==Shader.CTEXTURE then
		if v.subtype=="shadow" then p="hS1"
		else p="lT1"
		end
	elseif v.type==Shader.CMATRIX then p="hF44"
	end
	assert(p,"Type not mapped:"..v.type.." for "..v.name)
	return p
end

local function shAType(v)
	if v.mult==0 then return nil end
	local p
	if v.type==Shader.DFLOAT then p="hF"
	elseif v.type==Shader.DUBYTE then p="lF"
	end
	assert(p,"Type not mapped:"..v.type.." for "..v.name)
	p=p..(v.mult or 1)
	return p
end

function Shader.lua(vf,ff,opt,uniforms,attrs,varying,funcs,const,debug)
	local lang=Shader.getShaderLanguage()
	local mtd=Shader["lua_"..lang]
	assert(mtd,"Language not supported: "..lang)
	
	local _vshader,_fshader=mtd(vf,ff,opt,uniforms,attrs,varying,funcs or {},const)
	if funcs then
		for _,fg in ipairs(funcs) do fg.code=nil end
	end

	if debug then
		print("VSHADER_CODE:\n".._vshader)	
		print("FSHADER_CODE:\n".._fshader)
	end
	if isHTML or isES3 then opt = opt | Shader.FLAG_NO_DEFAULT_HEADER end
	return Shader.new(_vshader,_fshader,Shader.FLAG_FROM_CODE|opt,uniforms,attrs)
end

local function GEN_RETURN(rval)
	if rval then return "return "..rval..";\n"
	else return ""
	end
end

local function genFunction(fg,gmap,tmap,omap,ophandler)
	local _code=""
	local oh={}
	for k,v in pairs(ophandler) do oh[k]=v end
	oh.RETURN=GEN_RETURN
	local ff=gmap[fg.name]._fcode
	local fccode=gmap[fg.name].fccode
	assert(ff or fccode,"Function "..fg.name.." not defined")
	if ff or fccode then
		if fg.rtype then _code=_code..tmap[fg.rtype]
		else _code=_code.."void"
		end
		_code=_code.." "..fg.name.."("
		local _args=""
		local amap={}
		for k,v in ipairs(fg.args or {}) do
			amap[k]={type="var",name=v.name,value=v.name,vtype=v.type}
			_args=_args..","..tmap[v.type].." "..v.name
		end
		local fcode=fccode
		if not fcode then fcode=codegen(ff,amap,gmap,tmap,omap,oh) end
		local callarg=""
		if ff and ff.extra_args then
			for k,v in ipairs(ff.extra_args or {}) do 
				_args=_args..v.decl
				if #callarg>0 then callarg=callarg.."," end
				callarg=callarg..v.use
			end
		end
		if #_args then _args=_args:sub(2) end
		_code=_code.._args..") {\n"
		_code=_code..fcode.."}\n"
		fg.code=_code
		gmap[fg.name].callarg=callarg
	end
end

local function genFunctions(gmap,tmap,omap,ophandler,funcs)
	local _code=""
	for _,fg in ipairs(funcs) do
		if not fg.code then genFunction(fg,gmap,tmap,omap,ophandler) end
		_code=_code..fg.code
	end
	return _code
end

local function _GLSL_GENOP(o,a,b)
	if o=="%" then return ("mod(%s,%s)"):format(a.value,b.value) end
end

local function _GLSL_GENOPEQ(o,a,b)
	if o=="%" then return ("%s=mod(%s,%s)"):format(a.value,a.value,b.value) end
end

function Shader.lua_glsl(vf,ff,opt,uniforms,attrs,varying,funcs,const)
	local amap={}
	local gmap={}
	local tmap={
		hF1="highp float",
		hF2="highp vec2",
		hF3="highp vec3",
		hF4="highp vec4",
		lF1="lowp float",
		lF2="lowp vec2",
		lF3="lowp vec3",
		lF4="lowp vec4",
		hI1="highp int",
		hT1="highp sampler2D",
		lT1="lowp sampler2D",
		hS1="highp sampler2DShadow",
		hF22="highp mat2",
		hF33="highp mat3",
		hF44="highp mat4",
		BOOL="bool",
	}
	local omap=OPTYPE_MAP
	populateGMap(gmap,tmap,funcs,const)
	gmap["discard"]={type="func", value="discard", evaluate=function (ff,fn,args) return "discard" end}
	gmap["atan2"]={type="func", value="atan",rtype="1", acount=2}
	
	local _headers = nil
	local _eheaders = ""
	if isHTML then
		_headers=[[
#define shadow2D(tex,pt) vec4(shadow2DEXT(tex,pt),0.0,0.0,0.0)
]]
		if isWebGL2 then
		_headers=[[#version 300 es
#extension GL_OES_standard_derivatives : enable
#define shadow2D(tex,pt) vec4(texture(tex,pt),0.0,0.0,0.0)
#define texture2D(tex,pt) texture(tex,pt)
  ]]
		end
	elseif isES3 then
		_headers=[[#version 300 es
#extension GL_OES_standard_derivatives : enable
#define shadow2D(tex,pt) vec4(texture(tex,pt),0.0,0.0,0.0)
#define texture2D(tex,pt) texture(tex,pt)
	]]
	else
		_headers=[[#ifdef GLES2
#extension GL_OES_standard_derivatives : enable
#extension GL_EXT_shadow_samplers : enable
#define shadow2D(tex,pt) vec4(shadow2DEXT(tex,pt),0.0,0.0,0.0)
#endif
]]
	end

	local isGLSL300=isES3 or isWebGL2

	local _code=_eheaders.._headers
	for k,v in ipairs(attrs) do 
		local atype=shAType(v)
		if atype then
			amap[k]={type="cvar", name=v.name, value=v.name, vtype=atype}
			assert(tmap[atype],"Attribute type not handled :"..atype)
			if isGLSL300 then _code=_code..("in %s %s;\n"):format(tmap[atype],v.name)
			else _code=_code..("attribute %s %s;\n"):format(tmap[atype],v.name)
			end
		else 
			amap[k]={}
		end
	end
	for k,v in ipairs(uniforms) do 
		if v.vertex then
			local atype=shType(v)
			local mult,vmult="",nil
			if v.mult and v.mult>1 then mult="["..v.mult.."]" vmult=v.mult end
			gmap[v.name]={type="cvar", value=v.name, vtype=atype, vmult=mult}
			_code=_code..("uniform %s %s%s;\n"):format(tmap[atype],v.name,mult)
		end
	end
	for k,v in ipairs(varying) do 
		local atype=shType(v)
		gmap[v.name]={type="var", value=v.name, vtype=atype}
		if isGLSL300 then _code=_code..("out %s %s;\n"):format(tmap[atype],v.name)
		else _code=_code..("varying %s %s;\n"):format(tmap[atype],v.name)
		end
	end
	_code=_code.."\n void main() {\n"
	_code=_code..codegen(vf,amap,gmap,tmap,omap,	{
		RETURN=function(rval)
			if rval then return "gl_Position = "..rval..";\n"
			else return ""
			end
		end,
		GENOP=_GLSL_GENOP,
		GENOPEQ=_GLSL_GENOPEQ,
	})
	_code=_code.."}"
	
	local _vshader=_code
	
	local outVariable="gl_FragColor"
	if isGLSL300 then outVariable="_gl_FragColor" end
	
	_code=_eheaders.._headers
	for k,v in ipairs(uniforms) do 
		if not v.vertex then
			local atype=shType(v)
			local mult,vmult="",nil
			if v.mult and v.mult>1 then mult="["..v.mult.."]" vmult=v.mult end
			gmap[v.name]={type="cvar", value=v.name, vtype=atype, vmult=mult}
			_code=_code..("uniform %s %s%s;\n"):format(tmap[atype],v.name,mult)
		end
	end
	for k,v in ipairs(varying) do 
		local atype=shType(v)
		gmap[v.name]={type="cvar", value=v.name, vtype=atype}
		if isGLSL300 then _code=_code..("in %s %s;\n"):format(tmap[atype],v.name)
		else _code=_code..("varying %s %s;\n"):format(tmap[atype],v.name)
		end
	end
	if isGLSL300 then _code=_code.."out lowp vec4 _gl_FragColor;\n" end

	local mainCode=codegen(ff,amap,gmap,tmap,omap, {
		RETURN=function(rval)
			if rval then return outVariable.." = "..rval..";\n"
			else return ""
			end
		end,
		GENOP=_GLSL_GENOP,
		GENOPEQ=_GLSL_GENOPEQ,
	})
	_code=_code.."\n"
	..genFunctions(gmap,tmap,omap,{GENOP=_GLSL_GENOP, GENOPEQ=_GLSL_GENOPEQ,},funcs)
	.."void main() {\n"..mainCode.."}"
	
	local _fshader=_code
	
	return _vshader,_fshader	
end

local function _MSL_GENOP(o,a,b)
	if o=="%" then return ("fmod(%s,%s)"):format(a.value,b.value) end
end

local function _MSL_GENOPEQ(o,a,b)
	if o=="%" then return ("%s=fmod(%s,%s)"):format(a.value,a.value,b.value) end
end

local function _MSL_SWIZZLE(p)
	return p:gsub(".",{ s="x",t="y",p="z", q="w" })
end

function Shader.lua_msl(vf,ff,opt,uniforms,attrs,varying,funcs,const)
	local amap={}
	local gmap={}
	local tmap={
		hF1="float",
		hF2="float2",
		hF3="float3",
		hF4="float4",
		lF1="half",
		lF2="half2",
		lF3="half3",
		lF4="half4",
		hI1="int",
		hT1="sampler2D",
		lT1="sampler2D",
		hF22="float2x2",
		hF33="float3x3",
		hF44="float4x4",
		BOOL="bool",
	}
	local omap=OPTYPE_MAP
	populateGMap(gmap,tmap,funcs,const)
	gmap["discard"]={type="func", value="discard_fragment"}
	gmap["texture2D"]={type="func", value="texture2D", evaluate=function (ff,fn,tex,sp)
		ff._MSL_args=ff._MSL_args or { }
		if not ff._MSL_args["_smp_"..tex.value] and ff._MSL_spec then
			ff._MSL_args["_smp_"..tex.value]=true
			ff._MSL_args["_tex_"..tex.value]=true
			ff.extra_args=ff.extra_args or { }
			table.insert(ff.extra_args,ff._MSL_spec[tex.value])
		end
		return ("_tex_%s.sample(_smp_%s, %s)"):format(tex.value,tex.value,sp.value)
	end, rtype="lF4"}
	gmap["shadow2D"]={type="func", value="shadow2D", evaluate=function (ff,fn,tex,sp)
		ff._MSL_args=ff._MSL_args or { }
		if not ff._MSL_args["_smp_"..tex.value] and ff._MSL_spec then
			ff._MSL_args["_smp_"..tex.value]=true
			ff._MSL_args["_tex_"..tex.value]=true
			ff.extra_args=ff.extra_args or { }
			table.insert(ff.extra_args,ff._MSL_spec[tex.value])
		end
		return ("float4(_tex_%s.sample_compare(_smp_%s, (%s).xy,((%s).z-0.5)*2.0),0.0,0.0,0.0)"):format(tex.value,tex.value,sp.value,sp.value)
	end, rtype="hF4"}
	gmap["mod"]={type="func", value="fmod", rtype="1", acount=2}
	gmap["dFdx"]={type="func", value="dfdx", rtype="1", acount=1}
	gmap["dFdy"]={type="func", value="dfdy", rtype="1", acount=1}
	gmap["inversesqrt"]={type="func", value="rsqrt", rtype="1", acount=1}
	gmap["FragCoord"]={type="cvar", value="vert.gl_Position", vtype="hF4"}

	_code=[=[
#include <metal_stdlib>
using namespace metal;
]=]

	_code=_code.."struct InVertex {\n"
	for k,v in ipairs(attrs) do 
		local atype=shAType(v)
		if atype then
			amap[k]={type="cvar", name=v.name, value="inVertex."..v.name, vtype=atype} 
			assert(tmap[atype],"Attribute type not handled :"..atype)
			_code=_code..("%s %s [[attribute(%d)]];\n"):format(tmap[atype],v.name,v.slot)
		else 
			amap[k]={}
		end
	end
	_code=_code.."};\n"
	
	local _fargs=""
	local _ucode="struct Uniforms {\n"
	local _texc=0
	local _msl_spec={}
	for k,v in ipairs(uniforms) do 
		local atype=shType(v)
		if v.type==Shader.CTEXTURE then
			gmap[v.name]={type="cvar", value=v.name, vtype=atype}
			local exarg
			if v.subtype=="shadow" then
				exarg=(",depth2d<float> _tex_%s [[texture(%d)]], sampler _smp_%s [[sampler(%d)]]"):format(v.name,_texc,v.name,_texc)
			else
				exarg=(",texture2d<half> _tex_%s [[texture(%d)]], sampler _smp_%s [[sampler(%d)]]"):format(v.name,_texc,v.name,_texc)
			end
			_fargs=_fargs..exarg
			_msl_spec[v.name]={ decl=exarg, use=("_tex_%s,_smp_%s"):format(v.name,v.name) }
			_texc+=1
		else
			local mult,vmult="",nil
			if v.mult and v.mult>1 then mult="["..v.mult.."]" vmult=v.mult end
			_ucode=_ucode..("%s %s%s;\n"):format(tmap[atype],v.name,mult)
			gmap[v.name]={type="cvar", value="uniforms."..v.name, vtype=atype, vmult=mult}
		end
	end
	_ucode=_ucode.."};\n"

	local _pcode="struct PVertex {\n"
	_pcode=_pcode.."    float4 gl_Position [[position]];\n"
	for k,v in ipairs(varying) do 
		local atype=shType(v)
		gmap[v.name]={type="var", value="outVert."..v.name, vtype=atype}
		_pcode=_pcode..("%s %s [[user(%s)]];\n"):format(tmap[atype],v.name,v.name)
	end
	_pcode=_pcode.."};\n"

	_code=_code.._ucode.._pcode..(([=[
vertex PVertex vmain(InVertex inVertex [[stage_in]],
			uint gl_InstanceID [[instance_id]],
           constant Uniforms &uniforms [[buffer(0)]]%s)
{
    PVertex outVert;
]=]):format(_fargs))
	_code=_code..codegen(vf,amap,gmap,tmap,omap,	{
		RETURN=function(rval)
			if rval then return "outVert.gl_Position = "..rval.."; return outVert;\n"
			else return "return outVert;"
			end
		end,
		SWIZZLE=_MSL_SWIZZLE,
		GENOP=_MSL_GENOP,
		GENOPEQ=_MSL_GENOPEQ,
	})
	_code=_code.."}"
	
	local _vshader=_code
	
	for k,v in ipairs(varying) do
		local atype=shType(v)
		gmap[v.name]={type="cvar", value="vert."..v.name, vtype=atype}
	end
	
	local _code=[=[
#include <metal_stdlib>
using namespace metal;
]=]

	local mainCode=codegen(ff,amap,gmap,tmap,omap,	{
		RETURN=GEN_RETURN,
		GENOP=_MSL_GENOP,
		GENOPEQ=_MSL_GENOPEQ,
		SWIZZLE=_MSL_SWIZZLE,
		SUBFUNC=function(f,fg)
			fg._fcode._MSL_spec=_msl_spec
			genFunction(fg.fdef,gmap,tmap,omap,{	GENOP=_MSL_GENOP,GENOPEQ=_MSL_GENOPEQ,})
		end
	})
	_code=_code.._ucode.._pcode
		..genFunctions(gmap,tmap,omap,{	GENOP=_MSL_GENOP,GENOPEQ=_MSL_GENOPEQ,},funcs)
		..(([=[
fragment half4 fmain(PVertex vert [[stage_in]],
                    constant Uniforms &uniforms [[buffer(0)]]%s)
{
]=]):format(_fargs))
	_code=_code..mainCode.."}\n"

	local _fshader=_code
	
	return _vshader,_fshader	
end

local function ismatrix(v)
	return ((tonumber(v.vtype:sub(3)) or 0)>=22)
end

local function _HLSL_GENOP(o,a,b)
	if o=="*" then
		if ismatrix(a) or ismatrix(b) then
			return ("mul(%s,%s)"):format(a.value,b.value)
		end
	elseif o=="%" then
		return ("fmod(%s,%s)"):format(a.value,b.value)
	end
end

local function _HLSL_GENOPEQ(o,a,b)
	if o=="*" then
		if ismatrix(a) or ismatrix(b) then
			return ("%s=mul(%s,%s)"):format(a.value,a.value,b.value)
		end
	elseif o=="%" then
		return ("%s=fmod(%s,%s)"):format(a.value,a.value,b.value)
	end
end

local function _HLSL_ARGSWIZZLE(func,...)
end

local function _HLSL_SWIZZLE(p)
	return p:gsub(".",{ s="x",t="y",p="z", q="w" })
end

function Shader.lua_hlsl(vf,ff,opt,uniforms,attrs,varying,funcs,const)
	local amap={}
	local gmap={}
	local tmap={
		hF1="float",
		hF2="float2",
		hF3="float3",
		hF4="float4",
		lF1="half",
		lF2="half2",
		lF3="half3",
		lF4="half4",
		hI1="int",
		hT1="sampler2D",
		lT1="sampler2D",
		hF22="float2x2",
		hF33="float3x3",
		hF44="float4x4",
		BOOL="bool",
	}
	local omap=OPTYPE_MAP
	populateGMap(gmap,tmap,funcs,const)
	for k,_ in pairs(tmap) do
		local np=tonumber(k:sub(3)) or 0
		if np>1 and np<=4 then
			gmap[k].vecSize=np
			--gmap[k].evaluate=_HLSL_ARGSWIZZLE
		end
	end

	gmap["discard"]={type="func", value="discard", evaluate=function (ff,fn,args) return "discard" end}
	gmap["texture2D"]={type="func", value="texture2D", evaluate=function (ff,fn,tex,sp)
		return ("_tex_%s.Sample(_smp_%s, %s)"):format(tex.value,tex.value,sp.value)
	end, rtype="lF4"}
	gmap["shadow2D"]={type="func", value="shadow2D", evaluate=function (ff,fn,tex,sp)
		return ("_tex_%s.SampleCmp(_smp_%s, (%s).xy,((%s).z-0.5)*2.0)"):format(tex.value,tex.value,sp.value,sp.value)
	end, rtype="hF4"}
	gmap["mix"]={type="func", value="lerp", rtype="1", acount=3}
	gmap["atan2"]={type="func", value="atan2", 
		evaluate=function (ff,fn,y,x)
			return ("atan2(%s,%s)"):format(x.value,y.value)
		end,rtype="1", acount=2}
	gmap["fract"]={type="func", value="frac", rtype="1", acount=1}
	gmap["dFdx"]={type="func", value="ddx", rtype="1", acount=1}
	gmap["dFdy"]={type="func", value="ddy", rtype="1", acount=1}
	gmap["inversesqrt"]={type="func", value="rsqrt", rtype="1", acount=1}
	gmap["mod"]={type="func", value="fmod", rtype="1", acount=2}
	gmap["hF44"]={type="func", value="float4x4", rtype="hF44", acount=4, evaluate=function(ff,fn,a,b,c,d)
		return ("transpose(float4x4(%s,%s,%s,%s))"):format(a.value,b.value,c.value,d.value)
	end}

	local _code,_dcode="",""

	local _vargs,_fargs="",""
	for k,v in ipairs(attrs) do 
		local atype=shAType(v)
		if atype then
			amap[k]={type="cvar", name=v.name, value=v.name, vtype=atype} 
			assert(tmap[atype],"Attribute type not handled :"..atype)
			_vargs=_vargs..(",%s %s : %s"):format(tmap[atype],v.name,v.name)
		else 
			amap[k]={}
		end
	end
	if #_vargs>0 then _vargs=_vargs:sub(2) end
	
	local _ucode="cbuffer cbv : register(b0) {\n"
	local _texc=0
	for k,v in ipairs(uniforms) do 
		local atype=shType(v)
		if v.type==Shader.CTEXTURE then
			gmap[v.name]={type="cvar", value=v.name, vtype=atype}
			if v.subtype=="shadow" then
				_dcode=_dcode..("Texture2D _tex_%s : register(t%d);\nSamplerComparisonState _smp_%s : register(s%d);\n"):format(v.name,_texc,v.name,_texc)
			else
				_dcode=_dcode..("Texture2D _tex_%s : register(t%d);\nSamplerState _smp_%s : register(s%d);\n"):format(v.name,_texc,v.name,_texc)
			end
			_texc+=1
		elseif v.vertex then
			local mult,vmult="",nil
			if v.mult and v.mult>1 then mult="["..v.mult.."]" vmult=v.mult end
			_ucode=_ucode..("%s %s%s;\n"):format(tmap[atype],v.name,mult)
			gmap[v.name]={type="cvar", value=v.name, vtype=atype, vmult=mult}
		end
	end
	_ucode=_ucode.."};\n"

	local _pcode="struct PVertex {\n"
	_pcode=_pcode.."    float4 gl_Position : SV_POSITION;\n"
	_fargs=_fargs.."float4 gl_FragCoord : SV_POSITION"
	for k,v in ipairs(varying) do 
		local atype=shType(v)
		gmap[v.name]={type="var", value="outVert."..v.name, vtype=atype}
		local vdecl=("%s %s : %s"):format(tmap[atype],v.name,v.name)
		_pcode=_pcode..vdecl..";\n"
		_fargs=_fargs..","..vdecl
	end
	_pcode=_pcode.."};\n"

	_code=_code.._dcode.._ucode.._pcode..(([=[
PVertex VShader(%s)
{
    PVertex outVert;
]=]):format(_vargs))
	_code=_code..codegen(vf,amap,gmap,tmap,omap,	{
		RETURN=function(rval)
			if rval then return "outVert.gl_Position = "..rval.."; return outVert;\n"
			else return "return outVert;"
			end
		end,
		GENOP=_HLSL_GENOP,
		GENOPEQ=_HLSL_GENOPEQ,
		SWIZZLE=_HLSL_SWIZZLE,
	})
	_code=_code.."}"
	
	local _vshader=_code
	
	_ucode="cbuffer cbp : register(b1) {\n"
	for k,v in ipairs(uniforms) do 
		local atype=shType(v)
		if v.type==Shader.CTEXTURE then
			gmap[v.name]={type="cvar", value=v.name, vtype=atype}
		elseif not v.vertex then
			local mult,vmult="",nil
			if v.mult and v.mult>1 then mult="["..v.mult.."]" vmult=v.mult end
			_ucode=_ucode..("%s %s%s;\n"):format(tmap[atype],v.name,mult)
			gmap[v.name]={type="cvar", value=v.name, vtype=atype, vmult=mult}
		end
	end
	_ucode=_ucode.."};\n"
	
	for k,v in ipairs(varying) do 
		local atype=shType(v)
		gmap[v.name]={type="cvar", value=v.name, vtype=atype}
	end
--	local _code=""
	_code=""

	local mainCode=codegen(ff,amap,gmap,tmap,omap,	{
		RETURN=GEN_RETURN,
		GENOP=_HLSL_GENOP,
		GENOPEQ=_HLSL_GENOPEQ,
		SWIZZLE=_HLSL_SWIZZLE,
	})
	_code=_code.._dcode.._ucode.._pcode
	..genFunctions(gmap,tmap,omap,{	GENOP=_HLSL_GENOP,GENOPEQ=_HLSL_GENOPEQ,},funcs)
	..(([=[
half4 PShader(%s) : SV_TARGET
{
]=]):format(_fargs))
	_code=_code..mainCode.."}\n"
	
	local _fshader=_code
	
	return _vshader,_fshader	
end
