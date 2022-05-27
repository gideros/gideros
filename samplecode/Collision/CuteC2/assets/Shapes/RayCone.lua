--!NOEXEC
RayCone = Core.class(Ray, function(amount, fov, sx, sy) return sx, sy, 1, 0 end)

function RayCone:init(amount, fov, sx, sy)
	self.name = "RayCone"
	
	self.rays = {}
	
	self.fov = fov
	self.amount = amount
	self.total = amount * 2
	self.step = math.rad( fov / (self.total + 1) )
	local len = self.collisionShape:getLength()
	
	for i = 1, amount do 
		local ang = i * self.step
		local r1 = CuteC2.rayFromRotation(sx, sy, ang, len)
		self.rays[i] = r1
		
		local r2 = CuteC2.rayFromRotation(sx, sy, -ang, len)
		self.rays[self.total - i + 1] = r2
	end
end

function RayCone:redraw(list, isFilled, alpha)
	Ray.redraw(self, list, isFilled, alpha)
	
	for i, ray in ipairs(self.rays) do 
		local x1, y1 = ray:getPosition()
		local x2, y2 = ray:getFacePosition()
		
		list:addLine(x1, y1, x2, y2, self.drawColor, alpha)
	end
end

function RayCone:onPropertiesDraw(ui)
	Ray.onPropertiesDraw(self, ui)
	local changed = false
	self.fov, changed = ui:dragFloat("FOV", self.fov, 1, 0, 360)
	
	if (changed) then 
		self.step = math.rad( self.fov / (self.total + 1) )
		self:onMoveTarget()
	end
end

function RayCone:onMove(dx, dy, mx, my)
	for i, ray in ipairs(self.rays) do 
		ray:move(dx, dy)
	end
end

function RayCone:onMoveTarget(dx, dy, x, y)
	local d = self.collisionShape:getDirection()
	local l = self.collisionShape:getLength()
	
	for i = 1, self.amount do
		local r1 = self.rays[i]
		local r2 = self.rays[self.total - i + 1]
		
		r1:setDirection(d - i * self.step)
		r2:setDirection(d + i * self.step)
		r1:setLength(l)
		r2:setLength(l)
	end
end