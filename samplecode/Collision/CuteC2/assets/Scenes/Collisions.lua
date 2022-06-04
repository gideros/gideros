CollisionsScene = Core.class(BaseScene, function() return "Dark", true end)

function CollisionsScene:init()
	self.objects = {}
	self:createRandomShapes(self.objects, 8)
end

function CollisionsScene:onDraw()
	local ui = self.ui
	
	for i,obj in ipairs(self.objects) do 
		obj:draw(ui)
		
		for j, other in ipairs(self.objects) do 
			if (other ~= obj) then 
				
				local mainfold = CuteC2.collide(obj.collisionShape, other.collisionShape)
				
				if (mainfold.count > 0) then 
					local nx = mainfold.normal.x
					local ny = mainfold.normal.y
					local d = mainfold.depths[1]
					local x, y = other.collisionShape:getPosition()
					
					local sx = x + nx * d
					local sy = y + ny * d
					other.collisionShape:setPosition(sx, sy)
				end
			end
		end
	end
end