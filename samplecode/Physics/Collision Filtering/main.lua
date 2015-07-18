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

-- create ground bitmap and add to stage
local groundBitmap = Bitmap.new(Texture.new("ground.png"))
groundBitmap:setAnchorPoint(0, 1)
groundBitmap:setY(320)
stage:addChild(groundBitmap)

-- create ground body
local ground = world:createBody({})

-- ground body is consists of 24 tiny edges
local shape = b2.EdgeShape.new()
local groundFilter = {categoryBits = 1, maskBits = 6}
for x=0,460,20 do
	local x1 = x
	local x2 = x + 20
	local y1 = math.cos((x1 - 240) * 0.01) * 80 + 240
	local y2 = math.cos((x2 - 240) * 0.01) * 80 + 240

	shape:set(x1, y1, x2, y2)
	ground:createFixture({shape = shape, density = 0, filter})
end

-- create a circle body with given texture and collision filter and add it to stage
local function createCircle(texture, x, y, filter)
	local body = world:createBody{type = b2.DYNAMIC_BODY, position = {x = x, y = y}}

	local shape = b2.CircleShape.new(0, 0, 25)
	body:createFixture{shape = shape, density = 1, restitution = 0.2, friction = 0.1, filter = filter}

	local sprite = Bitmap.new(Texture.new(texture, true))
	sprite:setAnchorPoint(0.5, 0.5)
	stage:addChild(sprite)
	
	actors[body] = sprite
end

-- create circles
local redFilter = {categoryBits = 2, maskBits = 3}
local blueFilter = {categoryBits = 4, maskBits = 5}

createCircle("red-circle.png", 60, 100, redFilter)
createCircle("red-circle.png", 120, 100, redFilter)
createCircle("red-circle.png", 180, 100, redFilter)

createCircle("blue-circle.png", 480-60, 100, blueFilter)
createCircle("blue-circle.png", 480-120, 100, blueFilter)
createCircle("blue-circle.png", 480-180, 100, blueFilter)

-- step the world and then update the position and rotation of sprites
local function onEnterFrame()
	world:step(1/60, 8, 3)
	
	for body,sprite in pairs(actors) do
		sprite:setPosition(body:getPosition())
		sprite:setRotation(body:getAngle() * 180 / math.pi)
	end
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)

