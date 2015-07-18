--[[

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

AnimatedSprite = Core.class(Sprite)

function AnimatedSprite:init()
	local pack = TexturePack.new("anim.txt", "anim.png")
	
	self.anim = {
		Bitmap.new(pack:getTextureRegion("anim_01.png")),
		Bitmap.new(pack:getTextureRegion("anim_02.png")),
		Bitmap.new(pack:getTextureRegion("anim_03.png")),
		Bitmap.new(pack:getTextureRegion("anim_04.png")),
		Bitmap.new(pack:getTextureRegion("anim_05.png")),
		Bitmap.new(pack:getTextureRegion("anim_06.png")),
		Bitmap.new(pack:getTextureRegion("anim_07.png")),
		Bitmap.new(pack:getTextureRegion("anim_08.png")),
		Bitmap.new(pack:getTextureRegion("anim_09.png")),
		Bitmap.new(pack:getTextureRegion("anim_10.png")),
		Bitmap.new(pack:getTextureRegion("anim_11.png")),
		Bitmap.new(pack:getTextureRegion("anim_12.png")),
		Bitmap.new(pack:getTextureRegion("anim_13.png")),
	}
	
	self.frame = 1
	self:addChild(self.anim[1])

	self.nframes = #self.anim

	self.subframe = 0
	
	self:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
end

function AnimatedSprite:onEnterFrame()
	self.subframe = self.subframe + 1

	if self.subframe > 4 then
		self:removeChild(self.anim[self.frame])
		
		self.frame = self.frame + 1
		if self.frame > self.nframes then
			self.frame = 1
		end

		self:addChild(self.anim[self.frame])
		
		self.subframe = 0
	end
end

