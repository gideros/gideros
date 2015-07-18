local spritesOnStage = {}

MySprite = Core.class(Sprite)

function MySprite:init()
	self:addEventListener(Event.ADDED_TO_STAGE, self.onAddedToStage, self)
	self:addEventListener(Event.REMOVED_FROM_STAGE, self.onRemovedFromStage, self)
end

function MySprite:onAddedToStage()
	if spritesOnStage[self] == true then
		print("error! onAddedToStage")
	end
	spritesOnStage[self] = true
end

function MySprite:onRemovedFromStage()
	if spritesOnStage[self] == nil then
		print("error! onRemovedFromStage")
	end
	spritesOnStage[self] = nil
end

local function populate(sprite, sprites)
	for i=1,sprite:getNumChildren() do
		local child = sprite:getChildAt(i)
		sprites[child] = true
		populate(child, sprites)
	end
	return sprites
end

local function check()
	local spritesOnStage2 = populate(stage, {})
	
	for k,v in pairs(spritesOnStage) do
		if spritesOnStage2[k] == nil then
			print("error! check1")
		end
	end

	for k,v in pairs(spritesOnStage2) do
		if spritesOnStage[k] == nil then
			print("error! check2")
		end
	end
end

local sprites = {}
for i=1,20 do
	sprites[i] = MySprite.new()
	stage:addChild(sprites[i])
end
sprites[#sprites + 1] = stage

local n = 0

local function onEnterFrame()
	if math.random(1, 4) == 1 then
		local sprite = sprites[math.random(1, #sprites - 1)]
		sprite:removeFromParent()
	else
		local sprite1 = sprites[math.random(1, #sprites)]       
		local sprite2 = sprites[math.random(1, #sprites - 1)]       
		pcall(function() sprite1:addChild(sprite2) end)
	end
	check()
	n = n + 1
	if n % 100 == 0 then
		print("tested "..n.." tree mutations...")
	end
end

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)