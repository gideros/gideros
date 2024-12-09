local game = Game
local TILE = game.TILE
local OFFSET = game.OFFSET
local Utils = game.Utils
local SCR = game.Screen
local SHAPES_COUNT = game.SHAPES_COUNT
local BOTTOM_CELL = game.BOTTOM_CELL
local DOWNSCALE = game.DOWNSCALE
local TEXTURE = game.PLACEHOLDER_TEXTURE
local TOP_PANEL = game.TOP_PANEL_H
local GRID_DATA = game.grid

local function tweenComplete(shape)
	shape.dragable = true
end

GameScene = Core.class(Sprite)

function GameScene:init()
	self.layers = Layers.new("panels", "board", "shapes", "ui")
	self:addChild(self.layers)
	
	self:setupVars()
	self:createBoard()
	self:spawnShapes()
	self:createUI()
	
	self:addEventListener("touchesBegin", self.touchBegin, self)
	self:addEventListener("touchesMove", self.touchMove, self)
	self:addEventListener("touchesEnd", self.touchEnd, self)
end
--
function GameScene:setupVars()
	-- drag&drop vars
	self.id = -1
	self.selectedIndex = -1
	self.startX = 0
	self.startY = 0
	
	self.shapeX = 0
	self.shapeY = 0
	
	self.oldTX = 0
	self.oldTY = 0
	self.tx = 0
	self.ty = 0
	
	self.allowDrop = false
	--
	self.shapes = {}
	self.deletedShapes = 0 -- amount of used shapes
	self.points = 0
	self.tilesToRemove = {}
end
--
function GameScene:createBoard()
	local t = {w = 8, h = 8, z = 2}
	Utils.append(t, GRID_DATA)
	self.grid = Grid.new(t)
	
	-- add background tiles
	self.grid:iterate2D(1, function(x, y)
		local px = Tile.new(TEXTURE, TILE, TILE)
		px:setColor(0, 0.1)
		self.grid:add(x, y, 1, px)
		px:setScale(0)
		Timer.delayedCall(math.random() * 200, function()
			local mc = MovieClip.new{
				{1, 60, px, {scale = {0, 1, "outExponential"}}}
			}
		end)
	end)
	-- center grid
	self.grid:setPosition(
		SCR.Left + (SCR.W - self.grid:getWidth()) / 2,
		SCR.Top + TOP_PANEL + (SCR.H - self.grid:getHeight() - BOTTOM_CELL - TOP_PANEL) / 2
	)
	self.layers:add("board", self.grid)
	
	local panel = Pixel.new(0, 0.1, SCR.W, BOTTOM_CELL)
	panel:setPosition(
		SCR.Left,
		SCR.Bottom - BOTTOM_CELL
	)
	self.layers:add("panels", panel)
	
	local panel = Pixel.new(0, 0.1, SCR.W, TOP_PANEL)
	panel:setPosition(
		SCR.Left,
		SCR.Top
	)
	self.layers:add("panels", panel)
end
--
function GameScene:spawnShapes()	
	for i = 1, SHAPES_COUNT do 
		local shape = Shape.new()
		shape:setScale(DOWNSCALE)
		shape:setAnchorPoint(.5,.5)
		shape:setPosition(
			SCR.Left + (i - 1) * BOTTOM_CELL + BOTTOM_CELL / 2,
			SCR.Bottom - BOTTOM_CELL / 2	
		)
		shape:runINIT()
		shape.shadow:setVisible(false)
		self.layers:add("shapes", shape.shadow)
		self.layers:add("shapes", shape)
		self.shapes[i] = shape
	end
end
--
function GameScene:createUI()
	self.ui = UI.new()
	self.layers:add("ui", self.ui)
	
	self.resetBtn = UI.TextButton.new{
		w = 180, h = 100,
		image = TEXTURE,
		ninePatch = {20},
		text = "Reset",
		font = GetFont("main_48"),
		callback = function()
			self:gameOver()
			--self:reset()
		end
	}
	self.resetBtn:setPosition(SCR.Right - 200, SCR.Top + (TOP_PANEL - 100)/2)
	self.ui:addChild(self.resetBtn)
	
	self.pointsLabel = Label.new{
		w = SCR.W, h = TOP_PANEL,
		font = GetFont("main_48"),
		text = "Points: 00000"
	}
	self.pointsLabel:setPosition(SCR.Left, SCR.Top)
	self.layers:add("ui", self.pointsLabel)
end
--
function GameScene:reset()
	for i = 1, SHAPES_COUNT do 
		local s = self.shapes[i]
		if (s) then s:removeFromParent() end
		self.shapes[i] = nil
	end
	
	self.grid:iterate2D(2, function(x,y)
		self.grid:remove(x,y,2)
	end)
	
	self:setupVars()
	self:spawnShapes()
	self:setPoints(0)
end
--
function GameScene:gameOver()
	local maxTime = 0
	self.grid:iterate2D(2, function(x, y, v)
		if (v and v ~= 0) then
			local delay = math.random() * 200
			maxTime = math.max(maxTime, delay)
			Timer.delayedCall(delay, function()
				local mc = MovieClip.new{
					{1, 40, v, {
						scale = {1, 0, "inBack"},
						alpha = {1, 0, "inBack"},
					}},
				}
			end)
		end
	end)
	Timer.delayedCall(1000, function()
		self:reset()
	end)
end
--
function GameScene:tryFit(tx, ty, shape)
	local flag = true
	shape.grid:iterate2D(1, function(x,y,v)
		if (v ~= 0) then 
			local bg_tile = self.grid:getAt(tx + (x-1), ty + (y-1), 2)
			
			if (bg_tile ~= 0) then 
				flag = false
				return true
			end
		end
	end)
	return flag
end
--
function GameScene:insert(tx, ty, shape, setColor)
	shape.grid:iterate2D(1, function(x,y,v)
		if (v ~= 0) then 
			self.grid:add(tx + (x-1), ty + (y-1), 2, v)
			if (setColor) then 
				v:setColor(v.color)
			end
		end
	end)
end
--
function GameScene:remove(tx, ty, shape)
	shape.grid:iterate2D(1, function(x,y,v)
		if (v ~= 0) then 
			local v = self.grid:remove(tx + (x-1), ty + (y-1), 2)
			shape.grid:add(x,y,1, v)
		end
	end)
end
--
function GameScene:checkGrid()
	local flag = false
	for i = 1, SHAPES_COUNT do 
		local shape = self.shapes[i]
		if (shape) then 
			self.grid:iterate2D(2, function(x, y)
				if (self:tryFit(x,y,shape)) then 
					flag = true
					return true
				else
					flag = false
				end
			end)
			
			if (flag) then 
				shape:enable()
			else
				shape:disable()
			end
		end
	end
end
--
function GameScene:removeLines()

	self:setPoints(self.points + (#self.tilesToRemove > 0 and #self.tilesToRemove or self.dragShape.points))
	for i,t in ipairs(self.tilesToRemove) do 
		self.grid:remove(t[2],t[3],2)
		self.tilesToRemove[i] = nil
	end
end
--
function GameScene:checkLose()
	local disabled = 0
	for i = 1, SHAPES_COUNT do 
		local shape = self.shapes[i]
		if (shape and not shape.enabled) then	
			disabled += 1
		end
	end
	
	if (disabled == SHAPES_COUNT - self.deletedShapes) then 
		self:gameOver()
	end
end
--
function GameScene:clearTileToRemove()
	for i,t in ipairs(self.tilesToRemove) do 
		t[1].added = false
		t[1]:setScale(1)
		self.tilesToRemove[i] = nil
	end
end
--
function GameScene:markTiles()
	local g = self.grid
	local flag = true
	local tilesRemoved = 0
	self:clearTileToRemove()
	self:insert(self.tx, self.ty, self.dragShape)
	self.tilesToRemove = {}
	-- scan rows
	for y = self.ty,self.ty+self.dragShape.grid.h-1 do 
		flag = true
		for x = 1, g.w do 
			local v = g:getAt(x,y,2)
			if (not v or v == 0) then 
				flag = false
				break
			end
		end
		
		if (flag) then -- if whole line is filled with blocks then put them into table to remove later
			tilesRemoved += g.w 
			for x = 1, g.w do 
				--g:remove(x,y,2)
				local v = g:getAt(x, y, 2)
				v.added = true
				v:setScale(.8)
				self.tilesToRemove[#self.tilesToRemove+1] = {v,x,y}
			end
		end
	end
	
	flag = true
	-- scan columns
	for x = self.tx,self.tx+self.dragShape.grid.w-1 do 
		flag = true
		for y = 1, g.h do 
			local v = g:getAt(x,y,2)
			if (not v or v == 0) then 
				flag = false
				break
			end
		end
		
		if (flag) then -- if whole line is filled with blocks then remove them
			tilesRemoved += g.h 
			for y = 1, g.h do 
				local v = g:getAt(x, y, 2)
				if (not v.added) then 
					v.added = true
					v:setScale(.8)
					self.tilesToRemove[#self.tilesToRemove+1] = {v,x,y}
				end
			end
		end
	end
	self:remove(self.tx, self.ty, self.dragShape)
	--self.points += linesRemoved*tilesRemoved
end
--
function GameScene:setPoints(points)
	self.points = points
	self.pointsLabel:setText(("Points: %05i"):format(self.points))
end
-- called when shape successfully dropped onto the board
function GameScene:drop()
	self.deletedShapes += 1
	-- remove shadow 
	self.dragShadow:removeFromParent()
	-- add shape blocks to grid
	self:insert(self.tx, self.ty, self.dragShape, true)
	-- remove shape from spawn pool 
	self.shapes[self.selectedIndex] = nil
	-- remove lines
	self:removeLines()
	-- if shape pool is empty, spawn new
	if (self.deletedShapes == SHAPES_COUNT) then 
		self.deletedShapes = 0
		self:spawnShapes()	
	end
	-- disable blocks that can't be placed onto a grid
	self:checkGrid()
	--
	self:checkLose()
	self.allowDrop = false
end
--
function GameScene:touchBegin(e)
	local x, y, id = e.touch.x, e.touch.y, e.touch.id
	if (self.id == -1 and y > SCR.Bottom - BOTTOM_CELL) then 
		self.selectedIndex = 1 + ((x-SCR.Left) // BOTTOM_CELL)
		if (self.selectedIndex >= 1 or self.selectedIndex <= SHAPES_COUNT) then 
			self.dragShape = self.shapes[self.selectedIndex]
			if (self.dragShape and self.dragShape.dragable and self.dragShape.enabled) then 
				self.dragShadow = self.dragShape.shadow
				
				self.layers:add("shapes", self.dragShape)
				self.shapeX, self.shapeY = self.dragShape:getPosition()
				self.dragShape:setY(self.shapeY - 200)
				self.dragShape:runGrowUP()
				--
				self.startX = x
				self.startY = y
				self.id = id
			end
		end
	end
end
--
function GameScene:touchMove(e)
	local x, y, id = e.touch.x, e.touch.y, e.touch.id
	if (self.id == id) then 
		local HT = (TILE + OFFSET) / 2
		local newX = self.shapeX + (x - self.startX)
		local newY = self.shapeY + (y - self.startY) - 200
		
		self.dragShape:setPosition(newX, newY)
		
		local w,h = self.dragShape:getSize()
		
		local lx, ly = self.grid:globalToLocal(newX - w / 2 + HT , newY - h / 2 + HT)
		
		self.oldTX = self.tx
		self.oldTY = self.ty
		
		self.tx, self.ty = self.grid:toLocal(lx, ly)
		
		if (self.oldTX ~= self.tx or self.oldTY ~= self.ty) then 
			self.allowDrop = self:tryFit(self.tx, self.ty, self.dragShape)
			
			local gx, gy = self.grid:localToGlobal(self.grid:toGlobal(self.tx, self.ty))
			self.dragShadow:setVisible(self.allowDrop)
			self.dragShadow:setPosition(gx - HT, gy - HT)
			
			if (self.allowDrop) then 
				self:markTiles()
			else
				self:clearTileToRemove()
			end
		end
	end
end
--
function GameScene:touchEnd(e)
	local x, y, id = e.touch.x, e.touch.y, e.touch.id
	if (self.id == id) then 
		if (self.allowDrop) then
			self:drop()
		else
			local sx,sy = self.dragShape:getPosition()
			--self.dragShape:setPosition(self.shapeX, self.shapeY)
			local dx = SCR.Left + (self.selectedIndex - 1) * BOTTOM_CELL + BOTTOM_CELL / 2
			local dy = SCR.Bottom - BOTTOM_CELL / 2
			local mc = MovieClip.new{
				{1, 30, self.dragShape, {x = {sx,dx,"inOutBack"}, y = {sy,dy,"inOutBack"}}}
			}
			mc:addEventListener("complete", tweenComplete, self.dragShape)
			
			self.dragShape.dragable = false
			self.dragShape:runGrowDOWN()
		end
		
		self.dragShape = nil
		self.id = -1
		self.selectedIndex = -1
	end
end