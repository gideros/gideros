--[[

A frame by frame bird animation example
The old frame is removed by Sprite:removeChild and the new frame is added by Sprite:addChild

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

--local bit=require("bit")
--print("bitop output=",bit.tobit(0xffffffff))
--print("bitop bor=",bit.bor(1, 2, 4, 8))

application:setBackgroundColor(255)

cl=application:get("commandLine")
print("command line=",cl)

application:setFps(60)
print ("hello from Lua!")

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
local alertDialog2 = AlertDialog.new("Important Question", "Should the UK be a member of the EU?", "NOOOOO")

local textInputDialog = TextInputDialog.new("Text Input","Enter your reg number","text","Cancel","YES","NO")
local textInputDialog2 = TextInputDialog.new("Text Input2","Enter your phonr number","number","Cancel","Submit")

local function onComplete(event)
  print(event.buttonIndex, event.buttonText)
end

local function onComplete2(event)
  print("Number 2:", event.buttonIndex, event.buttonText)
end

local function TonComplete(event)
  print("TextInputDialog",event.buttonIndex, event.buttonText, event.text)
  application:openUrl(event.text)
end

local function TonComplete2(event)
  print("TextInputDialog 2:", event.buttonIndex, event.buttonText, event.text)
end


alertDialog:addEventListener(Event.COMPLETE, onComplete)
alertDialog2:addEventListener(Event.COMPLETE, onComplete2)

textInputDialog:addEventListener(Event.COMPLETE,TonComplete)
textInputDialog2:addEventListener(Event.COMPLETE,TonComplete2)

stage:addEventListener(Event.KEY_DOWN, function(event) 
 				         if event.keyCode==KeyCode.F then 
				           application:setFullScreen(true) 
				         else 
				           application:setFullScreen(false)
					 end
				       end)

sound=Sound.new("1.wav")
soundchannel=sound:play(0,false)
soundchannel:setLooping(false)
--soundchannel:setPosition(300)
soundchannel:setPitch(1.5)

function onMouseDown(event)
  if event.x>240 then
    if event.y>160 then
       textInputDialog:show()
    else
       textInputDialog2:show()
    end
  else
    if event.y>160 then
       alertDialog:show()
    else
       alertDialog2:show()
    end
  end
end

stage:addEventListener(Event.MOUSE_DOWN, onMouseDown)