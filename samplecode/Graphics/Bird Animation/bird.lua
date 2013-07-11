--[[
This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 
--]]

Bird = Core.class(Sprite)

function Bird:init(frameList)
	-- create a Bitmap for each frame
	self.frames = {}
	for i = 1, #frameList do
		self.frames[i] = Bitmap.new(Texture.new(frameList[i]))
	end

	self.nframes = #frameList

	-- add first Bitmap as a child
	self.frame = 1
	self:addChild(self.frames[1])
	
	-- subframe is used to adjust the speed of animation
	self.subframe = 0

	-- set initial position
	self:setPosition(540, math.random(160) + 40)

	-- set the speed of the bird
	self.speedy = math.random(-500, 500) / 1000
	self.speedx = math.random(2000, 4000) / 1000
	
	self:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)	
end

function Bird:onEnterFrame()
	self.subframe = self.subframe + 1
	
	-- for every 10 frames, we remove the old frame and add the new frame as a child
	if self.subframe > 10 then
		self:removeChild(self.frames[self.frame])

		self.frame = self.frame + 1
		if self.frame > self.nframes then
			self.frame = 1
		end

		self:addChild(self.frames[self.frame])

		self.subframe = 0
	end
	
	-- get the current position
	local x,y = self:getPosition()
		
	-- change the position according to the speed
	x = x - self.speedx
	y = y + self.speedy

	-- if the bird is out of the screen, set new speed and position
	if x < -100 then 
		self.speedy = math.random(-500, 500) / 1000
		self.speedx = math.random(2000, 4000) / 1000
		x = 540
		y = math.random(160) + 40
	end

	-- set the new position
	self:setPosition(x, y)
end
