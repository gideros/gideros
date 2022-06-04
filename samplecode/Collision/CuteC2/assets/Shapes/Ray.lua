--!NOEXEC
Ray = Core.class(CollisionShape, function(...) return "Ray" end)

function Ray:init(sx, sy, ex, ey)
	self.collisionShape = CuteC2.ray(sx, sy, ex, ey)
end

function Ray:onPropertiesDraw(ui)
	local shape = self.collisionShape
	local x1, y1 = shape:getPosition()
	local oldX1 = x1
	local oldY1 = y1
	local changed = false
	
	x1, y1, changed = ui:dragFloat2("Source", x1, y1)
	
	if (changed) then 
		local dx = x1 - oldX1
		local dy = y1 - oldY1
		shape:setPosition(x1, y1)
		self:onMove(dx, dy)
	end
	
	local x2, y2 = shape:getFacePosition()
	changed = false
	x2, y2, changed = ui:dragFloat2("Target", x2, y2)
	
	if (changed) then 
		shape:faceTo(x2, y2)
		self:onMoveTarget()
	end
	
	local dir = shape:getDirection()
	changed = false
	dir, changed = ui:dragFloat("Rotation", dir, 0.01)
	
	if (changed) then 
		shape:setDirection(dir)
		self:onMoveTarget()
	end
end

function Ray:redraw(list, isFilled, alpha)
	local x1, y1 = self.collisionShape:getPosition()
	local x2, y2 = self.collisionShape:getFacePosition()
	
	list:addLine(x1, y1, x2, y2, self.drawColor, alpha)
	drawCircle(list, x1, y1, DRAG_POINT_RADIUS, self.drawColor)
	drawCircle(list, x2, y2, DRAG_POINT_RADIUS, self.drawColor)
end

function Ray:updateDragAndDrop(ui)
	local mx, my = ui:getMousePos()
	
	if (ui:isMouseClicked(KeyCode.MOUSE_LEFT)) then 
		local x, y = self.collisionShape:getPosition()
		local d = math.distance(x, y, mx, my)
		
		if (d <= DRAG_POINT_RADIUS) then 
			self:startDrag(mx, my)
			self:onStartMove(mx, my)
			self.dragStartPos = true
		else
			x, y = self.collisionShape:getFacePosition()
			d = math.distance(x, y, mx, my)
			
			if (d <= DRAG_POINT_RADIUS) then 
				self:startDrag(mx, my)
				self:onStartMoveTarget(mx, my)
				self.offsetX = x - mx
				self.offsetY = y - my
				self.dragEndPos = true
			end
		end
	end
	
	if (self.dragStartPos) then
		if (ui:isMouseReleased(KeyCode.MOUSE_LEFT)) then
			self.dragStartPos = false
			self:stopDrag()
		else
			local dx, dy = self:updateDrag(mx, my)
			self.collisionShape:move(dx, dy)
			self:onMove(dx, dy, mx, my)
		end
	end
	
	if (self.dragEndPos) then
		if (ui:isMouseReleased(KeyCode.MOUSE_LEFT)) then
			self.dragEndPos = false
			self:stopDrag()
		else
			local dx, dy = self:updateDrag(mx, my)
			self.collisionShape:faceTo(mx + self.offsetX, my + self.offsetY)
			self:onMoveTarget(dx, dy, mx + self.offsetX, my + self.offsetY)
		end
	end
end

function Ray:onStartMoveTarget(mx, my)
	-- override
end

function Ray:onMoveTarget(dx, dy, x, y)
	-- override
end