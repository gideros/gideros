--[[

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

local font = TTFont.new("Roboto-Regular.ttf", 18)

local text1 = TextField.new(font, "press keys on your android")
text1:setPosition(10, 100)
stage:addChild(text1)

local text2 = TextField.new(font, "press back button 3 time(s) to exit")
text2:setPosition(10, 130)
stage:addChild(text2)

-- key codes are integer. we map to strings so that we can display key name easily
local keyNames = {
	[KeyCode.BACK] = "back",
	[KeyCode.SEARCH] = "search",
	[KeyCode.MENU] = "menu",
	[KeyCode.CENTER] = "center",
	[KeyCode.SELECT] = "select",
	[KeyCode.START] = "start",
	[KeyCode.L1] = "L1",
	[KeyCode.R1] = "R1",
	[KeyCode.LEFT] = "left",
	[KeyCode.UP] = "up",
	[KeyCode.RIGHT] = "right",
	[KeyCode.DOWN] = "down",
	[KeyCode.X] = "x",
	[KeyCode.Y] = "y",
}

local function onKeyDown(event)
	text1:setText("key down: "..keyNames[event.keyCode])
end

local backCount = 0

local function onKeyUp(event)
	text1:setText("key up: "..keyNames[event.keyCode])
	
	if event.keyCode == KeyCode.BACK then
		backCount = backCount + 1		
		text2:setText("press back button "..(3 - backCount).." time(s) to exit")
		if backCount == 3 then
			application:exit()
		end
	end
end

-- key events are dispatched to all Sprite instances on the scene tree (similar to mouse and touch events)
stage:addEventListener(Event.KEY_DOWN, onKeyDown)
stage:addEventListener(Event.KEY_UP, onKeyUp)
