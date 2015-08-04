--[[

5 stars fading in on the screen. Fade animation is done through a common Fade class.

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

local function onTimer(timer)
	local star = Bitmap.new(Texture.new("star.png"))
	star:setPosition(80, timer:getCurrentCount() * 80 - 60)

	local fade = Fade.new(star)
	stage:addChild(fade)
end


timer = Timer.new(500, 5)
timer:addEventListener(Event.TIMER, onTimer, timer)
timer:start()
