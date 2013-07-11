LevelMeter = Core.class(Sprite)

function LevelMeter:init()
	local bars = {}
	self.bars = bars

	for x=10,300,10 do
		local shape = Shape.new()
		shape:setFillStyle(Shape.SOLID, 0xffffff)
		shape:beginPath()
		shape:moveTo(x, 0)
		shape:lineTo(x + 8, 0)
		shape:lineTo(x + 8, 50)
		shape:lineTo(x, 50)
		shape:closePath()
		shape:endPath()
		self:addChild(shape)		
		bars[#bars+1] = shape		
	end
	
	self:setLevel(0)
end


function LevelMeter:setLevel(level)
	local bars = self.bars
	
	for i=1,#bars do
		local t = i / #bars
		if t <= level then
			if t < 0.5 then
				bars[i]:setColorTransform(0, 1, 0)
			elseif t < 0.75 then
				bars[i]:setColorTransform(1, 1, 0)
			else
				bars[i]:setColorTransform(1, 0, 0)
			end
		else
			bars[i]:setColorTransform(0.7, 0.7, 0.7)
		end
	end
end
