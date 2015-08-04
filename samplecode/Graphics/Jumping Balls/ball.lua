--[[

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

--]]


Ball = Core.class(Sprite)

function Ball:init(texture)
	local bitmap = Bitmap.new(Texture.new(texture))
	self:addChild(bitmap)

	self.xdirection = 1
	self.ydirection = 1
	self.xspeed = math.random(40, 100) / 20
	self.yspeed = math.random(40, 100) / 20
	
	self:setX(math.random(0, 270))
	self:setY(math.random(0, 430))

	self.width = self:getWidth()
	self.height = self:getHeight()
	
	self:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
end


function Ball:onEnterFrame(event)
	local x, y = self:getPosition()

	x = x + (self.xspeed * self.xdirection)
	y = y + (self.yspeed * self.ydirection)
	
	if x < 0 then
		self.xdirection = 1
	end
		
	if x > 320 - self.width then
		self.xdirection = -1
	end

	if y < 0 then
		self.ydirection = 1
	end
		
	if y > 480 - self.height then
		self.ydirection = -1
	end

	self:setPosition(x, y)
end