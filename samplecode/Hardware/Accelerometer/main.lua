--[[

Accelerometer example with basic low-pass filtering

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

local accelerometer = Accelerometer.new()
accelerometer:start()

local info = TextField.new(nil, "you need to run this example on a real device")
info:setPosition(40, 20)
stage:addChild(info)

local dot = Bitmap.new(Texture.new("dot.png"))
dot:setAnchorPoint(0.5, 0.5)
stage:addChild(dot)

local filter = 1.0  -- the filtering constant, 1.0 means no filtering, lower values mean more filtering
local fx, fy, fz = 0, 0, 0

local filterText = TextField.new()
filterText:setPosition(30, 390)
stage:addChild(filterText)

local function updateFilterText()
	filterText:setText("Noise filtering constant: " .. filter)
end

updateFilterText()

local function onEnterFrame()
	-- get accelerometer values
	local x, y, z = accelerometer:getAcceleration()

	-- do the low-pass filtering
	fx = x * filter + fx * (1 - filter)
	fy = y * filter + fy * (1 - filter)
	fz = z * filter + fz * (1 - filter)

	dot:setPosition(160 + fx * 160, 240 + fy * 240)
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)
local button01 = Button.new(Bitmap.new(Texture.new("0.1-up.png")), Bitmap.new(Texture.new("0.1-down.png")))
button01:setPosition(25, 400)
button01:addEventListener("click", function() filter = 0.1 updateFilterText() end)
stage:addChild(button01)

local button02 = Button.new(Bitmap.new(Texture.new("0.2-up.png")), Bitmap.new(Texture.new("0.2-down.png")))
button02:setPosition(95, 400)
button02:addEventListener("click", function() filter = 0.2 updateFilterText() end)
stage:addChild(button02)

local button05 = Button.new(Bitmap.new(Texture.new("0.5-up.png")), Bitmap.new(Texture.new("0.5-down.png")))
button05:setPosition(165, 400)
button05:addEventListener("click", function() filter = 0.5 updateFilterText() end)
stage:addChild(button05)

local button10 = Button.new(Bitmap.new(Texture.new("1.0-up.png")), Bitmap.new(Texture.new("1.0-down.png")))
button10:setPosition(235, 400)
button10:addEventListener("click", function() filter = 1.0 updateFilterText() end)
stage:addChild(button10)

