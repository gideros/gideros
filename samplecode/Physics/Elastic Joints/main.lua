--[[

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

require "box2d"

b2.setScale(20)

-- add background
stage:addChild(Bitmap.new(Texture.new("background.png")))

-- this table holds the dynamic bodies and their sprites (for this example, it holds only one body and sprite)
local actors = {}

-- create world without gravity
local world = b2.World.new(0, 0)

-- create ground body
local ground = world:createBody({})

-- create sun body and sprite
local bodyDef = {type = b2.DYNAMIC_BODY, position = {x = 160, y = 110}, fixedRotation = true, linearDamping = 1}
local body = world:createBody(bodyDef)
local shape = b2.PolygonShape.new()
shape:setAsBox(20, 20)
body:createFixture{shape = shape, density = 1}

local sprite = Bitmap.new(Texture.new("sun_shades.png", true))
sprite:setAnchorPoint(0.5, 0.5)
stage:addChild(sprite)

actors[body] = sprite

-- attach sun body and ground body with distance joint
local jointDef = b2.createDistanceJointDef(ground, body, 160, 110, 160, 110)
jointDef.frequencyHz = 2.0;
jointDef.dampingRatio = 0;
world:createJoint(jointDef)



local mouseJoint = nil

-- create a mouse joint on mouse down
function sprite:onMouseDown(event)
	if self:hitTestPoint(event.x, event.y) then
		local jointDef = b2.createMouseJointDef(ground, body, event.x, event.y, 100000)
		mouseJoint = world:createJoint(jointDef)
	end
end

-- update the target of mouse joint on mouse move
function sprite:onMouseMove(event)
	if mouseJoint ~= nil then
		mouseJoint:setTarget(event.x, event.y)
	end
end

-- destroy the mouse joint on mouse update up
function sprite:onMouseUp(event)
	if mouseJoint ~= nil then
		world:destroyJoint(mouseJoint)
		mouseJoint = nil
	end
end

-- register for mouse events
sprite:addEventListener(Event.MOUSE_DOWN, sprite.onMouseDown, sprite)
sprite:addEventListener(Event.MOUSE_MOVE, sprite.onMouseMove, sprite)
sprite:addEventListener(Event.MOUSE_UP, sprite.onMouseUp, sprite)

local function onEnterFrame()
	world:step(1/60, 8, 3)
	
	for body,sprite in pairs(actors) do
		sprite:setPosition(body:getPosition())
		sprite:setRotation(body:getAngle() * 180 / math.pi)
	end
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)
