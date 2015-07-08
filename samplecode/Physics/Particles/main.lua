require "box2d"

application:setScaleMode("letterbox")
application:setOrientation(application.LANDSCAPE_LEFT)
application:setLogicalDimensions(600,800)

local cw=application:getContentWidth()
local ch=application:getContentHeight()

-- this table holds the dynamic bodies and their sprites
local actors = {}

-- create world
local world = b2.World.new(0, 9.8)

-- create ground body
local ground = world:createBody({})

-- create an edge shape, and attach it to the ground body as a fixture
local shapeb = b2.EdgeShape.new(0, ch, cw, ch)
local shapet = b2.EdgeShape.new(0, 0, cw, 0)
local shapel = b2.EdgeShape.new(0, 0, 0, ch)
local shaper = b2.EdgeShape.new(cw, 0, cw, ch)
ground:createFixture({shape = shapeb, density = 0})
ground:createFixture({shape = shaper, density = 0})
ground:createFixture({shape = shapel, density = 0})
ground:createFixture({shape = shapet, density = 0})


local shape1 = b2.PolygonShape.new()
shape1:setAsBox(200, 50)
local shape2 = b2.PolygonShape.new()
shape2:setAsBox(200, 100)

local ps1= world:createParticleSystem({ radius=5})
ps1:setTexture(Texture.new("Bubble.png"))
stage:addChild(ps1)

ps1:createParticleGroup({shape=shape2, 
position={x=500,y=250},
color = 0xFF0000,
alpha=1,
flags=0
})

ps1:createParticleGroup({shape=shape1, 
position={x=400,y=50},
color = 0x0000FF,
alpha=1,
flags=0
})


-- step the world and then update the position and rotation of sprites
local function onEnterFrame()
	world:step(1/60, 8, 3)
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)