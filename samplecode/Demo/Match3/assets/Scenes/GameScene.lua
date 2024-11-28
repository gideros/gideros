local sqrt,cos,sin = math.sqrt,math.cos,math.sin

DELAY_SCALE = 140
ANIM_SPEED = 0.08
BLOCKS = TexturePack.new("gfx/Blocks.txt", "gfx/Blocks.png", true)
FX = TexturePack.new("gfx/FX.txt", "gfx/FX.png", true)

GameScene = Core.class(Sprite)

function GameScene:init()
	
	--local data = require("data/Level_1")
	local data = {}
	data.cells = {}
	
	local w,h = 6,6--math.random(6,8),math.random(10,15)
	for y = 1,h do 
		data.cells[y] = {}
		for x = 1,w do 
			data.cells[y][x] = math.random() < 0.15 and 1 or 0
		end
	end
	
	self.board = Board.new{tileW = TILE, offsetX = 8, ax = ANCHOR_X, ay = ANCHOR_Y}
	self.board:load(data)
	self.board:setPosition((720 - self.board:getWidth()) / 2, (1280 - self.board:getHeight()) / 2)
	self:addChild(self.board)
	
	self.dist = 0
	self.sx = 0
	self.sy = 0	
	self.maxDist = (TILE+self.board.offsetX)^2+(TILE+self.board.offsetY)^2
	self.activeColor = ""
	self.activeLine = nil
	self.activeTile = nil
	self.prevTile = nil
	self.connections = {}
	self.board:spawn()
	
	self:addEventListener("touchesBegin", self.touchesBegin, self)
	self:addEventListener("touchesMove", self.touchesMove, self)
	self:addEventListener("touchesEnd", self.touchesEnd, self)
end
--
function GameScene:pushConnection(tx,ty,tile)
	local selector = Bitmap.new(BLOCKS:getTextureRegion("selector.png"))
	selector:setAnchorPoint(ANCHOR_X,ANCHOR_Y)
	self.board:add(tx,ty,SELECT_LEVEL, selector)
	
	local line = Pixel.new(BLOCKS:getTextureRegion("line.png"), 0,16)
	line:setNinePatch(4)
	line:setAnchorPoint(0,0.5)
	self.board:add(tx,ty,LINE_LEVEL, line, true)
	
	tile.isConnected = true
	tile:setSleep(false)
	self.connections[#self.connections+1] = {
		tx = tx, 
		ty = ty,
		line = line,
		selector = selector,
		tile = tile,
	}
	
	self.activeLine = line
	self.activeTile = tile
end
--
function GameScene:popConnection()
	local n = #self.connections
	local removed = table.remove(self.connections, n)
	removed.selector:removeFromParent()
	removed.line:removeFromParent()
	removed.tile.isConnected = false
	removed.tile:setSleep(true)
	
	local last = self.connections[n-1]
	if last then 
		self.prevTile = nil
		if n-2 > 0 then 
			self.prevTile = self.connections[n-2].tile
		end
		self.activeTile = last.tile
		self.activeLine = last.line
	end
	
	return removed
end
--
function GameScene:removeTile(tile)
	self.board:remove(tile.tx,tile.ty,DATA_LEVEL)
end
--
function GameScene:translateToTile(x,y)
	local lx,ly = self.board:globalToLocal(x,y)
	return self.board:toTile(lx,ly)
end
--
function GameScene:translateToGlobal(tx,ty)
	local x,y = self.board:toGlobal(tx,ty)
	return self.board:localToGlobal(x,y)
end
--
function GameScene:updateLine(line, dstX, dstY)
	local ang = angle(self.sx,self.sy,dstX, dstY)
	local dist = distance(self.sx,self.sy,dstX, dstY)
	local clampDist = clamp(dist,0,self.maxDist)
	local clampDistSq = sqrt(clampDist)
	line:setRotation(^>ang)
	line:setDimensions(clampDistSq, 16)
	return dist,clampDistSq,ang
end
--
function GameScene:touchesBegin(e)
	local x,y = e.touch.x,e.touch.y
	local tx,ty = self:translateToTile(x,y)
	local tile = self.board:getAt(tx,ty,DATA_LEVEL)
	if self.board.state == "" and self.activeColor == "" and self.board:checkTile(tile) then 
		self:pushConnection(tx,ty,tile)
		self.sx, self.sy = self:translateToGlobal(tx,ty)
		self.activeColor = tile.name
		tile:setSleep(false)
	end
end
--
function GameScene:touchesMove(e)
	--self.board:remove(e.x,e.y)
	local x,y = e.touch.x,e.touch.y
	if self.activeColor ~= "" then
		local dist,distSq,ang = self:updateLine(self.activeLine, x, y)		
		local ex = self.sx + cos(ang) * (distSq - 10)
		local ey = self.sy + sin(ang) * (distSq - 10)
		
		local tx,ty = self:translateToTile(ex,ey)
		local tile = self.board:getAt(tx,ty,DATA_LEVEL)
		
		if self.board:checkTile(tile) then 
			if tile.name == self.activeColor then 
				local gx,gy = self:translateToGlobal(tx,ty)
				-- UNDO
				if tile == self.prevTile then
					self:popConnection()
					self.sx, self.sy = gx,gy
				-- NEW CONNECTION
				elseif tile ~= self.activeTile and not tile.isConnected then 	
					self.prevTile = self.activeTile
					self.touchTile = tile
					self:updateLine(self.activeLine, gx,gy)
					self.sx, self.sy = gx,gy
					self:pushConnection(tx,ty,tile)
				end
			end
		end
	end
end
--
function GameScene:touchesEnd(e)
	self.activeLine = nil
	self.activeTile = nil
	self.touchTile = nil
	self.activeColor = ""
	
	local n = #self.connections
	while #self.connections > 0 do
		local removed = self:popConnection()
		if n > 2 then 
			self:removeTile(removed.tile)
		end
	end
	self.board:drop()
end