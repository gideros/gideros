--[[

A frame by frame bird animation example
The old frame is removed by Sprite:removeChild and the new frame is added by Sprite:addChild

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

application:setKeepAwake(true)
playfield=Mesh.new(true)

-- load texture, create bitmap from it and set as background
local background = Bitmap.new(Texture.new("sky_2.png"))
background:setScale(0.5)
playfield:addChild(background)
background:setZ(-1)

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
local bird5 = Bird.new(frames1)
local bird6 = Bird.new(frames1)
local bird7 = Bird.new(frames2)
local bird8 = Bird.new(frames2)


-- add birds to the stage
playfield:addChild(bird1)
playfield:addChild(bird2)
playfield:addChild(bird3)
playfield:addChild(bird4)
playfield:addChild(bird5)
playfield:addChild(bird6)
playfield:addChild(bird7)
playfield:addChild(bird8)
--stage:addChild(playfield)

w=application:getContentWidth()
h=application:getContentHeight()

local eyeDelta=0.0025--0.01
local eyeAngle=-math.atan(eyeDelta)*180/math.pi
local projection=Matrix.new()
projection:perspectiveProjection(90,w/h,0.1,1000)

local leftEye=Matrix.new()
leftEye:translate(-w/2,-h/2)
leftEye:scale(2/w,2/h,1)
leftEye:translate(-eyeDelta,0)
print(eyeAngle)
leftEye:rotate(-eyeAngle,0,1,0)

left=Viewport.new()
left:setPosition(w/4,h/2)
left:setClip(-1,-1,2,2)
left:setContent(playfield)
left:setScale(w/4,h/2,1)
left:setProjection(projection)
left:setTransform(leftEye)
stage:addChild(left)

local rightEye=Matrix.new()
rightEye:translate(-w/2,-h/2)
rightEye:scale(2/w,2/h,1)
rightEye:translate(eyeDelta,0)
rightEye:rotate(eyeAngle,0,1,0)


right=Viewport.new()
right:setPosition(3*w/4,h/2)
right:setClip(-1,-1,2,2)
right:setContent(playfield)
right:setScale(w/4,h/2,1)
right:setProjection(projection)
right:setTransform(rightEye)
stage:addChild(right)

text=TextField.new(nil,"Birds")
text:setScale(2)
text:setPosition(226,159,-0.999)
text:setTextColor(0xff0000)
playfield:addChild(text)
local counter=0

function gameLoop(e)
	counter=counter+1
	text:setZ((math.sin(counter/200)/2.5)-0.55)
end

stage:addEventListener(Event.ENTER_FRAME,gameLoop)
