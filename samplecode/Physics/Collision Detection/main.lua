--[[

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

require "box2d"

b2.setScale(20)

local sky = Bitmap.new(Texture.new("sky.png"))
stage:addChild(sky)

local grass = Bitmap.new(Texture.new("grass.png"))
grass:setY(400)
stage:addChild(grass)

-- this table holds the dynamic bodies and their sprites
local actors = {}

-- create world
local world = b2.World.new(0, 9.8)

-- this function creates a 80x80 physical box and adds it to the stage
local function createBox(x, y, name)
	local body = world:createBody{type = b2.DYNAMIC_BODY, position = {x = x, y = y}}
	
	body.name = name

	local shape = b2.PolygonShape.new()
	shape:setAsBox(40, 40)
	body:createFixture{shape = shape, density = 1, restitution = 0.2, friction = 0.3}

	local sprite = Bitmap.new(Texture.new("box.png"))
	sprite:setAnchorPoint(0.5, 0.5)
	stage:addChild(sprite)
	
	actors[body] = sprite
end

-- create ground body
local ground = world:createBody({})

ground.name = "ground"

local shape = b2.EdgeShape.new(-200, 400, 520, 400)
ground:createFixture({shape = shape, density = 0})

-- create two boxes
createBox(140, -30, "box1")
createBox(180, 300, "box2")

local function onBeginContact(event)
	-- you can get the fixtures and bodies in this contact like:
	local fixtureA = event.fixtureA
	local fixtureB = event.fixtureB
	local bodyA = fixtureA:getBody()
	local bodyB = fixtureB:getBody()
	
	print("begin contact: "..bodyA.name.."<->"..bodyB.name)
end

local function onEndContact(event)
	-- you can get the fixtures and bodies in this contact like:
	local fixtureA = event.fixtureA
	local fixtureB = event.fixtureB
	local bodyA = fixtureA:getBody()
	local bodyB = fixtureB:getBody()

	print("end contact: "..bodyA.name.."<->"..bodyB.name)
end

local function onPreSolve(event)
	-- you can get the fixtures and bodies in this contact like:
	local fixtureA = event.fixtureA
	local fixtureB = event.fixtureB
	local bodyA = fixtureA:getBody()
	local bodyB = fixtureB:getBody()

	print("pre solve: "..bodyA.name.."<->"..bodyB.name)
end

local function onPostSolve(event)
	-- you can get the fixtures and bodies in this contact like:
	local fixtureA = event.fixtureA
	local fixtureB = event.fixtureB
	local bodyA = fixtureA:getBody()
	local bodyB = fixtureB:getBody()

	print("post solve: "..bodyA.name.."<->"..bodyB.name)
end


-- register 4 physics events with the world object
world:addEventListener(Event.BEGIN_CONTACT, onBeginContact)
world:addEventListener(Event.END_CONTACT, onEndContact)
world:addEventListener(Event.PRE_SOLVE, onPreSolve)
world:addEventListener(Event.POST_SOLVE, onPostSolve)

-- step the world and then update the position and rotation of sprites
local function onEnterFrame()
	world:step(1/60, 8, 3)
	
	for body,sprite in pairs(actors) do
		sprite:setPosition(body:getPosition())
		sprite:setRotation(body:getAngle() * 180 / math.pi)
	end
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)
