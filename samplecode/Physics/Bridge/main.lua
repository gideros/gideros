--[[
  
This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 
 
]]

require "box2d"

b2.setScale(10)

-- this table holds the dynamic bodies and their sprites
local actors = {}

-- create world
local world = b2.World.new(0, 9.8)

-- create ground body
local ground = world:createBody({})

-- create an edge shape, and attach it to the ground body as a fixture
local shape = b2.EdgeShape.new(0, 480, 320, 480)
ground:createFixture({shape = shape, density = 0})

-- this box shape will be used while creating the bridge elements
local shape = b2.PolygonShape.new()
shape:setAsBox(11, 2)

-- and our fixture definition
local fixtureDef = {shape = shape, density = 20, friction = 0.2}

-- start to create the bridge
local prevBody = ground

for i=0,13 do
	local bodyDef = {type = b2.DYNAMIC_BODY, position = {x = 30 + i * 20, y = 250}}
	local body = world:createBody(bodyDef)
	body:createFixture(fixtureDef)

	local bitmap = Bitmap.new(Texture.new("rect.png", true))
	bitmap:setAnchorPoint(0.5, 0.5)
	stage:addChild(bitmap)
	
	actors[body] = bitmap

	-- attach each pair of bridge elements with revolute joint
	local jointDef = b2.createRevoluteJointDef(prevBody, body, 20 + i * 20, 250)
	world:createJoint(jointDef)

	prevBody = body
end

-- attach last bridge element to the ground body
local jointDef = b2.createRevoluteJointDef(prevBody, ground, 300, 250)
world:createJoint(jointDef)



-- this box shape will be used while creating boxes falling down
local shape = b2.PolygonShape.new()
shape:setAsBox(10, 10)

local fixtureDef = {shape = shape, density = 2, restitution = 0.7}

-- create 9 boxes
for j=1,3 do
	for i=1,3 do
		local bodyDef = {type = b2.DYNAMIC_BODY, position = {x = 80 + i * 30, y = j * 30 + 20}}
		local body = world:createBody(bodyDef)
		body:createFixture(fixtureDef)

		local bitmap = Bitmap.new(Texture.new("box.png", true))
		bitmap:setAnchorPoint(0.5, 0.5)
		stage:addChild(bitmap)
		
		actors[body] = bitmap
	end
end


-- step the world and then update the position and rotation of sprites
local function onEnterFrame()
	world:step(1/60, 8, 3)
	
	for body,sprite in pairs(actors) do
		sprite:setPosition(body:getPosition())
		sprite:setRotation(body:getAngle() * 180 / math.pi)
	end
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)

