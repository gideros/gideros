--[[
  
This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 
 
]]

require "box2d"

b2.setScale(20)

-- this function creates a box sprite with 2 happy and sad children
local function createBoxSprite(sx, sy)
	local happy = Bitmap.new(Texture.new("happy-box.png", true))
	happy:setAnchorPoint(0.5, 0.5)

	local sad = Bitmap.new(Texture.new("sad-box.png", true))
	sad:setAnchorPoint(0.5, 0.5)

	local sprite = Sprite.new()
	sprite:addChild(happy)
	sprite:addChild(sad)

	sprite:setScale(sx, sy)

	return sprite
end

-- this table holds the dynamic bodies and their sprites
local actors = {}

-- create world
local world = b2.World.new(0, 9.8)

-- create a ground body and attach an edge shape
local ground = world:createBody({})
local shape = b2.EdgeShape.new(-200, 480, 520, 480)
ground:createFixture({shape = shape, density = 0})

-- every 3 seconds, we create a random box
local function onTimer()
	local sx = math.random(70, 100) / 100
	local sy = math.random(70, 100) / 100
	
	local body = world:createBody{type = b2.DYNAMIC_BODY, position = {x = math.random(0, 320), y = -35}}

	local shape = b2.PolygonShape.new()
	-- box images are 70x70 pixels. we create bodies 1 pixel smaller than that.
	shape:setAsBox(34.5 * sx, 34.5 * sy)
	body:createFixture{shape = shape, density = 1, restitution = 0.1, friction = 0.3}

	local sprite = createBoxSprite(sx, sy)
	stage:addChild(sprite)
	
	actors[body] = sprite
end

-- set the timer for every 3 seconds
local timer = Timer.new(3000, 0)
timer:addEventListener(Event.TIMER, onTimer)
timer:start()
onTimer() -- create a box immediately

-- step the world and then update the position and rotation of sprites
local function onEnterFrame()
	world:step(1/60, 8, 3)
	
	for body,sprite in pairs(actors) do
		sprite:setPosition(body:getPosition())
		sprite:setRotation(body:getAngle() * 180 / math.pi)
		
		if body:isAwake() then
			sprite:getChildAt(1):setVisible(false)
			sprite:getChildAt(2):setVisible(true)
		else
			sprite:getChildAt(1):setVisible(true)
			sprite:getChildAt(2):setVisible(false)
		end
	end

	for body,sprite in pairs(actors) do
		if sprite:getY() > 600 then
			world:destroyBody(body)
			stage:removeChild(sprite)
			actors[body] = nil
		end	
	end
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)

