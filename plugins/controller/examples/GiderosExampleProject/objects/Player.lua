Player = Core.class(Sprite)

function Player:init(level, x, y)
	self.level = level
	self.world = level.world
	
	self.angle = -math.pi/2
	self.speed = 10
	self.scale = 0.3
	self.wheelCount = 0
	self.curWheel = 1
	self.power = 0
	
	self:setScale(self.scale)
	
	self.tank = Sprite.new()
	self:addChild(self.tank)
	
	self.image = Bitmap.new(Texture.new("images/tank.png", true))
	self.image:setAnchorPoint(0.5, 0.5)
	self.tank:addChild(self.image)
	
	self:createWheels()
	
	--create box2d physical object    
	self.body = self.world:createBody{type = b2.DYNAMIC_BODY}    
	self.body:setPosition(x, y)      
	self.body:setLinearDamping(30)
	self.body:setAngle(self.angle)
	
	--create first shape    
	local poly = b2.PolygonShape.new()    
	poly:setAsBox(self:getWidth()/2, self:getHeight()/2)    
	self.body:createFixture{shape = poly, density = 1.0, friction = 0.1, restitution = 0.8}
	
	self.level.bodies:addChild(self)
	
	self.turret = Turret.new(self)
	self.turret:setPosition(-20, 0)
	self:addChild(self.turret)
end

function Player:setAngle(angle)
	self.turret:setAngle(angle - self.body:getAngle())
end

function Player:setMovement(angle, power)
	self.angle = angle
	self.turret:setAngle(self.turret:getAngle() - (angle-self.body:getAngle()))
	self.body:setAngle(angle)
	self.power = power
end

function Player:stopMovement()
	self.power = 0
end

function Player:step()
	if self.power > 0 then
		local koef = self.speed*self.power
		local dist = 100
		local xVect = -dist * math.sin(self.angle-math.pi/2)
		local yVect = dist * math.cos(self.angle-math.pi/2)
		local x, y = self.body:getPosition()
		self.body:applyForce(xVect*koef, yVect*koef, x, y)
		self.wheelCount = self.wheelCount + 1
		if self.wheelCount > self.speed*self.power then
			self.curWheel = self.curWheel + 1
			if self.curWheel > 12 then self.curWheel = 1 end
			for i = 1, 4 do
				self.wheels[i]:setTexture(self.textures[self.curWheel])
			end
		end
	else
		self.body:setLinearVelocity(0, 0)
	end
	self.body:setAngularVelocity(0)
end

function Player:createWheels()
	local img = 11
	self.textures = {}
	local num
	for i = 0, img do
		num = i
		if i < 10 then num = "0"..i end
		self.textures[#self.textures+1] = Texture.new("images/wheel00"..num..".png", true)
	end
	self.wheels = {}
	local b
	for i = 1, 4 do
		b = Bitmap.new(self.textures[1])
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
		self.tank:addChildAt(b, 1)
		self.wheels[#self.wheels+1] = b
	end
end