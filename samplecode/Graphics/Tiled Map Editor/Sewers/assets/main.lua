--[[

Tile Map example
sewers.lua is generated using Lua export feature of Tiled Qt 0.8.0
(don't forget to Exclude it from Execution)

This code is MIT licensed, see https://opensource.org/license/mit
(C) 2010 - 2011 Gideros Mobile 

updated: 2024-12-12
]]


local map = require("sewers")

local floor = math.floor

-- bg
application:setBackgroundColor(0x55aaff)

-- map size and texture
for i = 1, #map.tilesets do
	local tileset = map.tilesets[i]
	tileset.sizex = floor((tileset.imagewidth-tileset.margin+tileset.spacing)/(tileset.tilewidth+tileset.spacing))
	tileset.sizey = floor((tileset.imageheight-tileset.margin+tileset.spacing)/(tileset.tileheight+tileset.spacing))
	tileset.lastgid = tileset.firstgid+(tileset.sizex*tileset.sizey)-1
	tileset.texture = Texture.new(tileset.image, false, {transparentColor = 0xff00ff})
end

-- build the Tiled map
local function gid2tileset(gid)
	for i = 1, #map.tilesets do
		local tileset = map.tilesets[i]
		if tileset.firstgid <= gid and gid <= tileset.lastgid then
			return tileset
		end
	end
end

local allLayers = Sprite.new()

-- bits on the far end of the 32-bit global tile ID are used for tile flags (flip, rotate)
local FLIPPED_HORIZONTALLY_FLAG = 0x80000000
local FLIPPED_VERTICALLY_FLAG   = 0x40000000
local FLIPPED_DIAGONALLY_FLAG   = 0x20000000
local flipHor = 0
local flipVer = 0
local flipDia = 0

for i = 1, #map.layers do
	local layer = map.layers[i]
	local tilemaps = {}
	local group = Sprite.new()
	for y = 1, layer.height do
		for x = 1, layer.width do
			local index = x+(y-1)*layer.width
			local gid = layer.data[index]
			if gid ~= 0 then
				-- read flipping flags
				flipHor = gid & FLIPPED_HORIZONTALLY_FLAG
				flipVer = gid & FLIPPED_VERTICALLY_FLAG
				flipDia = gid & FLIPPED_DIAGONALLY_FLAG
				-- convert flags to Gideros style
				if flipHor ~= 0 then flipHor = TileMap.FLIP_HORIZONTAL end
				if flipVer ~= 0 then flipVer = TileMap.FLIP_VERTICAL end
				if flipDia ~= 0 then flipDia = TileMap.FLIP_DIAGONAL end
				-- clear the flags from gid so other information is healthy
				gid = gid & ~ (	FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG )
			end
			local tileset = gid2tileset(gid)
			if tileset then
				local tilemap = nil
				if tilemaps[tileset] then
					tilemap = tilemaps[tileset]
				else
					tilemap = TileMap.new(
						layer.width, layer.height,
						tileset.texture,
						tileset.tilewidth, tileset.tileheight,
						tileset.spacing, tileset.spacing,
						tileset.margin, tileset.margin,
						map.tilewidth, map.tileheight
					)
					tilemaps[tileset] = tilemap
					group:addChild(tilemap)
				end
				local tx = (gid-tileset.firstgid)%tileset.sizex+1
				local ty = floor((gid-tileset.firstgid)/tileset.sizex)+1
				-- set the tile with flip info
				tilemap:setTile(x, y, tx, ty, flipHor|flipVer|flipDia)
			end
		end
	end
	group:setAlpha(layer.opacity)
	allLayers:addChild(group)
end
stage:addChild(allLayers)

-- mouse listeners
local dragging, startx, starty
local function onMouseDown(event)
	dragging = true
	startx = event.x
	starty = event.y
end
local function onMouseMove(event)
	if dragging then
		local dx = event.x-startx
		local dy = event.y-starty
		allLayers:setX(allLayers:getX()+dx)
		allLayers:setY(allLayers:getY()+dy)
		startx = event.x
		starty = event.y
	end
end
local function onMouseUp(event)
	dragging = false
end
stage:addEventListener(Event.MOUSE_DOWN, onMouseDown)
stage:addEventListener(Event.MOUSE_MOVE, onMouseMove)
stage:addEventListener(Event.MOUSE_UP, onMouseUp)

-- help
local info1 = TextField.new(nil, "explore the tilemap by dragging")
local info2 = TextField.new(nil, "your mouse/finger across the screen")
info1:setTextColor(0xffffff)
info2:setTextColor(0xffffff)
info1:setPosition(70, 50)
info2:setPosition(60, 60)
stage:addChild(info1)
stage:addChild(info2)
