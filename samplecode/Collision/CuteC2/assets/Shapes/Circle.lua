--!NOEXEC
Circle = Core.class(CollisionShape, function(...) return "Circle" end)

function Circle:init(x, y, r)
	self.collisionShape = CuteC2.circle(x, y, r)
end

function Circle:contains(mx, my)
	return self.collisionShape:hitTest(mx, my)
end

function Circle:onMove(dx, dy, mx, my)
	self.collisionShape:move(dx, dy)
end

function Circle:onStartResize(mx, my)
	local x, y = self.collisionShape:getPosition()
	local d = math.distance(x, y, mx, my)
	self.startDistance = d -  self.collisionShape:getRadius()
end

function Circle:onSizeChanged(dx, dy, mx, my)
	local x, y = self.collisionShape:getPosition()
	local d = math.distance(x, y, mx, my)
	self.collisionShape:setRadius(d - self.startDistance)
end

function Circle:onPropertiesDraw(ui)
	local shape = self.collisionShape
	local x, y = shape:getPosition()
	local r = shape:getRadius()
	local changed = false
	x, y, r, changed = ui:dragFloat3("Position, radius", x, y, r)
	
	if (changed) then 
		shape:setPosition(x, y)
		shape:setRadius(r)
	end
end

function Circle:redraw(list)
	local shape = self.collisionShape
	local x, y = shape:getPosition()
	local r = shape:getRadius()
	
	drawCircle(list, x, y, r, self.drawColor)
end