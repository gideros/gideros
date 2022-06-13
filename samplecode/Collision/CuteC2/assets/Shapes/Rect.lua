--!NOEXEC
Rect = Core.class(CollisionShape, function(...) return "Rect" end)

function Rect:init(x, y, w, h)
	self.collisionShape = CuteC2.aabb(x, y, x + w, y + h)
end

function Rect:contains(mx, my)
	return self.collisionShape:hitTest(mx, my)
end

function Rect:onMove(dx, dy, mx, my)
	self.collisionShape:move(dx, dy)
end

function Rect:onSizeChanged(dx, dy, mx, my)
	local w, h = self.collisionShape:getSize()
	self.collisionShape:setHalfSize(w + dx, h + dy)
end

function Rect:onPropertiesDraw(ui)
	local shape = self.collisionShape
	local x, y = shape:getPosition()
	local w, h = shape:getSize()
	local changed = false
	
	x, y, changed = ui:dragFloat2("Position", x, y)
	if (changed) then 
		shape:setPosition(x, y)
	end
	
	changed = false
	w, h, changed = ui:dragFloat2("Size", w, h)
	
	if (changed) then 
		shape:setSize(w, h)
	end
end

function Rect:redraw(list)
	local shape = self.collisionShape
	local x1, y1, x2, y2 = shape:getBoundingBox()
	
	drawRect(list, x1, y1, x2, y2, self.drawColor)
end