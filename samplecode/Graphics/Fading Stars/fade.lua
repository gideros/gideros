Fade =  Core.class(Sprite)

function Fade:init(sprite)
	self.alpha = 0
	self:setAlpha(0)

	self:addChild(sprite)

	self:addEventListener(Event.ADDED_TO_STAGE, self.onAddedToStage, self)
	self:addEventListener(Event.REMOVED_FROM_STAGE, self.onRemovedFromStage, self)
end

function Fade:onAddedToStage()
	self:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
end

function Fade:onRemovedFromStage()
	self:removeEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
end

function Fade:onEnterFrame()
	self.alpha = self.alpha + 0.01
	
	if self.alpha > 1 then
		self.alpha = 1
		self:removeEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
	end
	
	self:setAlpha(self.alpha)
end
