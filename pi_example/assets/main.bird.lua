--[[

A frame by frame bird animation example
The old frame is removed by Sprite:removeChild and the new frame is added by Sprite:addChild

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

application:setBackgroundColor(255)

application:setFps(60)
print ("hello")

-- load texture, create bitmap from it and set as background
local background = Bitmap.new(Texture.new("sky_world.png"))
stage:addChild(background)

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
stage:addChild(bird1)
stage:addChild(bird2)
stage:addChild(bird3)
stage:addChild(bird4)

local alertDialog = AlertDialog.new("Error: not found", "Try reinstalling all software", "Cancel","OK", "No Way!")
local alertDialog2 = AlertDialog.new("Important Question", "Should the UK be a member of the EU?", "Cancel", "Yes", "NOOOOOOOOO")

local function onComplete(event)
  print(event.buttonIndex, event.buttonText)
end

local function onComplete2(event)
  print("Number 2:", event.buttonIndex, event.buttonText)
end


alertDialog:addEventListener(Event.COMPLETE, onComplete)
alertDialog2:addEventListener(Event.COMPLETE, onComplete2)

stage:addEventListener(Event.KEY_DOWN, function() alertDialog2:show() end)

sound=Sound.new("1.wav")
soundchannel=sound:play(0,false)
--soundchannel:setLooping(false)
--soundchannel:setPosition(300)
soundchannel:setPitch(1.5)

function onMouseDown(event)
  print (event.x, event.y)
end

stage:addEventListener(Event.MOUSE_DOWN, onMouseDown)