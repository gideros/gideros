--!NEEDS:luac_codegen.lua
local function shType(v)
	local p
	if v.type==Shader.CFLOAT then
		p="hF1"
	elseif v.type==Shader.CFLOAT2 then
		p="hF2"
	elseif v.type==Shader.CFLOAT3 then
		p="hF3"
	elseif v.type==Shader.CFLOAT4 then
		p="hF4"
	elseif v.type==Shader.CINT then
		p="hI1"
	elseif v.type==Shader.CTEXTURE then
		p="lT1"
	elseif v.type==Shader.CMATRIX then
		p="hF44"
	end
	assert(p,"Type not mapped:"..v.type.." for "..v.name)
	return p
end

local function shAType(v)
	local p
	if v.type==Shader.DFLOAT then
		p="hF"
	elseif v.type==Shader.DUBYTE then
		p="lF"
	end
	assert(p,"Type not mapped:"..v.type.." for "..v.name)
	p=p..(v.mult or 1)
	return p
end

function Shader.lua(vf,ff,opt,uniforms,attrs,varying,debug)
	local lang=Shader.getShaderLanguage()
	local mtd=Shader["lua_"..lang]
	assert(mtd,"Language not supported: "..lang)
	
	local _vshader,_fshader=mtd(vf,ff,opt,uniforms,attrs,varying)

	if debug then
		print("VSHADER_CODE:\n".._vshader)	
		print("FSHADER_CODE:\n".._fshader)
	end
	
	return Shader.new(_vshader,_fshader,Shader.FLAG_FROM_CODE|opt,uniforms,attrs)	
end


local function _GLSL_GENOP(o,a,b)
	if o=="%" then
		return ("mod(%s,%s)"):format(a.value,b.value)
	end
end

local function _GLSL_GENOPEQ(o,a,b)
	if o=="%" then
		return ("%s=mod(%s,%s)"):format(a.value,a.value,b.value)
	end
end

function Shader.lua_glsl(vf,ff,opt,uniforms,attrs,varying)
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
		hF44="highp mat4",
		BOOL="bool",
	}
	local omap={
		["hF44*hF4"]="hF4",
		["hF4*hF44"]="hF4",
	}
		
	gmap["hF1"]={type="func", value="float", rtype="hF1"}
	gmap["hF2"]={type="func", value="vec2", rtype="hF2"}
	gmap["hF3"]={type="func", value="vec3", rtype="hF3"}
	gmap["hF4"]={type="func", value="vec4", rtype="hF4"}
	gmap["lF1"]={type="func", value="float", rtype="lF1"}
	gmap["lF2"]={type="func", value="vec2", rtype="lF2"}
	gmap["lF3"]={type="func", value="vec3", rtype="lF3"}
	gmap["lF4"]={type="func", value="vec4", rtype="lF4"}
	gmap["hF44"]={type="func", value="mat4", rtype="hF44"}
	
	gmap["discard"]={type="func", value="discard", evaluate=function (args) return "discard" end}
	gmap["texture2D"]={type="func", value="texture2D", rtype="lF4"}	
	gmap["sin"]={type="func", value="sin", rtype="1"}
	gmap["cos"]={type="func", value="cos", rtype="1"}
	gmap["floor"]={type="func", value="floor", rtype="1"}
	gmap["dot"]={type="func", value="dot", rtype="hF1"}
	gmap["mix"]={type="func", value="mix", rtype="1"}
	gmap["tan"]={type="func", value="tan", rtype="1"}
	gmap["asin"]={type="func", value="asin", rtype="1"}
	gmap["acos"]={type="func", value="acos", rtype="1"}
	gmap["atan"]={type="func", value="atan", rtype="1"}
	gmap["abs"]={type="func", value="abs", rtype="1"}
	gmap["sign"]={type="func", value="sign", rtype="1"}
	gmap["ceil"]={type="func", value="ceil", rtype="1"}
	gmap["fract"]={type="func", value="fract", rtype="1"}
	gmap["clamp"]={type="func", value="clamp", rtype="1"}
	gmap["step"]={type="func", value="step", rtype="2"}
	gmap["smoothstep"]={type="func", value="smoothstep", rtype="2"}	
	gmap["length"]={type="func", value="length", rtype="hF1"}
	gmap["distance"]={type="func", value="distance", rtype="hF1"}
	gmap["cross"]={type="func", value="cross", rtype="1"}
	gmap["normalize"]={type="func", value="normalize", rtype="1"}


	local _code=""
	for k,v in ipairs(attrs) do 
		local atype=shAType(v)
		amap[k]={type="cvar", value=v.name, vtype=atype} 
		assert(tmap[atype],"Attribute type not handled :"..atype)
		_code=_code..("attribute %s %s;\n"):format(tmap[atype],v.name)
	end
	for k,v in ipairs(uniforms) do 
		if v.vertex then
			local atype=shType(v)
			gmap[v.name]={type="cvar", value=v.name, vtype=atype}
			_code=_code..("uniform %s %s;\n"):format(tmap[atype],v.name)
		end
	end
	for k,v in ipairs(varying) do 
		local atype=shType(v)
		gmap[v.name]={type="var", value=v.name, vtype=atype}
		_code=_code..("varying %s %s;\n"):format(tmap[atype],v.name)
	end
	_code=_code.."\n void main() {\n"
	_code=_code..codegen(vf,amap,gmap,tmap,omap,	{
		RETURN=function(rval)
			if rval then
				return "gl_Position = "..rval..";\n"
			else
				return ""
			end
		end,
		GENOP=_GLSL_GENOP,
		GENOPEQ=_GLSL_GENOPEQ,
	})
	_code=_code.."}"
	
	local _vshader=_code
	
	
	_code=""
	for k,v in ipairs(uniforms) do 
		if not v.vertex then
			local atype=shType(v)
			gmap[v.name]={type="cvar", value=v.name, vtype=atype}
			_code=_code..("uniform %s %s;\n"):format(tmap[atype],v.name)
		end
	end
	for k,v in ipairs(varying) do 
		local atype=shType(v)
		gmap[v.name]={type="cvar", value=v.name, vtype=atype}
		_code=_code..("varying %s %s;\n"):format(tmap[atype],v.name)
	end
	_code=_code.."\n void main() {\n"
	_code=_code..codegen(ff,amap,gmap,tmap,omap,	{
		RETURN=function(rval)
			if rval then
				return "gl_FragColor = "..rval..";\n"
			else
				return ""
			end
		end,
		GENOP=_GLSL_GENOP,
		GENOPEQ=_GLSL_GENOPEQ,
	})
	_code=_code.."}"
	
	local _fshader=_code
	
	return _vshader,_fshader	
end


local function _MSL_GENOP(o,a,b)
	if o=="%" then
		return ("fmod(%s,%s)"):format(a.value,b.value)
	end
end

local function _MSL_GENOPEQ(o,a,b)
	if o=="%" then
		return ("%s=fmod(%s,%s)"):format(a.value,a.value,b.value)
	end
end

function Shader.lua_msl(vf,ff,opt,uniforms,attrs,varying)
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
		hF44="float4x4",
		BOOL="bool",
	}
	local omap={
		["hF44*hF4"]="hF4",
		["hF4*hF44"]="hF4",
	}
		
	gmap["hF1"]={type="func", value="float", rtype="hF1"}
	gmap["hF2"]={type="func", value="float2", rtype="hF2"}
	gmap["hF3"]={type="func", value="float3", rtype="hF3"}
	gmap["hF4"]={type="func", value="float4", rtype="hF4"}
	gmap["lF1"]={type="func", value="half", rtype="lF1"}
	gmap["lF2"]={type="func", value="half2", rtype="lF2"}
	gmap["lF3"]={type="func", value="half3", rtype="lF3"}
	gmap["lF4"]={type="func", value="half4", rtype="lF4"}
	gmap["hF44"]={type="func", value="float4x4", rtype="hF44"}
	
	gmap["discard"]={type="func", value="discard_fragment"}
	gmap["texture2D"]={type="func", value="texture2D", evaluate=function (tex,sp)
		return ("_tex_%s.sample(_smp_%s, %s)"):format(tex,tex,sp)
	end, rtype="lF4"}
	gmap["sin"]={type="func", value="sin", rtype="1"}
	gmap["cos"]={type="func", value="cos", rtype="1"}
	gmap["floor"]={type="func", value="floor", rtype="1"}
	gmap["dot"]={type="func", value="dot", rtype="hF1"}
	gmap["mix"]={type="func", value="mix", rtype="1"}

	gmap["tan"]={type="func", value="tan", rtype="1"}
	gmap["asin"]={type="func", value="asin", rtype="1"}
	gmap["acos"]={type="func", value="acos", rtype="1"}
	gmap["atan"]={type="func", value="atan", rtype="1"}
	gmap["abs"]={type="func", value="abs", rtype="1"}
	gmap["sign"]={type="func", value="sign", rtype="1"}
	gmap["ceil"]={type="func", value="ceil", rtype="1"}
	gmap["fract"]={type="func", value="fract", rtype="1"}
	gmap["clamp"]={type="func", value="clamp", rtype="1"}
	gmap["step"]={type="func", value="step", rtype="2"}
	gmap["smoothstep"]={type="func", value="smoothstep", rtype="2"}	
	gmap["length"]={type="func", value="length", rtype="hF1"}
	gmap["distance"]={type="func", value="distance", rtype="hF1"}
	gmap["cross"]={type="func", value="cross", rtype="1"}
	gmap["normalize"]={type="func", value="normalize", rtype="1"}

	local _code=[=[
#include <metal_stdlib>
using namespace metal;
]=]

	_code=_code.."struct InVertex {\n"
	for k,v in ipairs(attrs) do 
		local atype=shAType(v)
		amap[k]={type="cvar", value="inVertex."..v.name, vtype=atype} 
		assert(tmap[atype],"Attribute type not handled :"..atype)
		_code=_code..("%s %s [[attribute(%d)]];\n"):format(tmap[atype],v.name,v.slot)
	end
	_code=_code.."};\n"
	
	local _fargs=""
	local _ucode="struct Uniforms {\n"
	local _texc=0
	for k,v in ipairs(uniforms) do 
		local atype=shType(v)
		if v.type==Shader.CTEXTURE then
			gmap[v.name]={type="cvar", value=v.name, vtype=atype}
			_fargs=_fargs..(",texture2d<half> _tex_%s [[texture(%d)]], sampler _smp_%s [[sampler(%d)]]"):format(v.name,_texc,v.name,_texc)
			_texc+=1
		else
			_ucode=_ucode..("%s %s;\n"):format(tmap[atype],v.name)
			gmap[v.name]={type="cvar", value="uniforms."..v.name, vtype=atype}
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
           constant Uniforms &uniforms [[buffer(0)]]%s)
{
    PVertex outVert;
]=]):format(_fargs))
	_code=_code..codegen(vf,amap,gmap,tmap,omap,	{
		RETURN=function(rval)
			if rval then
				return "outVert.gl_Position = "..rval.."; return outVert;\n"
			else
				return "return outVert;"
			end
		end,
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
	_code=_code.._ucode.._pcode..(([=[
fragment half4 fmain(PVertex vert [[stage_in]],
                    constant Uniforms &uniforms [[buffer(0)]]%s)
{
]=]):format(_fargs))
	_code=_code..codegen(ff,amap,gmap,tmap,omap,	{
		RETURN=function(rval)
			if rval then
				return "return "..rval..";\n"
			else
				return ""
			end
		end,
		GENOP=_MSL_GENOP,
		GENOPEQ=_MSL_GENOPEQ,
	})
	_code=_code.."}"
	
	local _fshader=_code
	
	return _vshader,_fshader	
end


local function ismatrix(v)
	return (tonumber(v.vtype:sub(3))>=22)
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

function Shader.lua_hlsl(vf,ff,opt,uniforms,attrs,varying)
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
		hF44="float4x4",
		BOOL="bool",
	}
	local omap={
		["hF44*hF4"]="hF4",
		["hF4*hF44"]="hF4",
	}
		
	gmap["hF1"]={type="func", value="float", rtype="hF1"}
	gmap["hF2"]={type="func", value="float2", rtype="hF2"}
	gmap["hF3"]={type="func", value="float3", rtype="hF3"}
	gmap["hF4"]={type="func", value="float4", rtype="hF4"}
	gmap["lF1"]={type="func", value="half", rtype="lF1"}
	gmap["lF2"]={type="func", value="half2", rtype="lF2"}
	gmap["lF3"]={type="func", value="half3", rtype="lF3"}
	gmap["lF4"]={type="func", value="half4", rtype="lF4"}
	gmap["hF44"]={type="func", value="float4x4", rtype="hF44"}
	
	gmap["discard"]={type="func", value="discard", evaluate=function (args) return "discard" end}
	gmap["texture2D"]={type="func", value="texture2D", evaluate=function (tex,sp)
		return ("_tex_%s.Sample(_smp_%s, %s)"):format(tex,tex,sp)
	end, rtype="lF4"}
	gmap["sin"]={type="func", value="sin", rtype="1"}
	gmap["cos"]={type="func", value="cos", rtype="1"}
	gmap["floor"]={type="func", value="floor", rtype="1"}
	gmap["dot"]={type="func", value="dot", rtype="hF1"}
	gmap["mix"]={type="func", value="lerp", rtype="1"}

	gmap["tan"]={type="func", value="tan", rtype="1"}
	gmap["asin"]={type="func", value="asin", rtype="1"}
	gmap["acos"]={type="func", value="acos", rtype="1"}
	gmap["atan"]={type="func", value="atan", rtype="1"}
	gmap["abs"]={type="func", value="abs", rtype="1"}
	gmap["sign"]={type="func", value="sign", rtype="1"}
	gmap["ceil"]={type="func", value="ceil", rtype="1"}
	gmap["fract"]={type="func", value="frac", rtype="1"}
	gmap["clamp"]={type="func", value="clamp", rtype="1"}
	gmap["step"]={type="func", value="step", rtype="2"}
	gmap["smoothstep"]={type="func", value="smoothstep", rtype="2"}	
	gmap["length"]={type="func", value="length", rtype="hF1"}
	gmap["distance"]={type="func", value="distance", rtype="hF1"}
	gmap["cross"]={type="func", value="cross", rtype="1"}
	gmap["normalize"]={type="func", value="normalize", rtype="1"}

	local _code,_dcode="",""

	local _vargs,_fargs="",""
	for k,v in ipairs(attrs) do 
		local atype=shAType(v)
		amap[k]={type="cvar", value=v.name, vtype=atype} 
		assert(tmap[atype],"Attribute type not handled :"..atype)
		_vargs=_vargs..(",%s %s : %s"):format(tmap[atype],v.name,v.name)
	end
	if #_vargs>0 then _vargs=_vargs:sub(2) end
	
	local _ucode="cbuffer cbv : register(b0) {\n"
	local _texc=0
	for k,v in ipairs(uniforms) do 
		local atype=shType(v)
		if v.type==Shader.CTEXTURE then
			gmap[v.name]={type="cvar", value=v.name, vtype=atype}
			_dcode=_dcode..("Texture2D _tex_%s : register(t%d);\nSamplerState _smp_%s : register(s%d);\n"):format(v.name,_texc,v.name,_texc)
			_texc+=1
		elseif v.vertex then
			_ucode=_ucode..("%s %s;\n"):format(tmap[atype],v.name)
			gmap[v.name]={type="cvar", value=v.name, vtype=atype}
		end
	end
	_ucode=_ucode.."};\n"

	local _pcode="struct PVertex {\n"
	_pcode=_pcode.."    float4 gl_Position : SV_POSITION;\n"
	_fargs=_fargs.."float4 gl_Position : SV_POSITION"
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
			if rval then
				return "outVert.gl_Position = "..rval.."; return outVert;\n"
			else
				return "return outVert;"
			end
		end,
		GENOP=_HLSL_GENOP,
		GENOPEQ=_HLSL_GENOPEQ,
	})
	_code=_code.."}"
	
	local _vshader=_code
	
	local _ucode="cbuffer cbp : register(b1) {\n"
	for k,v in ipairs(uniforms) do 
		local atype=shType(v)
		if v.type==Shader.CTEXTURE then
			gmap[v.name]={type="cvar", value=v.name, vtype=atype}
		elseif not v.vertex then
			_ucode=_ucode..("%s %s;\n"):format(tmap[atype],v.name)
			gmap[v.name]={type="cvar", value=v.name, vtype=atype}
		end
	end
	_ucode=_ucode.."};\n"
	
	for k,v in ipairs(varying) do 
		local atype=shType(v)
		gmap[v.name]={type="cvar", value=v.name, vtype=atype}
	end
	local _code=""
	_code=_code.._dcode.._ucode.._pcode..(([=[
half4 PShader(%s) : SV_TARGET
{
]=]):format(_fargs))
	_code=_code..codegen(ff,amap,gmap,tmap,omap,	{
		RETURN=function(rval)
			if rval then
				return "return "..rval..";\n"
			else
				return ""
			end
		end,
		GENOP=_HLSL_GENOP,
		GENOPEQ=_HLSL_GENOPEQ,
	})
	_code=_code.."}"
	
	local _fshader=_code
	
	return _vshader,_fshader	
end
