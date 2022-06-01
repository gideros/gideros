CollisionsScene = Core.class(BaseScene, function() return "Dark", true end)

function CollisionsScene:init()
	
	self.objects = {}
	self:createRandomShapes(self.objects, 4)
end

function CollisionsScene:onDrawUI()
	local ui = self.ui
	local list = ui:getForegroundDrawList()
	
	for i,obj in ipairs(self.objects) do 
		obj:onDraw(ui, self.filledShapes, self.drawAlpha)
		
		for j, other in ipairs(self.objects) do 
			if (other ~= obj) then 
				local t = other:getType()
				
				local mainfold = CuteC2.collide(obj.collisionShape, other.collisionShape, obj.transform, other.transform)
				
				if (mainfold.count > 0) then 
					local nx = mainfold.normal.x
					local ny = mainfold.normal.y
					local d = mainfold.depths[1]
					local x = 0
					local y = 0
					
					if (t == CuteC2.TYPE_POLY) then 
						x, y = other.transform:getPosition()
					else
						x, y = other.collisionShape:getPosition()
					end
					
					local sx = x + nx * d
					local sy = y + ny * d
					local otherShape = other.collisionShape
					
					if (t == CuteC2.TYPE_POLY) then 
						other.transform:setPosition(sx, sy)
					else
						otherShape:setPosition(sx, sy)
					end
				end
			end
		end
	end
end