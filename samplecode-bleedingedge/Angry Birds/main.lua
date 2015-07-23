--[[
require "box2d"


local FLOOR_HEIGHT = 258

local sprite = nil
sprite = Bitmap.new(Texture.new("bg.png"))
stage:addChild(sprite)


-- create world 
local world = b2.World.new(0, 10, true)

local sling = Sling.new()
sling:setPosition(100, 100)
stage:addChild(sling)

actors = {}

local function onFire(event)
	local x, y = sling:localToGlobal(event.x, event.y)

	local bulletBodyDef = {
		type = b2.DYNAMIC_BODY,
        bullet = true,
		position = {x = x, y = y},
		linearVelocity = {x = -event.dx/5, y = -event.dy/5},
	}

	local circle = b2.CircleShape.new(0, 0, 15)
	local fixtureDef = {
		shape = circle,
		density = 0.8,
		restitution = 0.2,
		friction = 0.99,
	}
	
	local bullet = world:createBody(bulletBodyDef)
	bullet:createFixture(fixtureDef)

	local sprite = Bitmap.new(Texture.new("acorn.png"))
	sprite:setAnchorPoint(0.5, 0.5)
	stage:addChild(sprite)
		
	sprite.body = bullet
	
	actors[sprite] = true	
end

sling:addEventListener("fire", onFire)


local function onEnterFrame()
	world:step(1/60, 8, 3)
	
	for sprite in pairs(actors) do
		local body = sprite.body
		sprite:setPosition(body:getPosition())
		sprite:setRotation(body:getAngle() * 180 / math.pi)
	end
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)
--]]

--[[
local debugDraw = b2.DebugDraw.new()
world:setDebugDraw(debugDraw)
stage:addChild(debugDraw)
--]]

--stage:addChild(Puf.new())

stage:addChild(Game.new())