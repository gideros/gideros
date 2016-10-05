--[[

A frame by frame bird animation example
The old frame is removed by Sprite:removeChild and the new frame is added by Sprite:addChild

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

local bit=require("bit")
print("bitop output=",bit.tobit(0xffffffff))
print("bitop bor=",bit.bor(1, 2, 4, 8))

-- load texture, create bitmap from it and set as background
local bgTexture=Texture.new("sky_world.png")
local bgTextureP=Texture.new("field.png")

local background = Bitmap.new(bgTexture)

stage:addChild(background)

print ("local=",application:getLocale())
print ("language=",application:getLanguage())

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

function onKeyDown(event)
  print (event.keyCode)
  if event.keyCode==KeyCode.F then
    application:setFullScreen(true)

  elseif event.keyCode==KeyCode.W then
    application:setFullScreen(false)

  elseif event.keyCode==KeyCode.A then
    application:setWindowSize(400,700)

  elseif event.keyCode==KeyCode.P then
    application:setOrientation(Application.PORTRAIT)
    application:setWindowSize(300,600)
    background:setTexture(bgTextureP)

  elseif event.keyCode==KeyCode.L then
    application:setOrientation(Application.LANDSCAPE_LEFT)
    application:setWindowSize(300,600)
    background:setTexture(bgTexture)
  end
end

function onMouseDown(event)
print (event.x, event.y)
end

stage:addEventListener(Event.KEY_DOWN, onKeyDown)
stage:addEventListener(Event.MOUSE_DOWN, onMouseDown)
--application:setWindowSize(320,480)

function onComplete(event)
  print ("Lua reads a website!: ",event.data)
end

local filename = "ego.png"
local file = io.open(filename, "rb")
local contents = file:read( "*a" )
local boundary = "somerndstring"
 
local send = "--"..boundary..
			"\r\nContent-Disposition: form-data; "..
			"name="..filename.."; filename="..filename..
			"\r\nContent-type: image/png"..
			"\r\n\r\n"..contents..
			"\r\n--"..boundary.."--\r\n";
			

 
local headers = {
	["Content-Type"] = "multipart/form-data; boundary="..boundary,
	["Content-Length"] = #send,
}

loader = UrlLoader.new("http://httpbin.org/post", UrlLoader.POST, headers, send)

loader:addEventListener(Event.COMPLETE, onComplete)
