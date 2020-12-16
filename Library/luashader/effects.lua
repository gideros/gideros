--!NEEDS:luashader.lua

local shaderCache={}

local function genEffect(vshader,fshader)
	local s=Shader.lua(vshader,fshader,0,
	{
	{name="vMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP,vertex=true},
	{name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=false},
	{name="fTexture",type=Shader.CTEXTURE,vertex=false},
	{name="fTextureInfo",type=Shader.CFLOAT4,sys=Shader.SYS_TEXTUREINFO,vertex=false},
	{name="fAmount",type=Shader.CFLOAT2,vertex=false},
	{name="fDirection",type=Shader.CFLOAT2,vertex=false},
	{name="fTexture2",type=Shader.CTEXTURE,vertex=false},
	},
	{
	{name="vVertex",type=Shader.DFLOAT,mult=2,slot=0,offset=0},
	{name="vColor",type=Shader.DUBYTE,mult=4,slot=1,offset=0},
	{name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
	},
	{
	{name="fTexCoord",type=Shader.CFLOAT2},
	}
	)
	return s
end

local function makeEffect(name,vshader,fshader)
	shaderCache[name]={ 
		vshader=vshader, 
		fshader=fshader
		}
end

local function getEffect(name)
	assert(shaderCache[name],"No such effect:"..name)
	if not shaderCache[name].shader then 
		shaderCache[name].shader=genEffect(shaderCache[name].vshader,shaderCache[name].fshader)
	end
	return shaderCache[name].shader 
end

local function vertexShader(vVertex,vColor,vTexCoord)
	local vertex = hF4(vVertex,0.0,1.0)
	fTexCoord=vTexCoord
	return vMatrix*vertex
end


makeEffect("Blur",
	vertexShader,
	function ()
	 local frag=lF4(0,0,0,0)
	 local frad=fAmount.x
	 local ext=2*frad+1
	 local dir=fTextureInfo.zw*fDirection
	 local tc=fTexCoord-dir*frad
	 for v=0,19 do
		if v<ext then
			frag=frag+texture2D(fTexture, tc)
		end
		tc+=dir
	 end
	 frag=(frag/ext)*lF4(fColor)
	 if (frag.a==0.0) then discard() end
	 return frag
	end)

makeEffect("BloomExtract",
	vertexShader,
	function ()
	 local c=texture2D(fTexture, fTexCoord)
	 local BloomThreshold=fAmount.x
	 c=clamp((c-BloomThreshold)/(1.0-BloomThreshold), 0.0, 1.0)
	 c.a=1
	 return c
	end)

makeEffect("BloomCombine",
	vertexShader,
	function ()
	 local base=texture2D(fTexture, fTexCoord)*fAmount.x
	 local bloom=texture2D(fTexture2, fTexCoord)*fAmount.y
	 base *= (hF4(1.0) - clamp(bloom,0.0,1.0))
	 base+=bloom
	 return base
	end)


ShaderEffect=Core.class(Object)
function ShaderEffect:init()
	self.applied={}
    local weak = { __mode="k" }
    setmetatable(self.applied, weak)
end

function ShaderEffect:apply(sprite)
	sprite:setEffectStack(self.stack)
	self.applied[sprite]=true
	self:applyParametersTo(sprite)
end

function ShaderEffect:remove(sprite)
	sprite:setEffectStack()
	self.applied[sprite]=nil
end

function ShaderEffect:applyParameters()
	for k,_ in pairs(self.applied) do
		self:applyParametersTo(k)
	end
end

function ShaderEffect:applyParametersTo(sprite)
end


local BloomEffect=Core.class(ShaderEffect)
ShaderEffect.Bloom=BloomEffect

function BloomEffect:init(width,height,resolution)
	width*=(resolution or 1)
	height*=(resolution or 1)
	local xform,pxform
	if resolution then
		xform=Matrix.new()
		xform:setScale(resolution)
		pxform=Matrix.new()
		pxform:setScale(1/resolution)
	end
	
	local rt1=RenderTarget.new(width,height)
	local rt2=RenderTarget.new(width,height)
	local rt3=RenderTarget.new(width,height)

	self.stack={
		{ buffer=rt3, shader=getEffect("BloomExtract"), transform=xform},
		{ buffer=rt1, shader=getEffect("Blur")},
		{ buffer=rt2, shader=getEffect("Blur")},
		{ buffer=rt1, shader=getEffect("BloomCombine"),textures={rt3,rt1}, postTransform=pxform},
	}
	self.radius=7
	self.glow=2
end

function BloomEffect:setRadiusAndGlow(radius,glow)
	self.radius=radius
	self.glow=glow
	self:applyParameters()
end

function BloomEffect:applyParametersTo(sprite)
	sprite:setEffectConstant(2,"fDirection",Shader.CFLOAT2,1,1,0)
	sprite:setEffectConstant(3,"fDirection",Shader.CFLOAT2,1,0,1)
	sprite:setEffectConstant(1,"fAmount",Shader.CFLOAT2,1,1/self.glow,0)
	sprite:setEffectConstant(2,"fAmount",Shader.CFLOAT2,1,self.radius,0)
	sprite:setEffectConstant(3,"fAmount",Shader.CFLOAT2,1,self.radius,0)
	sprite:setEffectConstant(4,"fAmount",Shader.CFLOAT2,1,2,self.glow)
end

local BlurEffect=Core.class(ShaderEffect)
ShaderEffect.Blur=BlurEffect

function BlurEffect:init(width,height,resolution)
	width*=(resolution or 1)
	height*=(resolution or 1)
	local xform,pxform
	if resolution then
		xform=Matrix.new()
		xform:setScale(resolution)
		pxform=Matrix.new()
		pxform:setScale(1/resolution)
	end
	local rt1=RenderTarget.new(width,height)
	local rt2=RenderTarget.new(width,height)

	self.stack={
		{ buffer=rt1, shader=getEffect("Blur"), transform=xform},
		{ buffer=rt2, shader=getEffect("Blur"), postTransform=pxform},
	}
	
	self.radius=7
end

function BlurEffect:setRadius(radius)
	self.radius=radius
	self:applyParameters()
end

function BlurEffect:applyParametersTo(sprite)
	sprite:setEffectConstant(1,"fDirection",Shader.CFLOAT2,1,1,0)
	sprite:setEffectConstant(2,"fDirection",Shader.CFLOAT2,1,0,1)
	sprite:setEffectConstant(1,"fAmount",Shader.CFLOAT2,1,self.radius,0)
	sprite:setEffectConstant(2,"fAmount",Shader.CFLOAT2,1,self.radius,0)
end
