GJKScene = Core.class(BaseScene, function() return "Dark", true end)

function GJKScene:init()
	
	self.objects = {}
	self:createRandomShapes(self.objects, 8)
	
	self.minDistance = 150
end

function GJKScene:onDrawUI()
	self.minDistance = self.ui:dragFloat("Min distance", self.minDistance, nil, 0, 1000)
	BaseScene.onDrawUI(self)
end

function GJKScene:onDraw()
	local ui = self.ui
	local list = ui:getForegroundDrawList()
	
	for i,shape in ipairs(self.objects) do 
		shape:draw(ui)
		
		for j, other in ipairs(self.objects) do 
			if (shape ~= other) then 
				local distance, aX, aY, bX, bY, iterations = CuteC2.GJK(shape.collisionShape, other.collisionShape, shape.transform, other.transform)
				
				if (distance < self.minDistance) then
					-- draw a line between 2 points
					list:addCircleFilled(aX, aY, 6, 0x00ff00, 1)
					list:addLine(aX, aY, bX, bY, 0x00ff00, 1)
					list:addCircleFilled(bX, bY, 6, 0x00ff00, 1)
					
					-- midpoint between 2 points
					local dx = (bX - aX) / 2
					local dy = (bY - aY) / 2
					
					-- add text with shadow
					local text = ("%.3f"):format(distance)
					list:addText(aX + dx, aY + dy + 2, 0, 1, text)
					list:addText(aX + dx, aY + dy, 0xffffff, 1, text)
				end
			end
		end
	end
end