--[[

An animating color transform example

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

-- add all the images to the stage
local bitmap = Bitmap.new(Texture.new("bird.png"))
bitmap:setPosition(140, 12)
stage:addChild(bitmap)

local redbar = Bitmap.new(Texture.new("red-bar.png"))
redbar:setPosition(91, 230)
stage:addChild(redbar)

local greenbar = Bitmap.new(Texture.new("green-bar.png"))
greenbar:setPosition(91, 260)
stage:addChild(greenbar)

local bluebar = Bitmap.new(Texture.new("blue-bar.png"))
bluebar:setPosition(91, 290)
stage:addChild(bluebar)

local reddot = Bitmap.new(Texture.new("red-dot.png"))
reddot:setY(222)
stage:addChild(reddot)

local greendot = Bitmap.new(Texture.new("green-dot.png"))
greendot:setY(252)
stage:addChild(greendot)

local bluedot = Bitmap.new(Texture.new("blue-dot.png"))
bluedot:setY(282)
stage:addChild(bluedot)

local function onEnterFrame(event)
	-- get the precise timer
	local t = os.timer()
	
	-- animate r,g,b multipliers of color transform
	local r = (math.sin(t * 0.5 + 0.3) + 1) / 2
	local g = (math.sin(t * 0.8 + 0.2) + 1) / 2
	local b = (math.sin(t * 1.3 + 0.6) + 1) / 2
	
	-- set color transform
	bitmap:setColorTransform(r, g, b, 1)

	-- update the positions of dots
	reddot:setX(90 + r * 280)
	greendot:setX(90 + g * 280)
	bluedot:setX(90 + b * 280)
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)
