GiftizButton = Core.class(Sprite)

function GiftizButton:init(path)
	if not giftiz then
		require "giftiz"
	end
	self.logo = Texture.new(path.."/giftiz_logo.png", true)
	self.badge = Texture.new(path.."/giftiz_logo_badge.png", true)
	self.warning = Texture.new(path.."/giftiz_logo_warning.png", true)
	self.button = Bitmap.new(self.logo)
	self.state = giftiz:getButtonState()
	self:updateButton()
	self:addEventListener(Event.ADDED_TO_STAGE, self.__added, self)
	self:addEventListener(Event.REMOVED_FROM_STAGE, self.__removed, self)
	self:addEventListener(Event.MOUSE_UP, self.onMouseUp, self)
end

function GiftizButton:updateButton()
	if self.state == Giftiz.INVISIBLE then
		self.button:removeFromParent()
	elseif self.state == Giftiz.DEFAULT then
		self.button:setTexture(self.logo)
		self:addChild(self.button)
	elseif self.state == Giftiz.BADGE then
		self.button:setTexture(self.badge)
		self:addChild(self.button)
	elseif self.state == Giftiz.WARNING then
		self.button:setTexture(self.warning)
		self:addChild(self.button)
	end
end

function GiftizButton:onButtonStateChange(e)
	self.state = e.buttonState
	self:updateButton()
end

function GiftizButton:onMouseUp(e)
	if self:hitTestPoint(e.x, e.y) then
		e:stopPropagation()
		giftiz:buttonClicked()
	end
end

function GiftizButton:__added()
	giftiz:addEventListener(Event.BUTTON_STATE_CHANGE, self.onButtonStateChange, self)
end

function GiftizButton:__removed()
	giftiz:removeEventListener(Event.BUTTON_STATE_CHANGE, self.onButtonStateChange, self)
end