--[[

A nice example of a ball moving on the screen

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

-- load the background and add as a child of stage
local background = Bitmap.new(Texture.new("field.png"))
stage:addChild(background)

-- create a ball bitmap object (Bitmap class inherits from Sprite class)
local ball = Bitmap.new(Texture.new("ball.png"))

-- in Gideros, every created object is an ordinary Lua table
-- therefore, we can store our custom fields in this table
ball.xdirection = 1
ball.ydirection = 1
ball.xspeed = 2.5
ball.yspeed = 4.3

-- add the ball sprite to the stage
stage:addChild(ball)

-- before entering each frame, we update the position of the ball
function onEnterFrame(event)
	local x,y = ball:getPosition()

	x = x + (ball.xspeed * ball.xdirection)
	y = y + (ball.yspeed * ball.ydirection)
	
	if x < 0 then
		ball.xdirection = 1
	end
		
	if x > 320 - ball:getWidth() then
		ball.xdirection = -1
	end

	if y < 0 then
		ball.ydirection = 1
	end
		
	if y > 480 - ball:getHeight() then
		ball.ydirection = -1
	end

	ball:setPosition(x, y)
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)
