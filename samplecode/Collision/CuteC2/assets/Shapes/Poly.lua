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
	self.collisionShape:setPosition(x, y)
	self.dragVertex = 0
	self.drawNormals = false
	self.drawBBOX = false
	self.verticesInGlobalSpace = true
	self.verticesRotationRelative = true
	
	-- used to test if mouse is clicked close to the shape
	-- but not inside
	self.mousePos = CuteC2.circle(0,0,4)
end

function Poly:pointHitTest(x, y)
	local points = self.collisionShape:getRotatedPoints(true)
	
	for i, pt in ipairs(points) do
		if (circleHitTest(pt.x, pt.y, DRAG_POINT_RADIUS, x, y)) then
			return i, pt
		end
	end
end

function Poly:onVertexStartMove(mx, my)
end

function Poly:onStartRotate(mx, my)
	local x, y = self.collisionShape:getPosition()
	local ang = math.atan2(my - y, mx - x)
	self.clickAng = ang
	self.prevAng = ang
end

function Poly:onRotationChanged(dx, dy, mx, my)
	local x, y = self.collisionShape:getPosition()
	local currentAng = math.atan2(my - y, mx - x)
	self.collisionShape:rotate(currentAng - self.prevAng)
	self.prevAng = currentAng
end

-- update vertex position with respect to shape rotation
function Poly:onVertexMove(ind, dx, dy, mx, my)
	local vx, vy = self.collisionShape:getVertex(ind)
	local rot = self.collisionShape:getRotation()
	local len = math.length(dx, dy)
	local ang = math.atan2(dy, dx)
	
	ang -= rot
	vx += math.cos(ang) * len
	vy += math.sin(ang) * len
	
	self.collisionShape:setVertex(ind, vx, vy)
end

function Poly:onMove(dx, dy, mx, my)
	self.collisionShape:move(dx, dy)
end

function Poly:onStartResize(mx, my)
	local x, y = self.collisionShape:getPosition()
	self.dragDistance = math.distance(x, y, mx, my)
end

function Poly:onSizeChanged(dx, dy, mx, my)
	local x, y = self.collisionShape:getPosition()
	local d = math.distance(x, y, mx, my)
	local delta = d - self.dragDistance
	if (math.abs(delta) > 0) then 
		self.collisionShape:inflate(self.dragDistance - d)
	end
	self.dragDistance = d
end

function Poly:isAllowedToDrag()
	return not(self.dragSize or self.dragRotation or self.dragVertex > 0)
end

function Poly:isAllowedToDragVertex()
	return not(self.dragSize or self.dragShape or self.dragRotation)
end

function Poly:isAllowedToResize()
	return not(self.dragShape or self.dragRotation or self.dragVertex > 0)
end

function Poly:isAllowedToRotate()
	return not(self.dragSize or self.dragShape or self.dragVertex > 0)
end

function Poly:isAllowedToDeleteVertex()
	return not(self.dragSize or self.dragShape or self.dragRotation or self.dragVertex > 0)
end

function Poly:updateDragAndDrop(ui)	
	local mx, my = ui:getMousePos()
	local tx, ty = self.collisionShape:getPosition()
	
	-- Drag & drop shape OR vertex
	if (ui:isMouseClicked(KeyCode.MOUSE_LEFT)) then 
		local ind = self:pointHitTest(mx, my)
		
		-- clicked on vertex
		if (ind and self:isAllowedToDragVertex()) then
			self:startDrag(mx, my)
			self:onVertexStartMove(mx, my, ind)
			self.dragVertex = ind
		-- clicked on shape body OR its position origin point
		elseif (self:isAllowedToDrag() and (self.collisionShape:hitTest(mx, my) or circleHitTest(tx, ty, DRAG_POINT_RADIUS, mx, my))) then 
			self:startDrag(mx, my)
			self:onStartMove(mx, my)
			self.dragShape = true
		end
	-- Scale the shape
	elseif (ui:isMouseClicked(KeyCode.MOUSE_RIGHT)) then 
		if (self.collisionShape:hitTest(mx, my) and self:isAllowedToResize()) then 
			self:startDrag(mx, my)
			self:onStartResize(mx, my)
			self.dragSize = true
		end
	-- Rotate the shape OR add/remove vertex
	elseif (ui:isMouseClicked(KeyCode.MOUSE_MIDDLE)) then 
		local ind = self:pointHitTest(mx, my)
		
		-- clicked on vertex
		if (ind) then 
			if (self:isAllowedToDeleteVertex()) then
				self.collisionShape:removeVertex(ind)
			end
		elseif (self.collisionShape:hitTest(mx, my)) then 
			if (self:isAllowedToRotate()) then 
				self:startDrag(mx, my)
				self:onStartRotate(mx, my)
				self.dragRotation = true
			end
		else
			-- using fake shape to calculate closest point with GJK
			self.mousePos:setPosition(mx, my)
			
			-- calculate distance to closest point
			local d, ax, ay = CuteC2.GJK(self.collisionShape, self.mousePos)
			
			if (d < 100) then 
				-- of there is more than 8 vertices the application will throw an error
				-- so just shake the shape a little :D
				if (self.collisionShape:getVertexCount() + 1 > CuteC2.MAX_POLYGON_VERTS) then 
					self:shake()
				else
					-- get points in GLOBAL space, because of CuteC2.GJK
					-- that retruns a point in GLOBAL space aswell
					local points = self.collisionShape:getRotatedPoints(true)
					local count = #points
					
					-- find closest line segment
					for i, pt in ipairs(points) do
						-- if too close to vertex point, break
						if (circleHitTest(pt.x, pt.y, DRAG_POINT_RADIUS, ax, ay)) then 
							break
						end
						
						-- loop point index
						local nextI = (i+1) % (count+1)
						
						if (nextI == 0) then 
							nextI = 1
						end
						
						local nextPoint = points[nextI]
						-- check if current line segment "contains" closest point given by GJK algorithm
						if (isBetween(pt.x, pt.y, nextPoint.x, nextPoint.y, ax, ay)) then 
							self.collisionShape:insertVertex(mx - tx, my - ty, false, i)
							break
						end
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
	
	if (self.dragSize) then 
		if (ui:isMouseReleased(KeyCode.MOUSE_RIGHT)) then
			self.dragSize = false
			self:stopDrag()
		else
			local dx, dy = self:updateDrag(mx, my)
			self:onSizeChanged(dx, dy, mx, my)
		end
	end
	
	if (self.dragRotation) then 
		if (ui:isMouseReleased(KeyCode.MOUSE_MIDDLE)) then
			self.dragRotation = false
			self:stopDrag()
		else
			local dx, dy = self:updateDrag(mx, my)
			self:onRotationChanged(dx, dy, mx, my)
		end
	end
end

function Poly:onPropertiesDraw(ui)
	local shape = self.collisionShape
	
	ui:sameLine()
	self.drawNormals = ui:checkbox("Draw normals", self.drawNormals)
	ui:sameLine()
	self.drawBBOX = ui:checkbox("Draw BBOX", self.drawBBOX)
	
	if (ui:button("Update center position")) then 
		shape:updateCenter()
	end
	
	local x, y = shape:getPosition()
	local changed = false
	x, y, changed = ui:dragFloat2("Position", x, y)
	
	if (changed) then 
		shape:setPosition(x, y)
	end
	
	local rot = shape:getRotation()
	
	changed = false
	rot, changed = ui:dragFloat("Rotation", rot, 0.01)
	
	if (changed) then 
		shape:setRotation(rot)
	end
	
	if (ui:treeNode("Vertices")) then 
		self.verticesInGlobalSpace = ui:checkbox("Global space", self.verticesInGlobalSpace)
		self.verticesRotationRelative = ui:checkbox("Rotation relative", self.verticesRotationRelative)
		
		local count = shape:getVertexCount()
		local deleteIndex = 0
		
		for i = 1, count do 
			if (ui:button("Delete##V"..i)) then 
				if (count - 1 > 2) then
					deleteIndex = i
				else
					self:shake()
				end
			end
			
			ui:sameLine()
			
			local vx, vy = shape:getVertex(i, self.verticesInGlobalSpace)
			local newX, newY, moved = ui:dragFloat2("#"..i, vx, vy)
			
			if (moved) then 
				if (self.verticesRotationRelative) then 
					local deltaX = newX - vx
					local deltaY = newY - vy
					
					self:onVertexMove(i, deltaX, deltaY)
				else
					shape:setVertex(i, newX, newY, self.verticesInGlobalSpace)
				end
			end
		end
		
		if (deleteIndex > 0) then 
			shape:removeVertex(deleteIndex)
		end
		
		ui:treePop()
	end
end

function Poly:redraw(list, alpha)
	local shape = self.collisionShape
	local points = shape:getRotatedPoints(true)
	local px, py = shape:getPosition()
	
	-- bounding box
	if (self.drawBBOX) then 
		local minX, minY, maxX, maxY = shape:getBoundingBox(true)
		drawRect(list, minX, minY, maxX, maxY, self.drawColor, 0.2)
	end
	
	drawPoly(list, px, py, points, self.drawColor, alpha)
	
	-- dragable vertices
	if (DRAG_POINT_VISIBLE) then
		for i, pt in ipairs(points) do 
			local x = pt.x
			local y = pt.y
			
			list:addCircle(x, y, DRAG_POINT_RADIUS, 0x00ff00, 1)
			list:addText(x, y, 0xffffff, 1, tostring(i))
		end
	end
	
	-- normals
	if (self.drawNormals) then 
		local normals = shape:getRotatedNormals()
		
		for i = 1, #points do 
			local x1 = points[i].x
			local y1 = points[i].y
			local x2 = x1 + normals[i].x * 16
			local y2 = y1 + normals[i].y * 16
			
			list:addLine(x1, y1, x2, y2, 0xffffff, alpha)
		end
	end
end