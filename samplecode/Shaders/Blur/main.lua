
local shader=Shader.new("vShader","fShader",0,
{
{name="vMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP,vertex=true},
{name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=false},
{name="fTexture",type=Shader.CTEXTURE,vertex=false},
{name="fTexelSize",type=Shader.CFLOAT4,vertex=false},
{name="fRad",type=Shader.CINT,vertex=false},
},
{
{name="vVertex",type=Shader.DFLOAT,mult=3,slot=0,offset=0},
{name="vColor",type=Shader.DUBYTE,mult=4,slot=1,offset=0},
{name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
});

print(shader:isValid())
local tex=Texture.new("texture.jpg",false)
local texw=tex:getWidth()
local texh=tex:getHeight()
local spr=Bitmap.new(tex)
shader:setConstant("fRad",Shader.CINT,1,0) --Initial blur level
shader:setConstant("fTexelSize",Shader.CFLOAT4,1,{1/texw,1/texh,0,0}) --Initial texel size
spr:setShader(shader)

local rtgt=RenderTarget.new(texw,texh,false)
local stgt=Bitmap.new(rtgt)
stgt:setShader(shader)

stage:addChild(stgt)

srot=0
stage:addEventListener(Event.ENTER_FRAME,function ()
	srot=srot+1
	local shad=math.floor((math.cos(srot/50)+1)*5)
	shader:setConstant("fRad",Shader.CINT,1,shad) --Set blur factor
	shader:setConstant("fTexelSize",Shader.CFLOAT4,1,{0,1/texh,0,0}) --Step 1: Vertical blur
	rtgt:draw(spr);
	shader:setConstant("fTexelSize",Shader.CFLOAT4,1,{1/texw,0,0,0}) --Step 2: Horizontal blur
end)
