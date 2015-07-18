--[[

Displays the touch event data.
You need to run this example on real device.

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

local info = TextField.new(nil, "you need to run this example on a real device")
info:setPosition(40, 20)
stage:addChild(info)

local function touch2str(touch)
	return string.format("#%d=%g,%g", touch.id, touch.x, touch.y)
end

local function touches2str(touches)
	local str = ""
	
	for i=1,#touches do
		str = str..touch2str(touches[i])
		if i < #touches then
			str = str.."; "
		end
	end

	return str
end

local function printTouches(type, event)
	print(type.." ["..touch2str(event.touch).."] - ["..touches2str(event.allTouches).."]")
end

local dot1 = Bitmap.new(Texture.new("1.png"))
dot1:setAnchorPoint(0.5, 0.5)

local dot2 = Bitmap.new(Texture.new("2.png"))
dot2:setAnchorPoint(0.5, 0.5)

local dot3 = Bitmap.new(Texture.new("3.png"))
dot3:setAnchorPoint(0.5, 0.5)

local dot4 = Bitmap.new(Texture.new("4.png"))
dot4:setAnchorPoint(0.5, 0.5)

local dot5 = Bitmap.new(Texture.new("5.png"))
dot5:setAnchorPoint(0.5, 0.5)

local dots = {dot1, dot2, dot3, dot4, dot5}

local function onTouchesBegin(event)
	printTouches("BEGIN", event)

	local dot = dots[event.touch.id]
	if dot then
		stage:addChild(dot)
		dot:setPosition(event.touch.x, event.touch.y)
	end
end

local function onTouchesMove(event)
	printTouches("MOVE", event)

	local dot = dots[event.touch.id]
	if dot then
		dot:setPosition(event.touch.x, event.touch.y)
	end
end

local function onTouchesEnd(event)
	printTouches("END", event)

	local dot = dots[event.touch.id]
	if dot and stage:contains(dot) then
		stage:removeChild(dot)
	end
end

local function onTouchesCancel(event)
	printTouches("CANCEL", event)

	local dot = dots[event.touch.id]
	if dot and stage:contains(dot) then
		stage:removeChild(dot)
	end
end

stage:addEventListener(Event.TOUCHES_BEGIN, onTouchesBegin)
stage:addEventListener(Event.TOUCHES_MOVE, onTouchesMove)
stage:addEventListener(Event.TOUCHES_END, onTouchesEnd)
stage:addEventListener(Event.TOUCHES_CANCEL, onTouchesCancel)
