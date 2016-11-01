
Bloom=Core.class(Sprite)

Bloom.bloomExtract=Shader.new("vShader","fExtract",0,
{
{name="vMatrix",type=Shader.CMATRIX,mult=1,vertex=true,sys=Shader.SYS_WVP},
{name="BloomThreshold",type=Shader.CFLOAT,mult=1,vertex=false},
{name="fTexture",type=Shader.CTEXTURE,mult=1,vertex=false},
},
{
{name="vVertex",type=Shader.DFLOAT,mult=3,slot=0,offset=0},
{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
{name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
});

Bloom.bloomBlur=Shader.new("vShader","fBlur",0,
{
{name="vMatrix",type=Shader.CMATRIX,mult=1,vertex=true,sys=Shader.SYS_WVP},
{name="SampleOffsets",type=Shader.CFLOAT4,mult=15,vertex=false},
{name="SampleWeights",type=Shader.CFLOAT,mult=15,vertex=false},
{name="fTexture",type=Shader.CTEXTURE,mult=1,vertex=false},
},
{
{name="vVertex",type=Shader.DFLOAT,mult=3,slot=0,offset=0},
{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
{name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
});

Bloom.bloomCombine=Shader.new("vShader","fCombine",0,
{
{name="vMatrix",type=Shader.CMATRIX,mult=1,vertex=true,sys=Shader.SYS_WVP},
{name="BloomIntensity",type=Shader.CFLOAT,mult=1,vertex=false},
{name="BloomSaturation",type=Shader.CFLOAT,mult=1,vertex=false},
{name="BloomSampler",type=Shader.CTEXTURE,mult=1,vertex=false},
{name="BaseIntensity",type=Shader.CFLOAT,mult=1,vertex=false},
{name="BaseSaturation",type=Shader.CFLOAT,mult=1,vertex=false},
{name="BaseSampler",type=Shader.CTEXTURE,mult=1,vertex=false},
},
{
{name="vVertex",type=Shader.DFLOAT,mult=3,slot=0,offset=0},
{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
{name="vTexCoord",type=Shader.DFLOAT,mult=2,slot=2,offset=0},
});

function Bloom:init(texw,texh)
local gaussian=4

gaussian=gaussian*gaussian
self.bloomOffsetsH={};
self.bloomOffsetsV={};
self.bloomWeights={};

self.rtgts=RenderTarget.new(texw,texh,false)
self.stgts=Bitmap.new(self.rtgts)
self.stgts:setShader(Bloom.bloomExtract)

self.rtgt1=RenderTarget.new(texw,texh,false)
self.stgt1=Bitmap.new(self.rtgt1)
self.stgt1:setShader(Bloom.bloomBlur)

self.rtgt2=RenderTarget.new(texw,texh,false)
self.stgt2=Bitmap.new(self.rtgt2)
self.stgt2:setShader(Bloom.bloomBlur)

local tsw,tsh=self.rtgt1:getTexelSize();
for i=-7,7,1 do
	self.bloomWeights[i+8]=math.exp(-i*i/(2*gaussian))/math.sqrt(2*math.pi*gaussian)
	self.bloomOffsetsH[4*i+29]=0
	self.bloomOffsetsH[4*i+30]=0
	self.bloomOffsetsH[4*i+31]=0
	self.bloomOffsetsH[4*i+32]=0
	self.bloomOffsetsV[4*i+29]=0
	self.bloomOffsetsV[4*i+30]=0
	self.bloomOffsetsV[4*i+31]=0
	self.bloomOffsetsV[4*i+32]=0
	if (i~=0) then
	self.bloomOffsetsH[4*i+29]=(2*i)*tsw
	self.bloomOffsetsV[4*i+30]=(2*i)*tsh
	end
end

local comb=Mesh.new()
comb:setVertexArray(0,0,texw,0,texw,texh,0,texh)
comb:setTextureCoordinateArray(0,0,texw,0,texw,texh,0,texh)
comb:setIndexArray(1,2,3,1,3,4)
comb:setTexture(self.rtgt1)
comb:setTexture(self.rtgts,1)
comb:setShader(Bloom.bloomCombine)
self:addChild(comb)
end

function Bloom:draw(scene)
Bloom.bloomExtract:setConstant("BloomThreshold",Shader.CFLOAT,1,0.25) --Initial blur level
Bloom.bloomBlur:setConstant("SampleWeights",Shader.CFLOAT,15,self.bloomWeights) --Initial blur level
Bloom.bloomCombine:setConstant("BloomIntensity",Shader.CFLOAT,1,4)
Bloom.bloomCombine:setConstant("BaseIntensity",Shader.CFLOAT,1,2)
Bloom.bloomCombine:setConstant("BloomSaturation",Shader.CFLOAT,1,1)
Bloom.bloomCombine:setConstant("BaseSaturation",Shader.CFLOAT,1,1)
self.rtgts:draw(scene)

self.rtgt1:draw(self.stgts)
Bloom.bloomBlur:setConstant("SampleOffsets",Shader.CFLOAT4,15,self.bloomOffsetsH) --Initial blur level
self.rtgt2:draw(self.stgt1)
Bloom.bloomBlur:setConstant("SampleOffsets",Shader.CFLOAT4,15,self.bloomOffsetsV) --Initial blur level
self.rtgt1:draw(self.stgt2)
end
