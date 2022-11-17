--!NEEDS:../luashader/luashader.lua

local debug = nil
if debug then print("3DLighting.lua debug !!!!!!!!!!!!!!!!!!!!!!") end

Lighting={}
local glversion=Shader.getEngineVersion()
local isES3Level=(glversion~="GLES2")
local isES3=(glversion=="GLES3")
local slang=Shader.getShaderLanguage()

if debug then print(glversion) end
if debug then print(Shader.getProperties().version) end

Shader.extensions={}
if slang=="glsl" then
	for ex in Shader.getProperties().extensions:gmatch("%S+") do Shader.extensions[ex]=true end
	if debug then print(json.encode(Shader.extensions)) end
end

local LightingShaderAttrs={
{name="POSITION",type=Shader.DFLOAT,amult=3,slot=0,offset=0},
{name="COLOR",type=Shader.DUBYTE,amult=4,slot=1,offset=0,code="c"}, --Placeholder: mult=0
{name="TEXCOORD",type=Shader.DFLOAT,amult=2,slot=2,offset=0,code="t"},
{name="NORMAL",type=Shader.DFLOAT,amult=3,slot=3,offset=0},
{name="ANIMIDX",type=Shader.DFLOAT,amult=4,slot=4,offset=0,code="a"},
{name="ANIMWEIGHT",type=Shader.DFLOAT,amult=4,slot=5,offset=0,code="a"},
{name="INSTMATA",type=Shader.DFLOAT,amult=4,slot=6,offset=0,instances=1,code="i"},
{name="INSTMATB",type=Shader.DFLOAT,amult=4,slot=7,offset=0,instances=1,code="i"},
{name="INSTMATC",type=Shader.DFLOAT,amult=4,slot=8,offset=0,instances=1,code="i"},
{name="INSTMATD",type=Shader.DFLOAT,amult=4,slot=9,offset=0,instances=1,code="i"}
}

local LightingShaderConstants={
{name="g_MVPMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP, vertex=true},
{name="g_MVMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WORLD, vertex=true},
{name="g_NMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WIT3, vertex=true},
{name="g_LMatrix",type=Shader.CMATRIX, vertex=true},
{name="g_Color",type=Shader.CFLOAT4,mult=1,sys=Shader.SYS_COLOR},
{name="lightPos",type=Shader.CFLOAT4,mult=1,vertex=false},
{name="cameraPos",type=Shader.CFLOAT4,mult=1,vertex=false},
{name="ambient",type=Shader.CFLOAT,mult=1,vertex=false},
{name="g_Texture",type=Shader.CTEXTURE,mult=1,vertex=false},
{name="g_NormalMap",type=Shader.CTEXTURE,mult=1,vertex=false,code="n"},
{name="g_ShadowMap",type=Shader.CTEXTURE,subtype="shadow",mult=1,vertex=false,code="s"},
{name="bones",type=Shader.CMATRIX,mult=64,vertex=true,code="a"},
{name="InstanceMatrix",type=Shader.CMATRIX,mult=1,vertex=true,code="i"},
}
--[[
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="g_InstanceMap",type=Shader.CTEXTURE,mult=1,vertex=true}
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="InstanceMapWidth",type=Shader.CFLOAT,mult=1,vertex=true}
]]

local LightingShaderVarying={
{name="position",type=Shader.CFLOAT3},
{name="texCoord",type=Shader.CFLOAT2,code="t"},
{name="normalCoord",type=Shader.CFLOAT3},
{name="lightSpace",type=Shader.CFLOAT4,code="s"},
{name="vcolor",type=Shader.CFLOAT4,code="c"},
}

-- Shaders defs
local function ShaderFilter(t,code)
	local o={}
	for _,l in ipairs(t) do
		if l.code==nil or code:find(l.code) then o[#o+1]=l end
	end
	return o
end

Lighting._shaders={}
Lighting.getShader=function(code)
	local cmap={
		{"t","TEXTURED",true},
		{"c","COLORED",true},
		{"s","SHADOWS",isES3Level and ((slang~="glsl") or
--			isES3 or
			Shader.extensions.GL_EXT_shadow_samplers or
			Shader.extensions.GL_EXT_shadow_funcs)},
		{"n","NORMMAP",true},
		{"i","INSTANCED",true},
		{"a","ANIMATED",true},
	}
	local lcode,ccode,acode="","",""
	local lconst={}
	for _,k in ipairs(cmap) do
		local active="false"
		if code:find(k[1]) then
			lcode=lcode..k[1]
			if k[3] then
				acode=acode..k[1]
				ccode=ccode.."#define "..k[2].."\n"
				active="true"
			end
		end
		table.insert(lconst,{ name="OPT_"..k[2], type="BOOL", value=active})
	end
	--if lcode=="" then return nil,nil end
	if D3._V_Shader then
		if not Lighting._shaders[lcode] then
			for _,a in ipairs(LightingShaderAttrs) do
				if not a.code or code:find(a.code) then a.mult=a.amult else a.mult=0 end
			end
			--[[
			v=Shader.new(
				ccode..D3._V_Shader,
				ccode..D3._F_Shader,
				Shader.FLAG_FROM_CODE,
				LightingShaderConstants,LightingShaderAttrs)
			]]
			local csts=ShaderFilter(LightingShaderConstants,acode)
			local v=Shader.lua(
				D3._VLUA_Shader,
				D3._FLUA_Shader,
				0,
				csts,
				LightingShaderAttrs,
				ShaderFilter(LightingShaderVarying,acode),
				D3._FLUA_Shader_FDEF,
				lconst
			)

			v:setConstant("lightPos",Shader.CFLOAT4,1,Lighting.light[1],Lighting.light[2],Lighting.light[3],1)
			v:setConstant("ambient",Shader.CFLOAT,1,Lighting.light[4])
			v:setConstant("cameraPos",Shader.CFLOAT4,1,Lighting.camera[1],Lighting.camera[2],Lighting.camera[3],1)
			v.acode=acode
			Lighting._shaders[lcode]=v

			local tidx=0
			v.textureIndex={}
			for _,tt in ipairs(csts) do
				if tt.type==Shader.CTEXTURE then
					v.textureIndex[tt.name]=tidx
					tidx+=1
				end
			end
		end
	end
	return Lighting._shaders[lcode],lcode
end

Lighting.prepareShader=function(v)
	if D3._V_Shader then
		if not Lighting._shaders[v] then
			v:setConstant("lightPos",Shader.CFLOAT4,1,Lighting.light[1],Lighting.light[2],Lighting.light[3],1)
			v:setConstant("ambient",Shader.CFLOAT,1,Lighting.light[4])
			v:setConstant("cameraPos",Shader.CFLOAT4,1,Lighting.camera[1],Lighting.camera[2],Lighting.camera[3],1)
			Lighting._shaders[v]=v
		end
	end
	return Lighting._shaders[v],v
end

function Lighting.setLight(x,y,z,a)
	Lighting.light={x,y,z,a}
	for k,v in pairs(Lighting._shaders) do
		v:setConstant("lightPos",Shader.CFLOAT4,1,x,y,z,1)
		v:setConstant("ambient",Shader.CFLOAT,1,a)
	end
end

function Lighting.setLightTarget(x,y,z,d,f)
	Lighting.lightTarget={x,y,z,d or 50,f or 120}
end

function Lighting.setCamera(x,y,z)
	Lighting.camera={x,y,z}
	for k,v in pairs(Lighting._shaders) do
		v:setConstant("cameraPos",Shader.CFLOAT4,1,x,y,z,1)
	end
end

Lighting._sprites={}
Lighting._shadowed={}
function Lighting.setSpriteMode(sprite,mode)
	local sh,sc=Lighting.getShader(mode)
	sprite:setShader(sh)
	sprite.shader=sh
	local lsc=sprite._lighting_Mode
	if lsc then
		Lighting._sprites[lsc]=Lighting._sprites[lsc] or {}
		Lighting._sprites[lsc][sprite]=nil
		Lighting._shadowed[sprite]=nil
		D3Anim._animated[sprite]=nil
	end
	sprite._lighting_Mode=sc
	if sc then
		Lighting._sprites[sc]=Lighting._sprites[sc] or {}
		Lighting._sprites[sc][sprite]=sprite
		if sc:find("s") then
			if sh.textureIndex.g_ShadowMap then
				sprite:setTexture(Lighting.getShadowMap(),sh.textureIndex.g_ShadowMap)
			end
			Lighting._shadowed[sprite]=sprite
		end
		if sc:find("a") then D3Anim._addMesh(sprite) end
	end
end

Lighting.shadowrt=nil
function Lighting.getShadowMap(swap)
	if not Lighting.shadowrt then
		local ssz=1024
		local rt1=RenderTarget.new(ssz,ssz,true,false,false,true)
		local rt2=RenderTarget.new(ssz,ssz,true,false,false,true)
		local view=Viewport.new()
		view:setScale(ssz/2,ssz/2)
		view:setPosition(ssz/2,ssz/2)
		Lighting.shadowview=view
		Lighting.shadowrt=rt1
		Lighting.shadowrt1=rt1
		Lighting.shadowrt2=rt2
	end
	if swap then
		if Lighting.shadowrt==Lighting.shadowrt1 then
			Lighting.shadowrt=Lighting.shadowrt2
		else
			Lighting.shadowrt=Lighting.shadowrt1
		end
	end
	return Lighting.shadowrt
end

function Lighting.computeShadows(scene)
	local p=Lighting.lightProj or Matrix.new()
	Lighting.lightProj=p
	local lz=Lighting.lightTarget[4]
	p:perspectiveProjection(Lighting.lightTarget[5],1,lz/16,lz)
	local srt=Lighting.getShadowMap(true)
	local view=Lighting.shadowview
	view:setContent(scene)
	view:setProjection(p)
	view:lookAt(
		Lighting.light[1],Lighting.light[2],Lighting.light[3],
		Lighting.lightTarget[1],Lighting.lightTarget[2],Lighting.lightTarget[3],
		0,1,0)
	p:multiply(view:getTransform())
	for k,v in pairs(Lighting._shaders) do
		v:setConstant("g_LMatrix",Shader.CMATRIX,1,p:getMatrix())
	end
	srt:clear(0,0)
	srt:draw(view)
	for _,v in pairs(Lighting._shadowed) do
		if v.shader.textureIndex.g_ShadowMap then
			v:setTexture(srt,v.shader.textureIndex.g_ShadowMap)
		end
	end
	--stage:addChild(Bitmap.new(srt))
	--stage:addChild(Lighting.shadowview)
end

Lighting.setCamera(0,0,0)
Lighting.setLight(0,10,0,0.3)
Lighting.setLightTarget(0,0,0) --X,Y,Z, DIST, FOV
