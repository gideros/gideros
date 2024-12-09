Board = Core.class(Grid)

function Board:init()
	self.spawnZones = {}
	self.linesCount = 1
	
	self.state = ""
	self.stateTimer = Timer.new(0,1)
	self.stateTimer:addEventListener("timerComplete", function()
		self.state = ""
	end)
	print(Event.TIMER)
	print(Event.TIMER_COMPLETE)
end
--
function Board:load(data)
	local h = #data.cells
	local w = #data.cells[1]
	
	if data.spawn == nil or data.spawn ~= nil and #data.spawn == 0 then 
		for i = 1, w do 
			self.spawnZones[i] = {i,1}
		end
	else
		for i,t in ipairs(data.spawn) do 
			self.spawnZones[i] = t
		end
	end
	
	self:setSize(w,h,BRICK_LEVEL)
	self:iterate2D(1, function(x,y)
		local v = data.cells[y][x]		
		if v == 1 then 
			local tile = Tile.new("brick")
			self:add(x,y,BRICK_LEVEL,tile)
			self:setAt(x,y,DATA_LEVEL,1)
		elseif v == 0 then 
			if (x+y)%2 == 0 then
				local bgTile = Tile.new("boardTileRounded")	
				self:add(x,y,BG_LEVEL,bgTile)
			end
			self:setAt(x,y,DATA_LEVEL,0)
		end		
	end)
	
end
--
function Board:checkTile(tile)
	return tile and tile ~= 0 and tile ~= 1
end
--
function Board:isSpawnPoint(x,y)
	for i,points in ipairs(self.spawnZones) do 
		if points[1] == x and points[2] == y then return true end
	end
end
--
function Board:getSpawnStack(stack)
	stack = stack or {}
	local justSpawned = {}
	
	for i,t in ipairs(self.spawnZones) do 
		local x = t[1]
		local y = t[2]
		
		local v = self:getAt(x,y,DATA_LEVEL)
		if v == 0 then 
			local clr = choose("2","3","4","5","6")
			local tile = Tile.new(clr,true)
			tile:setAlpha(0)
			tile:setSleep(true)
			tile.tx = x
			tile.ty = y
			
			local sx,sy = self:toGlobal(x,y-1)
			local gx,gy = self:toGlobal(x,y)
			tile:addWayPoint(gx,gy)
			tile:setPosition(sx,sy)
			--self:setAt(x,y,2,tile)
			self:setAt(x,y,DATA_LEVEL,tile)
			
			self:addChild(tile)
			stack[#stack+1] = tile
			justSpawned[#justSpawned+1] = tile
		end
	end
	for i,tile in ipairs(justSpawned) do 
		Timer.delayedCall(self.linesCount * DELAY_SCALE, function()
			tile:runFadeOUT()
			tile:move()
		end)
	end
	justSpawned = nil
end
--
function Board:dropStack(stack)
	local i = 1
	while #stack > 0 do 
		local tile = stack[i]
		
		if tile then 
			if self:dropTile(tile) then 
				i += 1
			else	
				table.remove(stack,i)
			end
		else
			i += 1
		end
		if i > #stack then 
			i = 1 
			self.linesCount += 1
			self:getSpawnStack(stack)
		end
	end
end
--
function Board:spawn()
	self.state = "SPAWN"
	local stack = {}
	self.linesCount = 1
	
	self:getSpawnStack(stack)
	
	if #stack > 0 then 
		self:dropStack(stack)
		self.stateTimer:setDelay(self.linesCount * DELAY_SCALE)
		self.stateTimer:reset()
		self.stateTimer:start()
	else
		self.state = ""
	end
end
--
function Board:checkTopTile(xdir, tile)
	local x = tile.tx + xdir
	for y = tile.ty, 1, -1 do 
		local v = self:getAt(x,y,DATA_LEVEL)
		if v == 1 then 
			return true
		else
			if self:checkTile(v) or self:isSpawnPoint(x,y) then return false end
		end
	end
	return true
end
--
function Board:dropTile(tile)
	local down = self:getAt(tile.tx, tile.ty + 1, DATA_LEVEL)
	if down == 0 then 
		local px,py = tile.tx,tile.ty
		tile.ty += 1
		self:swap(px,py,DATA_LEVEL, tile.tx, tile.ty,DATA_LEVEL)
		
		local gx,gy = self:toGlobal(tile.tx, tile.ty)
		tile:addWayPoint(gx,gy)
	elseif (down == 1 or (self:checkTile(down))) then 
		local left = self:getAt(tile.tx - 1, tile.ty, DATA_LEVEL)
		local down_left = self:getAt(tile.tx - 1, tile.ty + 1, DATA_LEVEL)
		if ((left == 0 or left == 1) and down_left == 0) and self:checkTopTile(-1,tile) then 
			local px,py = tile.tx,tile.ty
			tile.tx -= 1
			tile.ty += 1
			self:swap(px,py,DATA_LEVEL, tile.tx, tile.ty,DATA_LEVEL)
			
			local gx,gy = self:toGlobal(tile.tx, tile.ty)
			tile:addWayPoint(gx,gy)	
		else
			local right = self:getAt(tile.tx + 1, tile.ty, DATA_LEVEL)
			local down_right = self:getAt(tile.tx + 1, tile.ty + 1, DATA_LEVEL)
			if ((right == 0 or right == 1) and down_right == 0) and self:checkTopTile(1,tile) then 
				local px,py = tile.tx,tile.ty
				tile.tx += 1
				tile.ty += 1		
				self:swap(px,py,DATA_LEVEL, tile.tx, tile.ty,DATA_LEVEL)
				
				local gx,gy = self:toGlobal(tile.tx, tile.ty)
				tile:addWayPoint(gx,gy)
			else
				return false
			end
		end
	else
		return false
	end
	return true
end
--
function Board:removeTile(x,y)	
	local lx,ly = self:globalToLocal(x,y)
	local tx,ty = self:toTile(lx,ly)
	self:remove(tx,ty,DATA_LEVEL)
end
--
function Board:drop()
	self.state = "DROP"
	local stack = {}
	local count = 0
	for y = self.h-1,1,-1 do 
	--for y = 1,self.h-1 do 
		for x = 1,self.w do 
			local tile = self.data[DATA_LEVEL][y][x]
			if self:checkTile(tile) then 
				if self:dropTile(tile) then 
					stack[#stack+1] = tile
					count += 1
				end
			end
		end
	end
	if count > 0 then self:drop() end
	
	for k,tile in ipairs(stack) do 
		tile:move()
	end
	self:spawn()
end
--
function Board:clear()
	self:iterate2D(DATA_LEVEL,function(x,y,v)
		self:remove(x,y,DATA_LEVEL)
	end)
end
--