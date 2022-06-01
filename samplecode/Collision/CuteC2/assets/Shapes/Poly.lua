--!NOEXEC
local function circleHitTest(x, y, r, tx, ty)
	local d = math.distance(x, y, tx, ty)
	return d <= r
end

-- check if point "c" is between "a" and "b"
local function isBetween(ax, ay, bx, by, cx, cy)
	local eps = 1e-1
	local dx1 = bx - ax
	local dy1 = by - ay
	local dx2 = cx - ax
	local dy2 = cy - ay
	
    local _, _, cross = math.cross(dx1,dy1, dx2,dy2)--dy2 * dx1 - dx2 * dy1

    if math.abs(cross) > eps then 
        return false
	end

    local dot = math.dot(dx1,dy1, dx2,dy2)
    if dot < 0 then
        return false
	end

    local sqlen = dx1 * dx1 + dy1 * dy1
	
    if dot > sqlen then
        return false
	end

    return true
end

Poly = Core.class(CollisionShape, function(...) return "Poly" end)

function Poly:init(x, y, points)
	self.collisionShape = CuteC2.poly(points)
	self.transform = CuteC2.transform(x, y)
	self.dragVertex = 0
	self.drawNormals = false
	self.drawBBOX = false
	
	-- used to test if mouse is clicked close to the shape
	-- but not inside
	self.mousePos = CuteC2.circle(0,0,4)
	
	self.ax = 0
	self.ay = 0
	self.bx = 0
	self.by = 0
	self.cx = 0
	self.cy = 0
end

function Poly:pointHitTest(x, y)
	local points = self.collisionShape:getRotatedPoints(self.transform)
	local tx, ty = self.transform:getPosition()
	
	for i, pt in ipairs(points) do
		local px = tx + pt.x
		local py = ty + pt.y
		
		if (circleHitTest(px, py, DRAG_POINT_RADIUS, x, y)) then
			return i, pt
		end
	end
	
end

function Poly:onVertexStartMove(mx, my)
end

function Poly:onStartRotate(mx, my)
	local x, y = self.transform:getPosition()
	local ang = math.atan2(self.clickY - y, self.clickX - x)
	self.clickAng = ang
	self.prevAng = ang
end

function Poly:onRotationChanged(dx, dy, mx, my)
	local x, y = self.transform:getPosition()
	local currentAng = math.atan2(my - y, mx - x)
	self.transform:rotate(currentAng - self.prevAng)
	self.prevAng = currentAng
end

function Poly:onVertexMove(ind, dx, dy, mx, my)
	-- update vertex position with respect to shape rotation
	local vx, vy = self.collisionShape:getVertex(ind)
	local rot = self.transform:getRotation()
	local len = math.length(dx, dy)
	local ang = math.atan2(dy, dx)
	
	ang -= rot
	vx += math.cos(ang) * len
	vy += math.sin(ang) * len
	
	self.collisionShape:setVertex(ind, vx, vy)
end

function Poly:onMove(dx, dy, mx, my)
	self.transform:move(dx, dy)
end

function Poly:updateDragAndDrop(ui)	
	local mx, my = ui:getMousePos()
	local tx, ty = self.transform:getPosition()
	
	if (ui:isMouseClicked(KeyCode.MOUSE_LEFT)) then 
		local ind = self:pointHitTest(mx, my)
		
		-- clicked on vertex
		if (ind) then
			self:startDrag(mx, my)
			self:onVertexStartMove(mx, my, ind)
			self.dragVertex = ind
		-- clicked on shape body OR its position origin point
		elseif (self.collisionShape:hitTest(mx, my, self.transform) or circleHitTest(tx, ty, DRAG_POINT_RADIUS, mx, my)) then 
			self:startDrag(mx, my)
			self:onStartMove(mx, my)
			self.dragShape = true
		end
	elseif (ui:isMouseClicked(KeyCode.MOUSE_RIGHT)) then 
		-- clicked on shape body
		if (self.collisionShape:hitTest(mx, my, self.transform)) then 
			self:startDrag(mx, my)
			self:onStartRotate(mx, my)
			self.dragRotation = true
		end
	elseif (ui:isMouseClicked(KeyCode.MOUSE_MIDDLE)) then 
		local ind = self:pointHitTest(mx, my)
		
		-- clicked on vertex
		if (ind) then
			self.collisionShape:removeVertex(ind)
		else
			-- using fake shape to use GJK
			self.mousePos:setPosition(mx, my)
			
			-- calculate distance to closest point
			local d, ax, ay = CuteC2.GJK(self.collisionShape, self.mousePos, self.transform)
			
			if (d < 50) then 
				local points = self.collisionShape:getRotatedPoints(self.transform)
				local count = #points
				
				-- find closest line segment
				for i, pt in ipairs(points) do
					-- if too close to vertex point, break
					if (circleHitTest(tx + pt.x, ty + pt.y, DRAG_POINT_RADIUS, ax, ay)) then 
						break
					end
					
					-- loop point index
					local nextI = (i+1) % (count+1)
					
					if (nextI == 0) then 
						nextI = 1
					end
					
					local nextPoint = points[nextI]
					-- check if current line segment "contains" closest point given by GJK algorithm
					if (isBetween(pt.x, pt.y, nextPoint.x, nextPoint.y, ax - tx, ay - ty)) then 
						self.collisionShape:insertVertex(mx - tx, my - ty, i, self.transform)
						break
					end
				end
			end
		end
	end
	
	if (self.dragVertex > 0) then 
		if (ui:isMouseReleased(KeyCode.MOUSE_LEFT)) then
			self.dragVertex = 0
			self:stopDrag()
		else
			local dx, dy = self:updateDrag(mx, my)
			self:onVertexMove(self.dragVertex, dx, dy, mx, my)
		end
	end
	
	if (self.dragShape) then 
		if (ui:isMouseReleased(KeyCode.MOUSE_LEFT)) then
			self.dragShape = false
			self:stopDrag()
		else
			local dx, dy = self:updateDrag(mx, my)
			self:onMove(dx, dy, mx, my)
		end
	end
	
	if (self.dragRotation) then 
		if (ui:isMouseReleased(KeyCode.MOUSE_RIGHT)) then
			self.dragRotation = false
			self:stopDrag()
		else
			local dx, dy = self:updateDrag(mx, my)
			self:onRotationChanged(dx, dy, mx, my)
		end
	end
end

function Poly:onPropertiesDraw(ui)
	
	self.drawNormals = ui:checkbox("Draw normals", self.drawNormals)
	self.drawBBOX = ui:checkbox("Draw BBOX", self.drawBBOX)
	
	if (ui:button("Upadte center position")) then 
		self.collisionShape:updateCenter(self.transform)
	end
	
	local x, y = self.transform:getPosition()
	local changed = false
	x, y, changed = ui:dragFloat2("Position", x, y)
	
	if (changed) then 
		self.transform:setPosition(x, y)
	end
	
	local rot = self.transform:getRotation()
	
	changed = false
	rot, changed = ui:dragFloat("Rotation", rot, 0.01)
	
	if (changed) then 
		self.transform:setRotation(rot)
	end
end

function Poly:redraw(list, isFilled, alpha)
	local shape = self.collisionShape
	local points = shape:getRotatedPoints(self.transform)
	local tx, ty = self.transform:getPosition()
	
	-- bounding box
	if (self.drawBBOX) then 
		local minX, minY, maxX, maxY = shape:getBoundingBox(self.transform)
		minX += tx
		minY += ty
		maxX += tx
		maxY += ty
		drawRect(list, minX, minY, maxX, maxY, isFilled, self.drawColor, alpha)
	end
	
	drawPoly(list, points, self.transform, isFilled, self.drawColor, alpha)	
	
	-- dragable vertices
	for i, pt in ipairs(points) do 
		local x = tx + pt.x
		local y = ty + pt.y
		
		list:addCircle(x, y, DRAG_POINT_RADIUS, 0x00ff00, 1)
		list:addText(x, y, 0xffffff, 1, tostring(i - 1))
	end
	
	-- normals
	if (self.drawNormals) then 
		local normals = shape:getRotatedNormals(self.transform)
		for i = 1, #points do 
			local x1 = tx + points[i].x
			local y1 = ty + points[i].y
			local x2 = x1 + normals[i].x * 16
			local y2 = y1 + normals[i].y * 16
			
			list:addLine(x1, y1, x2, y2, 0xffffff, alpha)
		end
	end
end