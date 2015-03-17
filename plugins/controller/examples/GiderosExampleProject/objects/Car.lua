Car = Core.class(Sprite)

function Car:init()
	self.image = Bitmap.new(Texture.new("images/tank.png", true))
	self.image:setAnchorPoint(0.5, 0.5)
	self:addChild(self.image)
	local turret = Bitmap.new(Texture.new("images/turret.png", true))
	turret:setPosition(-20, 0)
	turret:setAnchorPoint(0.3, 0.5)
	self:addChild(turret)
	
	self:createWheels()
end

function Car:createWheels()
	local texture = Texture.new("images/wheel0000.png", true)
	local b
	for i = 1, 4 do
		b = Bitmap.new(texture)
		b:setAnchorPoint(0.5, 0.5)
		if i == 1 then
			b:setPosition(-self.image:getWidth()/2 + b:getWidth()/1.5, -self.image:getHeight()/2 + b:getHeight()/1.35)
		elseif i == 2 then
			b:setPosition(self.image:getWidth()/2 - b:getWidth()/1.5, -self.image:getHeight()/2 + b:getHeight()/1.35)
		elseif i == 3 then
			b:setPosition(-self.image:getWidth()/2 + b:getWidth()/1.5, self.image:getHeight()/2 - b:getHeight()/1.35)
		elseif i == 4 then
			b:setPosition(self.image:getWidth()/2 - b:getWidth()/1.5, self.image:getHeight()/2 - b:getHeight()/1.35)
		end
		self:addChildAt(b, 1)
	end
end