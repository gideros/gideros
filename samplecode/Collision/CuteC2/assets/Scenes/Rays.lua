RayScene = Core.class(BaseScene, function() return "Dark", true end)

function RayScene:init()
	self.ray = Ray.new(100, 400, 1, 0)
	self.rayCone = RayCone.new(4, 90, 100, 600)
	
	self.objects = {}
	self:createRandomShapes(self.objects, 128)
end

function RayScene:onDrawUI()
	self.ray:drawProperties(self.ui)
	self.rayCone:drawProperties(self.ui)
	BaseScene.onDrawUI(self)
end

function RayScene:testRay(list, ray, shape, displace, drawHitPoint)
	local hit, normalX, normalY, t = shape.collisionShape:rayTest(ray)
	
	if (hit) then 
		if (drawHitPoint) then
			local sx, sy = ray:getPosition()
			local tx, ty = ray:getTargetPosition()
			local hitX = sx + tx * t
			local hitY = sy + ty * t
			drawVec(list, hitX, hitY, normalX, normalY, 4, 20, 0xffffff, 1)
		end
		
		if (displace) then 
			ray:setLength(t - 0.0)
		end
	end
end

function RayScene:onDraw()
	local ui = self.ui	
	local list = ui:getForegroundDrawList()
	
	self.ray:draw(ui)
	self.rayCone:draw(ui)
	
	local ray1 = self.ray.collisionShape
	local ray2 = self.rayCone.collisionShape
	local rays = self.rayCone.rays
	
	self.rayCone:onMoveTarget()
	
	for i, shape in ipairs(self.objects) do 
		shape:draw(ui)
		
		self:testRay(list, ray1, shape, false, true)
		self:testRay(list, ray2, shape)
		
		for j, coneRay in ipairs(rays) do
			self:testRay(list, coneRay, shape, true)
		end
	end
	
end