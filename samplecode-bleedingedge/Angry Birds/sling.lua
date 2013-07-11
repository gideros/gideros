Sling = gideros.class(Sprite)

function Sling:init()
	local sling1 = Bitmap.new(Texture.new("sling1.png"))
	sling1:setPosition(28, 7)
	local sling2 = Bitmap.new(Texture.new("sling2.png"))

	self.acorn = Bitmap.new(Texture.new("acorn.png"))
	self.acorn:setAnchorPoint(0.5, 0.5)
	
	self.acornLayer = Sprite.new()
	
	self.acornLayer:addChild(self.acorn)

	self.rubber1 = Shape.new()
	self.rubber2 = Shape.new()

	self:addChild(sling1)
	self:addChild(self.rubber1)
	self:addChild(self.acornLayer)
	self:addChild(self.rubber2)
	self:addChild(sling2)
	
	self:addEventListener(Event.MOUSE_DOWN, self.onMouseDown, self)
	self:addEventListener(Event.MOUSE_MOVE, self.onMouseMove, self)
	self:addEventListener(Event.MOUSE_UP, self.onMouseUp, self)
	
	self:setAcornPosition(36, 52)
end

function Sling:onMouseDown(event)
	if self.acorn:hitTestPoint(event.x, event.y) then
		self.stretch = true
		self:setAcornPosition(event.x, event.y)
	end
end

function Sling:onMouseMove(event)
	if self.stretch then
		self:setAcornPosition(event.x, event.y)
	end
end

function Sling:onMouseUp()
	if self.stretch then
		self.stretch = false
		
		self.acorn:setVisible(false)
		self.rubber1:setVisible(false)
		self.rubber2:setVisible(false)
		
		local event = Event.new("fire")		
		event.x = self.acornx
		event.y = self.acorny		
		event.dx = self.rubberx - 35
		event.dy = self.rubbery - 35

		self:dispatchEvent(event)
	end
end

function Sling:setAcornPosition(x, y)
	x, y = self:globalToLocal(x, y)

	x, y = self:limitPosition(x, y)

	self.rubber1:clear()
	self.rubber1:setLineStyle(6, 0x301708)
	self.rubber1:beginPath()
	self.rubber1:moveTo(13, 34)
	self.rubber1:lineTo(x, y)
	self.rubber1:endPath()

	self.rubber2:clear()
	self.rubber2:setLineStyle(6, 0x301708)
	self.rubber2:beginPath()
	self.rubber2:moveTo(57, 36)
	self.rubber2:lineTo(x, y)
	self.rubber2:endPath()

	self.rubberx, self.rubbery = x, y

	local dx, dy = x - 35, y - 35
	local angle = math.atan2(dy, dx)
	local c = math.cos(angle)
	local s = math.sin(angle)
	
	x = x - 15 * c
	y = y - 15 * s
	
	self.acorn:setPosition(x, y)
	
	self.acornx, self.acorny = x, y
end

function Sling:limitPosition(x, y)
	local dx, dy = x - 35, y - 35

	local length = math.sqrt(dx * dx + dy * dy)
	
	dx = dx / length
	dy = dy / length
	
	length = math.min(length, 100)
	
	return 35 + length * dx, 35 + length * dy
end
