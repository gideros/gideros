Game = gideros.class(Sprite)

local FLOOR_HEIGHT = 258

function Game:init()
	self.slingx = 120 + 35
	self.slingy = 60 + 35

	local sprite = nil
	sprite = Bitmap.new(Texture.new("bg.png"))
	self:addChild(sprite)

	sprite = Bitmap.new(Texture.new("sling1.png"))
	sprite:setPosition(self.slingx + 28 - 35, self.slingy + 7 - 35)
	self:addChild(sprite)

	self.rubber1 = Shape.new()
	self:addChild(self.rubber1)

	self.targetLayer = Sprite.new()
	self:addChild(self.targetLayer)

	self.acornLayer = Sprite.new()	
	self:addChild(self.acornLayer)

	self.rubber2 = Shape.new()
	self:addChild(self.rubber2)

	sprite = Bitmap.new(Texture.new("sling2.png"))
	sprite:setPosition(self.slingx - 35, self.slingy - 35)
	self:addChild(sprite)

	sprite = Bitmap.new(Texture.new("fg.png"))
	sprite:setAnchorPoint(0, 1)
	sprite:setPosition(0, 320)
	self:addChild(sprite)

	self:createAcorn()

	self.actors = {}
	self.world = b2.World.new(0, 10, true)

	local groundBody = self.world:createBody{}
	local groundEdge = b2.EdgeShape.new(-960, FLOOR_HEIGHT, 960*2, FLOOR_HEIGHT)
	groundBody:createFixture{shape = groundEdge}

	self:createTargets()

	self.puf = {}

	self:addEventListener(Event.MOUSE_DOWN, self.onMouseDown, self)
	self:addEventListener(Event.MOUSE_MOVE, self.onMouseMove, self)
	self:addEventListener(Event.MOUSE_UP, self.onMouseUp, self)
	self:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
	self.world:addEventListener(Event.POST_SOLVE, self.onPostSolve, self)
end

function Game:createAcorn()
	self.acorn = Bitmap.new(Texture.new("acorn.png", true))
	self.acorn:setAnchorPoint(0.5, 0.5)
	self.acornLayer:addChild(self.acorn)
	self:setRubberPosition(self.slingx - 14, self.slingy)
end


function Game:onMouseDown(event)
	if self.acorn and self.acorn:hitTestPoint(event.x, event.y) then
		self.stretch = true
		self:setRubberPosition(event.x, event.y)
	end
end

function Game:onMouseMove(event)
	if self.stretch then
		self:setRubberPosition(event.x, event.y)
	end
end

function Game:destroyAcorn()
	local body = self.acornBody
	local sprite = body.sprite
	
	self.world:destroyBody(body)
	sprite:getParent():removeChild(sprite)

	local puf = Puf.new()
	puf:setScale(0.5)
	puf:setPosition(sprite:getPosition())
	self:addChild(puf)

	self.actors[sprite] = nil
	self.acornBody = nil
end

function Game:onMouseUp()
	if self.stretch then
		self.stretch = false
		self:fire()
		self:setRubberPosition(self.slingx, self.slingy)

		local timer = Timer.new(4000, 1)
		timer:addEventListener(Event.TIMER,
			function()
				self:destroyAcorn()
				self:createAcorn()
				GTween.new(self, 1, {x=0}, {delay = 1, ease = easing.inOutQuadratic})
				timer = nil
			end)
		timer:start()
	end
end

function Game:setRubberPosition(x, y)
	self.rubberx, self.rubbery = self:limitPosition(x - self.slingx, y - self.slingy)
	
	self.rubber1:clear()
	self.rubber1:setLineStyle(6, 0x301708)
	self.rubber1:beginPath()
	self.rubber1:moveTo(self.slingx - 35 + 57, self.slingy - 35 + 36)
	self.rubber1:lineTo(self.slingx + self.rubberx, self.slingy + self.rubbery)
	self.rubber1:endPath()

	self.rubber2:clear()
	self.rubber2:setLineStyle(6, 0x301708)
	self.rubber2:beginPath()
	self.rubber2:moveTo(self.slingx - 35 + 13, self.slingy - 35 + 34)
	self.rubber2:lineTo(self.slingx + self.rubberx, self.slingy + self.rubbery)
	self.rubber2:endPath()

	if self.acorn then
		local angle = math.atan2(self.rubbery, self.rubberx)
		local c = math.cos(angle)
		local s = math.sin(angle)
		
		self.acornx = self.slingx + self.rubberx - 15 * c
		self.acorny = self.slingy + self.rubbery - 15 * s
		
		self.acorn:setPosition(self.acornx, self.acorny)
	end
end

function Game:limitPosition(x, y)
	local length = math.max(math.sqrt(x * x + y * y), 1)
	x = x / length
	y = y / length	
	length = math.min(length, 100)
	
	return length * x, length * y
end

function Game:fire()
	local acornBodyDef = {
		type = b2.DYNAMIC_BODY,
        bullet = true,
		position = {x = self.acornx, y = self.acorny},
		linearVelocity = {x = -self.rubberx/5, y = -self.rubbery/5},
	}

	local circle = b2.CircleShape.new(0, 0, 15)
	local fixtureDef = {
		shape = circle,
		density = 0.8,
		restitution = 0.2,
		friction = 0.99,
	}
	
	self.acornBody = self.world:createBody(acornBodyDef)
	self.acornBody:createFixture(fixtureDef)

	self.acorn.body = self.acornBody
	self.acornBody.sprite = self.acorn
	self.actors[self.acorn] = true
	self.acorn = nil
end

function Game:createTarget(textureName,
							x, y,
							angle,
							isCircle,
							isStatic,
							isEnemy)
	local sprite = Bitmap.new(Texture.new(textureName, true))
	sprite:setAnchorPoint(0.5, 0.5)
	self.targetLayer:addChild(sprite)
	
	local width = sprite:getWidth()
	local height = sprite:getHeight()
	
	local bodyDef = {
		type = isStatic and b2.STATIC_BODY or b2.DYNAMIC_BODY,
		position = {x = x + width/2, y = y - height/2},
		angle = angle * math.pi / 180,		
	}
	
	local body = self.world:createBody(bodyDef)
	
	sprite.body = body
	body.sprite = sprite
	
	self.actors[sprite] = true
	
	local fixtureDef = {}
	if isCircle then
		fixtureDef.shape = b2.CircleShape.new(0, 0, width/2)
	else
		fixtureDef.shape = b2.PolygonShape.new()
		fixtureDef.shape:setAsBox(width/2, height/2)
	end	
	fixtureDef.density = 0.5
	local fixture = body:createFixture(fixtureDef)
	
	fixture.isEnemy = isEnemy
end


function Game:createTargets()
	
	-- First block
	self:createTarget("brick_2.png", 675, FLOOR_HEIGHT, 0, false, false, false)
	self:createTarget("brick_1.png", 741, FLOOR_HEIGHT, 0, false, false, false)
	self:createTarget("brick_1.png", 741, FLOOR_HEIGHT - 23, 0, false, false, false)
	self:createTarget("brick_3.png", 672, FLOOR_HEIGHT - 46, 0, false, false, false)
	self:createTarget("brick_1.png", 707, FLOOR_HEIGHT - 58, 0, false, false, false)
	self:createTarget("brick_1.png", 707, FLOOR_HEIGHT - 81, 0, false, false, false)

	self:createTarget("head_dog.png", 702, FLOOR_HEIGHT, 0, true, false, true)
    self:createTarget("head_cat.png", 680, FLOOR_HEIGHT - 58, 0, true, false, true)
    self:createTarget("head_dog.png", 740, FLOOR_HEIGHT - 58, 0, true, false, true)
 
	-- 2 bricks at the right of the first block
    self:createTarget("brick_2.png", 770, FLOOR_HEIGHT, 0, false, false, false)
    self:createTarget("brick_2.png", 770, FLOOR_HEIGHT + 46, 0, false, false, false)
 
	-- The dog between the blocks
    self:createTarget("head_dog.png", 830, FLOOR_HEIGHT, 0, true, false, true)

	-- Second block
    self:createTarget("brick_platform.png", 839, FLOOR_HEIGHT, 0, false, true, false)
    self:createTarget("brick_2.png", 854, FLOOR_HEIGHT - 28, 0, false, false, false)
    self:createTarget("brick_2.png", 854, FLOOR_HEIGHT - 28 - 46, 0, false, false, false)
    self:createTarget("head_cat.png", 881, FLOOR_HEIGHT - 28, 0, true, false, true)
    self:createTarget("brick_2.png", 909, FLOOR_HEIGHT - 28, 0, false, false, false)
    self:createTarget("brick_1.png", 909, FLOOR_HEIGHT - 28 - 46, 0, false, false, false)
    self:createTarget("brick_1.png", 909, FLOOR_HEIGHT - 28 - 46 - 23, 0, false, false, false)
    self:createTarget("brick_2.png", 882, FLOOR_HEIGHT - 108, 90, false, false, false)
end

function Game:onPostSolve(event)
	local isAEnemy = event.fixtureA.isEnemy
	local isBEnemy = event.fixtureB.isEnemy

	if isAEnemy or isBEnemy then
		if event.maxImpulse > 1.5 then
			if isAEnemy then
				local body = event.fixtureA:getBody()
				self.puf[body] = true
			end
			if isBEnemy then
				local body = event.fixtureB:getBody()
				self.puf[body] = true
			end
		end
	end
end

function Game:onEnterFrame()
	self.world:step(1/60, 8, 3)
	
	for sprite in pairs(self.actors) do
		local body = sprite.body
		sprite:setPosition(body:getPosition())
		sprite:setRotation(body:getAngle() * 180 / math.pi)
	end

	if self.acornBody then
		local x, y = self.acornBody:getPosition()
		
		if x > 240 then
			self:setX(math.max(-x + 240, -480))
		end
	end

	for body in pairs(self.puf) do
		local sprite = body.sprite
		sprite:getParent():removeChild(sprite)
		self.world:destroyBody(body)
		self.actors[body.sprite] = nil
		local puf = Puf.new()
		puf:setPosition(sprite:getPosition())
		self:addChild(puf)
		self.puf[body] = nil
	end
end