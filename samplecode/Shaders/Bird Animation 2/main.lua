--[[

A frame by frame bird animation example
The old frame is removed by Sprite:removeChild and the new frame is added by Sprite:addChild

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2013 Gideros Mobile 

]]

local mystage = Sprite.new()

-- load texture, create bitmap from it and set as background
local background = Bitmap.new(Texture.new("sky_world.png"))
mystage:addChild(background)

-- these arrays contain the image file names of each frame
local frames1 = {
	"bird_black_01.png",
	"bird_black_02.png",
	"bird_black_03.png"}

local frames2 = {
	"bird_white_01.png",
	"bird_white_02.png",
	"bird_white_03.png"}

-- create 2 white and 2 black birds
local bird1 = Bird.new(frames1)
local bird2 = Bird.new(frames1)
local bird3 = Bird.new(frames2)
local bird4 = Bird.new(frames2)

-- add birds to the stage
mystage:addChild(bird1)
mystage:addChild(bird2)
mystage:addChild(bird3)
mystage:addChild(bird4)

local rt = RenderTarget.new(480, 320, true)

local mesh = Mesh.new()
mesh:setVertexArray(0, 0, 480, 0, 480, 320, 0, 320)
mesh:setTextureCoordinateArray(0, 0, 480, 0, 480, 320, 0, 320)
mesh:setIndexArray(1, 2, 3, 1, 3, 4)
mesh:setTexture(rt)
stage:addChild(mesh)

local effect = Shader.new("vs","ps",0,
{
{name="g_MVPMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WVP, vertex=true},
{name="g_Color",type=Shader.CFLOAT4,mult=1,sys=Shader.SYS_COLOR},
{name="g_Texture",type=Shader.CTEXTURE,mult=1,vertex=false},
{name="time",type=Shader.CFLOAT,mult=1,vertex=false}
},
{
{name="POSITION0",type=Shader.DFLOAT,mult=3,slot=0,offset=0},
{name="vColor",type=Shader.DUBYTE,mult=0,slot=1,offset=0},
{name="TEXCOORD0",type=Shader.DFLOAT,mult=2,slot=2,offset=0}
})

mesh:setShader(effect)

local start = os.timer()
local function onEnterFrame()
	effect:setConstant("time", Shader.CFLOAT,1,os.timer() - start)
	rt:clear(0xffffff, 1.0)
	rt:draw(mystage)
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)
