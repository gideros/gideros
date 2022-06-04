--!NOEXEC
Capsule = Core.class(CollisionShape, function(...) return "Capsule" end)

function Capsule:init(x, y, h, r)
	self.collisionShape = CuteC2.capsule(x, y, h, r)
end

function Capsule:contains(mx, my)
	return self.collisionShape:hitTest(mx, my)
end

function Capsule:onMove(dx, dy, mx, my)
	self.collisionShape:move(dx, dy)
end

function Capsule:onSizeChanged(dx, dy, mx, my)
	local shape = self.collisionShape
	local radius = shape:getRadius()
	shape:setHeight(shape:getHeight() + dy)
	
	if (mx < shape:getX()) then 
		shape:setRadius(radius - dx)
	else
		shape:setRadius(radius + dx)
	end
end

function Capsule:onPropertiesDraw(ui)
	local shape = self.collisionShape
	local x, y = shape:getPosition()
	local r, h = shape:getSize()
	local changed = false
	
	x, y, changed = ui:dragFloat2("Position", x, y)
	
	if (changed) then 
		shape:setPosition(x, y)
	end
	
	changed = false
	h, r, changed = ui:dragFloat2("Height, radius", h, r)
	
	if (changed) then 
		shape:getSize(r, h)
	end
end

function Capsule:redraw(list)
	local shape = self.collisionShape
	local x, y = shape:getPosition()
	local r, h = shape:getSize()
	
	drawCapsule(list, x, y, h, r, self.drawColor)
end