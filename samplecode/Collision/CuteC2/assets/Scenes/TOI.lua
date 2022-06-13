local PI = math.pi

TOIScene = Core.class(BaseScene, function() return "Dark", true end)

function TOIScene:init()
	self.mainPoint = Circle.new(100, 400, 16)
	self.mainPoint.name = "Main"
	self.dstPoint = Circle.new(400, 400, 16)
	self.dstPoint.name = "Destenation"
	
	self.objects = {}
	self:createRandomShapes(self.objects, 8)
end

function TOIScene:onDraw()
	local ui = self.ui	
	local list = ui:getForegroundDrawList()
	
	self.mainPoint:draw(ui)
	self.dstPoint:draw(ui)
	
	local mainShape = self.mainPoint.collisionShape
	local dstShape = self.dstPoint.collisionShape
	
	local mainRadius = mainShape:getRadius()
	
	local x1, y1 = mainShape:getPosition()
	local x2, y2 = dstShape:getPosition()
	
	-- velocity vector
	local vx = x2 - x1
	local vy = y2 - y1
	
	for i, shape in ipairs(self.objects) do 
		shape:draw(ui)
		local hit, toi, nx, ny, px, py, iterations = CuteC2.TOI(mainShape, vx, vy, shape.collisionShape, 0, 0, true, nil, shape.transform)
		
		if (hit) then 
			-- draw normal
			list:addLine(px, py, px - nx * 20, py - ny * 20, 0x00ff00, 1)
			
			-- draw collision point
			list:addCircleFilled(px, py, 4, 0x00ff00, 1)
			
			-- draw original shape at resolved position
			list:addCircleFilled(px - nx * mainRadius, py - ny * mainRadius, mainRadius, 0xff0000, 0.5, 32)
		end
	end
	
	-- angle between circles
	local ang = math.atan2(-vx, vy) - PI
	
	-- limit velocity vector
	local nvx, nvy = math.normalize(vx, vy)	
	nvx *= mainRadius
	nvy *= mainRadius
	
	-- draw arrow that points to destenation point
	local arrowAng = ang + PI / 4
	local arrowX = math.cos(arrowAng) * mainRadius * 0.5
	local arrowY = math.sin(arrowAng) * mainRadius * 0.5
	
	list:addLine(x2, y2, x2 - nvx * 2,y2 - nvy * 2, 0x00ff00, 1)
	list:addLine(x2, y2, x2 + arrowX,y2 + arrowY, 0x00ff00, 1)
	list:addLine(x2, y2, x2 - arrowY,y2 + arrowX, 0x00ff00, 1)
	
	-- draw "tunnel" between two circles (capsule)
	list:pathArcTo(x1,y1, mainRadius, ang, ang + PI)
	list:pathArcTo(x2,y2, mainRadius, ang - PI, ang)
	list:pathStroke(0x00ff00, 1, true)
	
	-- make sure that second point have same radius
	dstShape:setRadius(mainRadius)
end