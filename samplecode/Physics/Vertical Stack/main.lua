--[[
  
This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 
 
]]

require "box2d"

b2.setScale(16)

-- create world
local world = b2.World.new(0, 9.8)

-- create ground body
local ground = world:createBody{}

local shape = b2.EdgeShape.new()

-- attach 1st fixture to the ground body
shape:set(-20, 290, 620, 290)
ground:createFixture({shape = shape, density = 0})

-- attach 2nd fixture to the ground body
shape:set(460, 290, 460, 130)
ground:createFixture({shape = shape, density = 0})

-- we will create boxes with this fixture definition
local shape = b2.PolygonShape.new()
shape:setAsBox(4, 4)
local fixtureDef = {shape = shape, density = 1, friction = 0.3}

-- create boxes
for x = 220,380,40 do
	for y = 104,284,12 do
		local bodyDef = {type = b2.DYNAMIC_BODY, position = {x = x, y = y}}					
		local body = world:createBody(bodyDef)
		body:createFixture(fixtureDef)
	end
end

-- create and launch the bullet (attention to bullet=true)
local function launchBullet()
	local bodyDef = {type = b2.DYNAMIC_BODY, bullet = true, position = {x=-50, y=250}}
	local bullet = world:createBody(bodyDef)
	
	local shape = b2.CircleShape.new(0, 0, 2)
	local fixtureDef = {shape = shape, density = 20, restitution = 0.05}
	bullet:createFixture(fixtureDef)
	
	bullet:setLinearVelocity(400, 0)
end

-- launch the bullet after 1.5 seconds
local timer = Timer.new(1500, 1)
timer:addEventListener(Event.TIMER, launchBullet)
timer:start()

-- in this example, we display the box2d world with debug draw
-- please look at other physics examples to understand binding box2d bodies with sprites
local debugDraw = b2.DebugDraw.new()
world:setDebugDraw(debugDraw)
stage:addChild(debugDraw)

-- step the world at each frame
local function onEnterFrame()
	world:step(1/60, 8, 3)
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)
