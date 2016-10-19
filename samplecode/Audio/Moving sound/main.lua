local audioScale=0.02
local scrW,scrH=application:getContentWidth(),application:getContentHeight()
Sound.setListenerPosition(scrW*audioScale/2,scrH*audioScale/2,0,0,0,0,0,1,0,0,0,1)

local label = TextField.new(nil, "Touch/Click on the screen and move the dots")
label:setPosition(5,20)
stage:addChild(label)

local dot1 = Bitmap.new(Texture.new("1.png"))
dot1:setAnchorPoint(0.5, 0.5)

local dot2 = Bitmap.new(Texture.new("2.png"))
dot2:setAnchorPoint(0.5, 0.5)

local dot3 = Bitmap.new(Texture.new("3.png"))
dot3:setAnchorPoint(0.5, 0.5)

local snd1=Sound.new("1.wav")
dot1.sndchn=snd1:play(0,true,true)
local snd2=Sound.new("2.wav")
dot2.sndchn=snd2:play(0,true,true)
local snd3=Sound.new("3.wav")
dot3.sndchn=snd3:play(0,true,true)


local dots = {dot1, dot2, dot3}


local function onTouchesBegin(event)
	local dot = dots[event.touch.id]
	if dot then
		stage:addChild(dot)
		dot:setPosition(event.touch.x, event.touch.y)
		dot.sndchn:setWorldPosition(event.touch.x*audioScale, event.touch.y*audioScale)
		dot.sndchn:setPaused(false)
	end
end

local function onTouchesMove(event)
	local dot = dots[event.touch.id]
	if dot then
		dot:setPosition(event.touch.x, event.touch.y)
		dot.sndchn:setWorldPosition(event.touch.x*audioScale, event.touch.y*audioScale)
	end
end

local function onTouchesEnd(event)
	local dot = dots[event.touch.id]
	if dot and stage:contains(dot) then
		stage:removeChild(dot)
		dot.sndchn:setPaused(true)
	end
end

local function onTouchesCancel(event)
	local dot = dots[event.touch.id]
	if dot and stage:contains(dot) then
		stage:removeChild(dot)
		dot.sndchn:setPaused(true)
	end
end

stage:addEventListener(Event.TOUCHES_BEGIN, onTouchesBegin)
stage:addEventListener(Event.TOUCHES_MOVE, onTouchesMove)
stage:addEventListener(Event.TOUCHES_END, onTouchesEnd)
stage:addEventListener(Event.TOUCHES_CANCEL, onTouchesCancel)

