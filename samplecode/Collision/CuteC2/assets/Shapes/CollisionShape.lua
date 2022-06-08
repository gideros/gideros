--!NOEXEC
CollisionShape = Core.class()

function CollisionShape:init(name)
	self.id = SHAPE_GLOBAL_ID
	self.name = name
	self.px = 0
	self.py = 0
	self.show = true
	self.dragPos = false
	self.dragSize = false
	SHAPE_GLOBAL_ID += 1
	
	self.drawColor = math.random(0xffffff)
end

function CollisionShape:shake(x, y)
	if (not self.timer) then 
		self.timer = Timer.new(10, 10)
		self.counter = 0.1
		self.originalX, self.originalY = self.collisionShape:getPosition()
		self.timer:start()
		
		self.timer:addEventListener(Event.TIMER, function()
			local dx = math.noise(self.originalX, self.originalY, self.counter) * 10
			local dy = math.noise(self.originalX, self.originalY, self.counter + 0.5) * 10
			self.counter += 0.6
			self.collisionShape:setPosition(self.originalX + dx, self.originalY + dy)
		end)
		
		self.timer:addEventListener(Event.TIMER_COMPLETE, function()
			self.collisionShape:setPosition(self.originalX, self.originalY)
			self.originalX = nil
			self.originalY = nil
			self.timer = nil
		end)
	end
end

function CollisionShape:contains(x, y)
	return false
end

function CollisionShape:startDrag(mx, my)
	self.px = mx
	self.py = my
	self.clickX = mx
	self.clickY = my
end

function CollisionShape:stopDrag()
	self.px = nil
	self.py = nil
	self.clickX = nil
	self.clickY = nil
end

function CollisionShape:updateDrag(mx, my)	
	local dx = mx - self.px
	local dy = my - self.py
	
	self.px = mx
	self.py = my
	
	return dx, dy
end

function CollisionShape:onStartMove(mx, my)
	-- override
end

function CollisionShape:onMove(dx, dy, mx, my)
	-- override
end

function CollisionShape:onStartResize(mx, my)
	-- override
end

function CollisionShape:onSizeChanged(dx, dy, mx, my)
	-- override
end

function CollisionShape:onPropertiesDraw(ui)
	-- override
end

function CollisionShape:redraw(list, isFilled, alpha)
	-- override
end

function CollisionShape:updateDragAndDrop(ui)	
	local mx, my = ui:getMousePos()
	
	if (ui:isMouseClicked(KeyCode.MOUSE_LEFT) and self:contains(mx, my)) then 
		self:startDrag(mx, my)
		self:onStartMove(mx, my)
		self.dragPos = true
	elseif (not self.dragPos and ui:isMouseClicked(KeyCode.MOUSE_RIGHT) and self:contains(mx, my)) then 
		self:startDrag(mx, my)
		self:onStartResize(mx, my)
		self.dragSize = true
	end
	
	if (self.dragPos) then 
		if (ui:isMouseReleased(KeyCode.MOUSE_LEFT)) then
			self.dragPos = false
			self:stopDrag()
		else
			local dx, dy = self:updateDrag(mx, my)
			self:onMove(dx, dy, mx, my)
		end
	end
	
	if (self.dragSize) then 
		if (ui:isMouseReleased(KeyCode.MOUSE_RIGHT)) then
			self.dragSize = false
			self:stopDrag()
		else
			local dx, dy = self:updateDrag(mx, my)
			self:onSizeChanged(dx, dy, mx, my)
		end
	end
end

function CollisionShape:drawProperties(ui)
	ui:pushID(self.id)
	self.show = ui:checkbox("##Visible", self.show)
	ui:sameLine()
	if (ui:treeNode(self.name)) then 
		self.drawColor = ui:colorEdit3("Color", self.drawColor)
		
		self:onPropertiesDraw(ui)
		ui:treePop()
	end
	ui:popID()
	ui:separator()
end

function CollisionShape:draw(ui)
	if (self.id < 0) then 
		self.id = ui:getID(self)
	end
	
	if (self.show) then 
		local list = ui:getBackgroundDrawList()
		
		self:redraw(list)
	end
	
	if (not ui:getIO():wantCaptureMouse()) then	
		self:updateDragAndDrop(ui)
	end
end

function CollisionShape:getType()
	return self.collisionShape.__shapeType
end