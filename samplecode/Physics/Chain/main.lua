--[[
  
This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 
 
]]

require "box2d"

b2.setScale(30)

-- this table holds the dynamic bodies and their sprites
local actors = {}

-- create world
local world = b2.World.new(0, 9.8)

-- create ground body
local ground = world:createBody({})

-- create an edge shape, and attach it to the ground body as a fixture
local shape = b2.EdgeShape.new(0, 350, 320, 350)
ground:createFixture({shape = shape, density = 0})

-- this box shape will be used while creating the chain elements
local shape = b2.PolygonShape.new()
shape:setAsBox(5, 1)

-- and our fixture definition
local fixtureDef = {shape = shape, density = 20, friction = 0.2}

-- start to create the bridge
local prevBody = ground

for i=1,30 do
	local bodyDef = {type = b2.DYNAMIC_BODY, position = {x = 164 + i * 8, y = 150}}
	local body = world:createBody(bodyDef)
	body:createFixture(fixtureDef)
	
	local bitmap = Bitmap.new(Texture.new("rect.png", true))
	bitmap:setAnchorPoint(0.5, 0.5)
	stage:addChild(bitmap)

	actors[body] = bitmap

	-- attach each pair of chain elements with revolute joint
	local jointDef = b2.createRevoluteJointDef(prevBody, body, 160 + i * 8, 150)
	jointDef.collideConnected = false
	world:createJoint(jointDef)
	
	prevBody = body
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
