local texture = Texture.new("cross.png")

local effect = Shader.new("vs","ps",0,
{
{name="g_MVPMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP, vertex=true},
{name="g_Color",type=Shader.CFLOAT4,mult=1,sys=Shader.SYS_COLOR},
{name="lightPos",type=Shader.CFLOAT4,mult=1,vertex=false},
{name="g_Texture",type=Shader.CTEXTURE,mult=1,vertex=false}
},
{
{name="POSITION0",type=Shader.DFLOAT,mult=3,slot=0,offset=0},
{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
{name="TEXCOORD0",type=Shader.DFLOAT,mult=2,slot=2,offset=0}
})

local mesh = Mesh.new()
mesh:setVertexArray(0, 0, 512, 0, 512, 512, 0, 512)
mesh:setTextureCoordinateArray(0, 0, 512, 0, 512, 512, 0, 512)
mesh:setIndexArray(1, 2, 3, 1, 3, 4)
mesh:setTexture(texture)
mesh:setShader(effect)

stage:addChild(mesh)

local function onEnterFrame(event)
	local x = 200 * math.cos(event.time * 2.1) + 256
	local y = 200 * math.sin(event.time * 1.4) + 256
	effect:setConstant("lightPos", Shader.CFLOAT4, 1, x, y,0,0)
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)
