Lighting={}
local glversion=Shader.getEngineVersion()
local isES3Level=(glversion~="GLES2")

print(glversion)
print(Shader.getProperties().version)
print(json.encode(Shader.getProperties().extensions))

local LightingShaderAttrs=
{
{name="POSITION0",type=Shader.DFLOAT,mult=3,slot=0,offset=0},
{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
{name="TEXCOORD0",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
{name="NORMAL0",type=Shader.DFLOAT,mult=3,slot=3,offset=0},
{name="ANIMIDX",type=Shader.DFLOAT,mult=4,slot=4,offset=0},
{name="ANIMWEIGHT",type=Shader.DFLOAT,mult=4,slot=5,offset=0},
{name="INSTMAT1",type=Shader.DFLOAT,mult=4,slot=6,offset=0,instances=1},
{name="INSTMAT2",type=Shader.DFLOAT,mult=4,slot=7,offset=0,instances=1},
{name="INSTMAT3",type=Shader.DFLOAT,mult=4,slot=8,offset=0,instances=1},
{name="INSTMAT4",type=Shader.DFLOAT,mult=4,slot=9,offset=0,instances=1},
}

local LightingShaderConstants={
{name="g_MVPMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP, vertex=true},
{name="g_MVMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WORLD, vertex=true},
{name="g_NMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WIT3, vertex=true},
{name="g_LMatrix",type=Shader.CMATRIX, vertex=true},
{name="g_Color",type=Shader.CFLOAT4,mult=1,sys=Shader.SYS_COLOR},
{name="lightPos",type=Shader.CFLOAT4,mult=1,vertex=false},
{name="cameraPos",type=Shader.CFLOAT4,mult=1,vertex=false},
{name="ambient",type=Shader.CFLOAT,mult=1,vertex=false}}
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="g_Texture",type=Shader.CTEXTURE,mult=1,vertex=false}
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="g_NormalMap",type=Shader.CTEXTURE,mult=1,vertex=false}
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="g_ShadowMap",type=Shader.CTEXTURE,mult=1,vertex=false}
--[[	
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="g_InstanceMap",type=Shader.CTEXTURE,mult=1,vertex=true}
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="InstanceMapWidth",type=Shader.CFLOAT,mult=1,vertex=true}
	]]
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="bones",type=Shader.CMATRIX,mult=16,vertex=true}
LightingShaderConstants[#LightingShaderConstants+1]=
	{name="InstanceMatrix",type=Shader.CMATRIX,mult=1,vertex=true}

-- Shaders defs
Lighting._shaders={}
Lighting.getShader=function(code)
	local cmap={
		{"t","TEXTURED",true},
		{"s","SHADOWS",isES3Level},
		{"n","NORMMAP",true},
		{"i","INSTANCED",true},
		{"a","ANIMATED",true},
	}	
	local lcode,ccode="",""
	for _,k in ipairs(cmap) do
		if code:find(k[1]) then
			lcode=lcode..k[1]
			if k[3] then
				ccode=ccode.."#define "..k[2].."\n"
			end
		end
	end
	if lcode=="" then return nil,nil end
	if D3._V_Shader then
		if not Lighting._shaders[lcode] then
			v=Shader.new(
				ccode..D3._V_Shader,
				ccode..D3._F_Shader,
				Shader.FLAG_FROM_CODE,
				LightingShaderConstants,LightingShaderAttrs)
			v:setConstant("lightPos",Shader.CFLOAT4,1,Lighting.light[1],Lighting.light[2],Lighting.light[3],1)
			v:setConstant("ambient",Shader.CFLOAT,1,Lighting.light[4])
			v:setConstant("cameraPos",Shader.CFLOAT4,1,Lighting.camera[1],Lighting.camera[2],Lighting.camera[3],1)
			Lighting._shaders[lcode]=v
		end
	end
	return Lighting._shaders[lcode],lcode
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
			sprite:setTexture(Lighting.getShadowMap(),2)
			Lighting._shadowed[sprite]=sprite
		end
		if sc:find("a") then
			D3Anim._addMesh(sprite)
		end
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
	view:lookAt(Lighting.light[1],Lighting.light[2],Lighting.light[3],Lighting.lightTarget[1],Lighting.lightTarget[2],Lighting.lightTarget[3],0,1,0)
	p:multiply(view:getTransform())
	for k,v in pairs(Lighting._shaders) do
		v:setConstant("g_LMatrix",Shader.CMATRIX,1,p:getMatrix())
	end
	srt:clear(0,0)
	srt:draw(view)
	for _,v in pairs(Lighting._shadowed) do
		v:setTexture(srt,2)
	end
	--stage:addChild(Bitmap.new(srt))
	--stage:addChild(Lighting.shadowview)
end

Lighting.setCamera(0,0,0)
Lighting.setLight(0,10,0,0.3)
Lighting.setLightTarget(0,0,0) --X,Y,Z, DIST, FOV
