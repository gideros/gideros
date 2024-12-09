local minX, minY, maxX, maxY = application:getLogicalBounds()
local w = -minX + maxX
local h = -minY + maxY
local TILE = 80
local OFFSET = 4
local SHAPES_COUNT = 3
local BOTTOM_CELL = w / SHAPES_COUNT
Game = {
	Utils = require "libs/utils",
	TILE = TILE,
	OFFSET = OFFSET,
	SHAPES_COUNT = SHAPES_COUNT,
	
	TOP_PANEL_H = 200,
	
	BOTTOM_CELL = BOTTOM_CELL,
	DOWNSCALE = BOTTOM_CELL / ((TILE + OFFSET) * 5 + 5), -- 5 is maximum shape width/height, 10 - extra size
	
	TILETEXTURE = Texture.new("gfx/9patch_4.png", true),
	PLACEHOLDER_TEXTURE = Texture.new("gfx/9patch_4.png", true),
	
	curScene = "Game",
	prevScene = "",
	
	Screen = {
		Left = minX, Top = minY,
		Right = maxX, Bottom = maxY,
		W = w, H = h,
		CX = minX + w / 2,
		CY = minY + h / 2,
	},
	
	grid = {
		tileW = TILE,
		offsetX = OFFSET,
		ax = .5, ay = .5
	},
	
	colors = {0x8dd0e0, 0x59b08d, 0x407fc2, 0xf7a96b, 0xfbcb65, 0xf6a86a, 0xfa515d}
}