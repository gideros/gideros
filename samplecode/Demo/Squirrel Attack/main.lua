require "box2d"

local FLOOR_HEIGHT = 258

-- add sprites that will not be part of the physics simulation
local sprite = nil
sprite = Bitmap.new(Texture.new("bg.png"))
stage:addChild(sprite)

sprite = Bitmap.new(Texture.new("catapult_base_2.png"))
sprite:setPosition(181, FLOOR_HEIGHT)
sprite:setAnchorPoint(0, 1)
stage:addChild(sprite)

sprite = Bitmap.new(Texture.new("squirrel_1.png"))
sprite:setPosition(11, FLOOR_HEIGHT)
sprite:setAnchorPoint(0, 1)
stage:addChild(sprite)

local catapultLayer = Sprite.new()
stage:addChild(catapultLayer)

local targetLayer = Sprite.new()
stage:addChild(targetLayer)

local acornLayer = Sprite.new()
stage:addChild(acornLayer)

sprite = Bitmap.new(Texture.new("catapult_base_1.png"))
sprite:setPosition(181, FLOOR_HEIGHT)
sprite:setAnchorPoint(0, 1)
stage:addChild(sprite)

sprite = Bitmap.new(Texture.new("squirrel_2.png"))
sprite:setPosition(240, FLOOR_HEIGHT)
sprite:setAnchorPoint(0, 1)
stage:addChild(sprite)

sprite = Bitmap.new(Texture.new("fg.png"))
sprite:setAnchorPoint(0, 1)
sprite:setPosition(0, 320)
stage:addChild(sprite)

-- create world 
world = b2.World.new(0, 10, true)

-- create the boundaries
local groundBody = world:createBody{}

local groundEdge = b2.EdgeShape.new()
groundEdge:set(0, FLOOR_HEIGHT, 960, FLOOR_HEIGHT)	-- bottom
groundBody:createFixture{shape = groundEdge}
groundEdge:set(0, 0, 960, 0)						-- top
groundBody:createFixture{shape = groundEdge}
groundEdge:set(0, 0, 0, 320)						-- left
groundBody:createFixture{shape = groundEdge}
--groundEdge:set(960, 0, 960, 320)					-- right
--groundBody:createFixture{shape = groundEdge}

actors = {}

-- Create the catapult's arm

local arm = Bitmap.new(Texture.new("catapult_arm.png", true))
arm:setAnchorPoint(0.5, 0.5)
catapultLayer:addChild(arm)

local armBodyDef = {
	type = b2.DYNAMIC_BODY,
	linearDamping = 1,
	angularDamping = 1,
	position = {x=230,y=167},
}

local armBody = world:createBody(armBodyDef)
arm.body = armBody

local armBox = b2.PolygonShape.new()
armBox:setAsBox(11, 91)
armBody:createFixture{shape = armBox, density = 0.3}

actors[arm] = true

-- create arm joint
local armJointDef = b2.createRevoluteJointDef(groundBody, armBody, 233, FLOOR_HEIGHT)
armJointDef.enableMotor = true
armJointDef.enableLimit = true
armJointDef.motorSpeed  = 1260
armJointDef.lowerAngle = -72 * math.pi / 180
armJointDef.upperAngle = -9 * math.pi / 180
armJointDef.maxMotorTorque = 500
local armJoint = world:createJoint(armJointDef)

local releasingArm = false

local mouseJoint = nil
local function onMouseDown(event)
	if mouseJoint == nil then
		local mouseJointDef = b2.createMouseJointDef(groundBody, armBody, event.x, event.y, 2000)
		mouseJoint = world:createJoint(mouseJointDef)
	end	
end

local function onMouseMove(event)
	if mouseJoint ~= nil then
		mouseJoint:setTarget(event.x, event.y)
	end	
end

local function onMouseUp(event)
	if mouseJoint ~= nil then
		world:destroyJoint(mouseJoint)
		mouseJoint = nil		
		if armJoint:getJointAngle() < (-15 * math.pi / 180) then
			releasingArm = true
		end
	end
end

stage:addEventListener(Event.MOUSE_DOWN, onMouseDown)
stage:addEventListener(Event.MOUSE_MOVE, onMouseMove)
stage:addEventListener(Event.MOUSE_UP, onMouseUp)

local bullets = {}

-- create bullets
local function createBullets(count)
	if count > 1 then
		delta = (165 - 62 - 30) / (count - 1)
	else
		delta = 0
	end

	local texture = Texture.new("acorn.png", true)

	local bulletBodyDef = {
		type = b2.DYNAMIC_BODY,
        bullet = true,
		position = {y = FLOOR_HEIGHT - 15},
	}
	
	local circle = b2.CircleShape.new(0, 0, 15)
	local fixtureDef = {
		shape = circle,
		density = 0.8,
		restitution = 0.2,
		friction = 0.99,
	}

	local pos = 62
	for i=1,count do	
		bulletBodyDef.position.x = pos
		
		local bullet = world:createBody(bulletBodyDef)
		bullet:setActive(false)
		
		bullet:createFixture(fixtureDef)
					
		local sprite = Bitmap.new(texture)
		sprite:setAnchorPoint(0.5, 0.5)
		acornLayer:addChild(sprite)
		
		sprite.body = bullet
		bullet.sprite = sprite
		
		actors[sprite] = true
		
		bullets[#bullets + 1] = bullet
	
		pos = pos + delta
	end
end

local currentBullet = 1
local bulletBody = nil
local bulletJoint = nil

local function attachBullet()
	if currentBullet <= #bullets then
		bulletBody = bullets[currentBullet]
		currentBullet = currentBullet + 1
		bulletBody:setPosition(230, FLOOR_HEIGHT - 155)
		bulletBody:setActive(true)
		
		local weldJointDef = b2.createWeldJointDef(bulletBody, armBody, 230, FLOOR_HEIGHT - 155)
		weldJointDef.collideConntected = false
		
		bulletJoint = world:createJoint(weldJointDef)
		
		return true
	end
	
	return false
end

local targets, enemies

local function createTarget(textureName,
							x, y,
							angle,
							isCircle,
							isStatic,
							isEnemy)
	local sprite = Bitmap.new(Texture.new(textureName, true))
	sprite:setAnchorPoint(0.5, 0.5)
	targetLayer:addChild(sprite)
	
	local width = sprite:getWidth()
	local height = sprite:getHeight()
	
	local bodyDef = {
		type = isStatic and b2.STATIC_BODY or b2.DYNAMIC_BODY,
		position = {x = x + width/2, y = y - height/2},
		angle = angle * math.pi / 180,		
	}
	
	local body = world:createBody(bodyDef)
	
	sprite.body = body
	body.sprite = sprite
	
	actors[sprite] = true
	
	local fixtureDef = {}
	if isCircle then
		fixtureDef.shape = b2.CircleShape.new(0, 0, width/2)
	else
		fixtureDef.shape = b2.PolygonShape.new()
		fixtureDef.shape:setAsBox(width/2, height/2)
	end	
	fixtureDef.density = 0.5
	local fixture = body:createFixture(fixtureDef)
	
	if isEnemy then
		fixture.isEnemy = isEnemy
		enemies[#enemies + 1] = body
	end
	
	targets[#targets + 1] = body	
end

local function createTargets()
	targets  = {}
	enemies = {}
	
	-- First block
	createTarget("brick_2.png", 675, FLOOR_HEIGHT, 0, false, false, false)
	createTarget("brick_1.png", 741, FLOOR_HEIGHT, 0, false, false, false)
	createTarget("brick_1.png", 741, FLOOR_HEIGHT - 23, 0, false, false, false)
	createTarget("brick_3.png", 672, FLOOR_HEIGHT - 46, 0, false, false, false)
	createTarget("brick_1.png", 707, FLOOR_HEIGHT - 58, 0, false, false, false)
	createTarget("brick_1.png", 707, FLOOR_HEIGHT - 81, 0, false, false, false)

	createTarget("head_dog.png", 702, FLOOR_HEIGHT, 0, true, false, true)
    createTarget("head_cat.png", 680, FLOOR_HEIGHT - 58, 0, true, false, true)
    createTarget("head_dog.png", 740, FLOOR_HEIGHT - 58, 0, true, false, true)
 
	-- 2 bricks at the right of the first block
    createTarget("brick_2.png", 770, FLOOR_HEIGHT, 0, false, false, false)
    createTarget("brick_2.png", 770, FLOOR_HEIGHT + 46, 0, false, false, false)
 
	-- The dog between the blocks
    createTarget("head_dog.png", 830, FLOOR_HEIGHT, 0, true, false, true)

	-- Second block
    createTarget("brick_platform.png", 839, FLOOR_HEIGHT, 0, false, true, false)
    createTarget("brick_2.png", 854, FLOOR_HEIGHT - 28, 0, false, false, false)
    createTarget("brick_2.png", 854, FLOOR_HEIGHT - 28 - 46, 0, false, false, false)
    createTarget("head_cat.png", 881, FLOOR_HEIGHT - 28, 0, true, false, true)
    createTarget("brick_2.png", 909, FLOOR_HEIGHT - 28, 0, false, false, false)
    createTarget("brick_1.png", 909, FLOOR_HEIGHT - 28 - 46, 0, false, false, false)
    createTarget("brick_1.png", 909, FLOOR_HEIGHT - 28 - 46 - 23, 0, false, false, false)
    createTarget("brick_2.png", 882, FLOOR_HEIGHT - 108, 90, false, false, false)
end


local puf = {}

local function onPostSolve(event)
	local isAEnemy = event.fixtureA.isEnemy
	local isBEnemy = event.fixtureB.isEnemy

	if isAEnemy or isBEnemy then
		if event.maxImpulse > 1.5 then
			if isAEnemy then
				local body = event.fixtureA:getBody()
				puf[body] = true
			end
			if isBEnemy then
				local body = event.fixtureB:getBody()
				puf[body] = true
			end
		end
	end
end

world:addEventListener(Event.POST_SOLVE, onPostSolve)


local function poofAnimation(x, y)
	local poof = Bitmap.new(Texture.new("poof.png"))
	poof:setPosition(x, y)
	poof:setAnchorPoint(0.5, 0.5)
	poof:setScale(0.5)
	
	local tween1 = GTween.new(	poof,
								1,
								{alpha = 0, scaleX = 1, scaleY = 1},
								{onComplete = 
									function()
										poof:getParent():removeChild(poof)
									end})
	
	stage:addChild(poof)
end

local function onEnterFrame()
	world:step(1/60, 8, 3)
	
	for sprite in pairs(actors) do
		local body = sprite.body
		
		sprite:setPosition(body:getPosition())
		sprite:setRotation(body:getAngle() * 180 / math.pi)
	end

	if releasingArm and bulletJoint then
		if armJoint:getJointAngle() > (-20 * math.pi / 180) then
			releasingArm = false
			world:destroyJoint(bulletJoint)
			bulletJoint = nil
						
			local timer = Timer.new(5000, 1)
			timer:addEventListener(Event.TIMER,
				function()
					if attachBullet() then
						GTween.new(stage, 1, {x=0}, {ease = easing.inOutQuadratic})
					end
					timer = nil
				end)
			timer:start()
			
		end
	end

	if bulletBody and not bulletJoint then
		local x, y = bulletBody:getPosition()
		
		if x > 240 then
			stage:setX(math.max(-x + 240, -480))
		end
	end
	
	
	for body in pairs(puf) do
		local sprite = body.sprite
		sprite:getParent():removeChild(sprite)
		world:destroyBody(body)
		actors[body.sprite] = nil
		poofAnimation(sprite:getPosition())
	end

	puf = {}
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)

createBullets(4)

do 
	local timer = Timer.new(500, 1)
	timer:addEventListener(Event.TIMER,
		function()
			attachBullet()
			timer = nil
		end)
	timer:start()
end

createTargets()