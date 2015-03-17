--helper function from Andy Bower
function Sprite:isVisibleDeeply()
	-- Answer true only if the sprite and all it's a parents are visible. Normally, isVisible() will
	-- return true even if a sprite is actually not visible on screen by wont of one of it's parents
	-- being made invisible.
	--
	local try=self
	while (try) do
		local scaleX, scaleY = try:getScale()
		if  not(try:isVisible() and try:getAlpha()>0 and scaleX > 0 and scaleY > 0) then
			return false
		end
		try = try:getParent()
	end
	return true
end

OUYALayer = Core.class(Sprite)

function OUYALayer:init()
	-- Settings
	self.conf = {
		lineWidth = 5,
	}
	if not ouya then
		require "ouya"
	end
	self.__hover = nil
	self.__defaultHover = Shape.new()
	if GTween then
		self.__tweener = GTween.new(self.__defaultHover, 0.1, {})
	end
	self.__objects = {}
	self.__selected = 1
	self:addEventListener(Event.ADDED_TO_STAGE, self.__added, self)
	self:addEventListener(Event.REMOVED_FROM_STAGE, self.__removed, self)
	self:addEventListener(Event.ENTER_FRAME, self.__frame, self)
end

function OUYALayer:addObject(sprite)
	if sprite:getParent() ~= nil then
		table.insert(self.__objects, sprite)
	end
	sprite:addEventListener(Event.REMOVED_FROM_STAGE, function()
		for i = 1, #self.__objects do
			if sprite == self.__objects[i] then
				table.remove(self.__objects, i)
				break
			end
		end
	end)
	
	sprite:addEventListener(Event.ADDED_TO_STAGE, function()
		table.insert(self.__objects, sprite)
	end)
end

function OUYALayer:__handleKeys(e)
	if e.keyCode == KeyCode.BUTTON_O then
		if self.__objects[self.__selected] then
			local sprite = self.__objects[self.__selected]
			local x, y, _, _ = sprite:getBounds(self) --get global coordinates
			local offset = 10
			x, y = self:localToGlobal(x + offset, y + offset)
			if sprite:hasEventListener(Event.MOUSE_DOWN) then
				local event = Event.new(Event.MOUSE_DOWN)
				event.x = x
				event.y = y
				sprite:dispatchEvent(event)
			end
			if sprite:hasEventListener(Event.MOUSE_UP) then
				local event = Event.new(Event.MOUSE_UP)
				event.x = x
				event.y = y
				sprite:dispatchEvent(event)
			end
		end
	elseif e.keyCode == KeyCode.BUTTON_A then
		if self.back then
			self:back()
		end
	elseif e.keyCode == KeyCode.BUTTON_DPAD_DOWN then
		self:__goDown()
	elseif e.keyCode == KeyCode.BUTTON_DPAD_LEFT then
		self:__goLeft()
	elseif e.keyCode == KeyCode.BUTTON_DPAD_RIGHT then
		self:__goRight()
	elseif e.keyCode == KeyCode.BUTTON_DPAD_UP then
		self:__goUp()
	end
end

function OUYALayer:setHover(sprite)
	self.__hover = sprite
end

function OUYALayer:__hoverNext()
	local sprite = self.__objects[self.__selected]
	if sprite then
		--local parent = sprite:getParent()
		local x, y, width, height = sprite:getBounds(self)
	
		if width + self.conf.lineWidth ~= self.__defaultHover:getWidth() or height + self.conf.lineWidth ~= self.__defaultHover:getHeight() then
			self.__defaultHover:clear()
			self.__defaultHover:setLineStyle(5, 0xffff00)
			self.__defaultHover:setFillStyle(Shape.SOLID, 0x0000ff, 0.3)
			self.__defaultHover:beginPath()
			self.__defaultHover:moveTo(0, 0)
			self.__defaultHover:lineTo(width, 0)
			self.__defaultHover:lineTo(width, height)
			self.__defaultHover:lineTo(0, height)
			self.__defaultHover:closePath()
			self.__defaultHover:endPath()
		end
		if x ~= self.__defaultHover:getX() or y ~= self.__defaultHover:getY() then
			if self.__tweener then
				self.__tweener:resetValues({x = x, y = y})
			else
				self.__defaultHover:setPosition(x, y)
			end
			--self.__defaultHover:setPosition(x, y)
			--parent:addChildAt(self.__defaultHover, parent:getChildIndex(sprite))
		end
		self:addChild(self.__defaultHover)
	end
end

function OUYALayer:__goUp()
	local current = self.__selected
	self.__selected = self.__selected - 1
	while current ~= self.__selected do
		if self.__selected == 0 then
			self.__selected = #self.__objects
		end
		if self:__checkObject(self.__selected) then
			break
		end
		self.__selected = self.__selected - 1
	end
end

function OUYALayer:__goDown()
	local current = self.__selected
	self.__selected = self.__selected + 1
	while current ~= self.__selected do
		if self.__selected == #self.__objects + 1 then
			self.__selected = 1
		end
		if self:__checkObject(self.__selected) then
			break
		end
		self.__selected = self.__selected + 1
	end
end

function OUYALayer:__goLeft()
	local current = self.__selected
	self.__selected = self.__selected - 1
	while current ~= self.__selected do
		if self.__selected == 0 then
			self.__selected = #self.__objects
		end
		if self:__checkObject(self.__selected) then
			break
		end
		self.__selected = self.__selected - 1
	end
end

function OUYALayer:__goRight()
	local current = self.__selected
	self.__selected = self.__selected + 1
	while current ~= self.__selected do
		if self.__selected == #self.__objects + 1 then
			self.__selected = 1
		end
		if self:__checkObject(self.__selected) then
			break
		end
		self.__selected = self.__selected + 1
	end
end

function OUYALayer:__checkObject(id)
	local sprite = self.__objects[id]
	if sprite then
		return sprite:isVisibleDeeply()
	end
	return false
end

function OUYALayer:__proxyEvent(event)
	self:dispatchEvent(event)
end

function OUYALayer:__added()
	ouya:addEventListener(Event.KEY_DOWN, OUYALayer.__handleKeys, self)
	ouya:addEventListener(Event.KEY_DOWN, OUYALayer.__proxyEvent, self)
	ouya:addEventListener(Event.KEY_UP, OUYALayer.__proxyEvent, self)
	ouya:addEventListener(Event.RIGHT_JOYSTICK, OUYALayer.__proxyEvent, self)
	ouya:addEventListener(Event.LEFT_JOYSTICK, OUYALayer.__proxyEvent, self)
	ouya:addEventListener(Event.RIGHT_TRIGGER, OUYALayer.__proxyEvent, self)
	ouya:addEventListener(Event.LEFT_TRIGGER, OUYALayer.__proxyEvent, self)
end

function OUYALayer:__removed()
	ouya:removeEventListener(Event.KEY_DOWN, OUYALayer.__handleKeys, self)
	ouya:removeEventListener(Event.KEY_DOWN, OUYALayer.__proxyEvent, self)
	ouya:removeEventListener(Event.KEY_UP, OUYALayer.__proxyEvent, self)
	ouya:removeEventListener(Event.RIGHT_JOYSTICK, OUYALayer.__proxyEvent, self)
	ouya:removeEventListener(Event.LEFT_JOYSTICK, OUYALayer.__proxyEvent, self)
	ouya:removeEventListener(Event.RIGHT_TRIGGER, OUYALayer.__proxyEvent, self)
	ouya:removeEventListener(Event.LEFT_TRIGGER, OUYALayer.__proxyEvent, self)
end

function OUYALayer:__frame()
	self:__hoverNext()
end