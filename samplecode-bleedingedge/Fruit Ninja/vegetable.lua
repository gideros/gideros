TexturedPolygon = Core.class(Shape)

function TexturedPolygon:init(texture, vertices)
	self.texture = texture
	self.vertices = vertices

	self:setFillStyle(Shape.TEXTURE, self.texture)
--	self:setFillStyle(Shape.SOLID, 0xff0000)
	self:beginPath()
	for i=1,#self.vertices do
		if i == 1 then
			self:moveTo(self.vertices[i].x, self.vertices[i].y)
		else
			self:lineTo(self.vertices[i].x, self.vertices[i].y)
		end
	end
	self:closePath()
	self:endPath()
end

function TexturedPolygon:split(x1, y1, x2, y2)
	if rayPolygonIntersection(self.vertices, x1, y1, x2 - x1, y2 - y1) == false then
		return false
	end
	
	local result, vertices1, vertices2 = splitPolygon(self.vertices, x1, y1, x2, y2)

	if result then
		return true, TexturedPolygon.new(self.texture, vertices1), TexturedPolygon.new(self.texture, vertices2)
	end
		
	return false
end

Vegetable = Core.class(Sprite)

function Vegetable:init(texture, vertices)
	self.polygon = TexturedPolygon.new(texture, vertices)
	self:addChild(self.polygon)
	self.timer = os.timer()
	self.npieces = 1
end

function Vegetable:split(x1, y1, x2, y2)
	local timer = os.timer()
	if timer - self.timer < 0.2 then
		return
	end

	local polygon = self.polygon

	x1, y1 = polygon:globalToLocal(x1, y1)
	x2, y2 = polygon:globalToLocal(x2, y2)
	
	local result, polygon1, polygon2 = polygon:split(x1, y1, x2, y2)
			
	if result then			
		self:removeChild(polygon)
		self:addChild(polygon1)
		self:addChild(polygon2)
		
		local area1 = polygonArea(polygon1.vertices)
		local area2 = polygonArea(polygon2.vertices)
		
		local dx = x2 - x1
		local dy = y2 - y1
		local len = math.sqrt(dx * dx + dy * dy)
		dx = (dx / len) * 40
		dy = (dy / len) * 40

		if area1 < area2 then
			self.polygon = polygon2
			polygon1:setColorTransform(1, 0, 0)
			GTween.new(polygon1, 0.6, {x = dx, y = dy}, {ease = easing.outExponential})
			GTween.new(polygon1, 0.3, {alpha = 0}, {delay = 0.2})
		else
			self.polygon = polygon1
			polygon2:setColorTransform(1, 0, 0)
			GTween.new(polygon2, 0.6, {x = dx, y = dy}, {ease = easing.outExponential})
			GTween.new(polygon2, 0.3, {alpha = 0}, {delay = 0.2})
		end
		
		self.npieces = self.npieces + 1
		
		self.timer = timer
	end
end
