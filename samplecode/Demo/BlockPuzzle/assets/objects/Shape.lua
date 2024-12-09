_Shape = Shape -- save gideros Shape class JUST IN CASE

local game = Game
local frames = require "data/frames"
local Utils = game.Utils
local TILE = game.TILE
local DOWNSCALE = game.DOWNSCALE
local TEXTURE = game.TILETEXTURE
Shape = Core.class(Sprite)

function Shape:init(name, frame)
	self.dragable = true
	self.points = 0
	
	-----------------------------
	if (not (name and frame)) then 
		name = Utils.choose("LL", "SL", "I", "O", "T", "J", "L", "S", "Z")
		frame = math.random(#frames[name])
	end
	
	local data = frames[name][frame]
	local h = #data
	local w = #data[h]
	local t = {w = w, h = h}
	Utils.append(t, game.grid)
	self.grid = Grid.new(t)
	self.shadow = Grid.new(t)
	
	self.color = color or Utils.chooseTable(game.colors)
	
	self.grid:iterate2D(1, function(x, y)
		if (data[y][x] == 1) then
			self.points += 1
			
			self.grid:add(x, y, 1, Tile.new(TEXTURE, TILE, TILE, self.color))
			
			local t = Tile.new(TEXTURE, TILE, TILE)
			t:setColor(0, 0.2)
			self.shadow:add(x, y, 1, t)
		end
	end)
	self:addChild(self.grid)
	
	-----------------------------
	
	self.mc = MovieClip.new{
		{1, 15, self, {
			scale = {DOWNSCALE, 1, "outBack"},
		}},
		{16, 40, self, {
			alpha = {0, 1, "outExponential"},
			scale = {0, DOWNSCALE, "outBack"},
		}},
	}
	self.mc:addEventListener("complete", self.tweenComplete, self)
	self.mc:stop()
	self.mc:setStopAction(15)
	self:setScale(1)
	
	self:enable()
end
--
function Shape:tweenComplete()
	self.dragable = true
end
--
function Shape:runINIT()
	self.dragable = false
	self.mc:gotoAndPlay(16)
end
--
function Shape:runGrowUP()
	self.dragable = false
	self.mc:gotoAndPlay(1)
end
--
function Shape:runGrowDOWN()
	self.dragable = false
	self.mc:gotoAndPlay(self.mc:getFrame()-1, true)
end
--
function Shape:enable()
	self.enabled = true
	local r,g,b = Utils.hex2rgb(self.color)
	self:setColorTransform(r,g,b,1)
end
--
function Shape:disable()
	self.enabled = false
	self:setColorTransform(0.5,0.5,0.5,1)	
end
--