Grid = Core.class(Sprite)
Grid.OUT_OF_RANGE = "[Grid]: out of range"

function Grid:init(t)
	self.tileW = t.tileW or 1
	self.tileH = t.tileH or self.tileW
	
	self.offsetX = t.offsetX or 0
	self.offsetY = t.offsetY or self.offsetX
	
	self.margin = t.margin or 0
	
	self.ax = t.ax or 0
	self.ay = t.ay or 0
	
	self.w = 1
	self.h = 1
	self.z = 1
	self.data = {}
	
	if (t.w and t.h and t.w > 0 and t.h > 0) then 
		self:setSize(t.w, t.h, t.z)
	end
end
--
function Grid:setSize(w, h, z)
	z = z or 1
	for z = 1, z do 
		if (not self.data[z]) then self.data[z] = {} end
		for y = 1, h do 
			if (not self.data[z][y]) then self.data[z][y] = {} end
			for x = 1, w do 
				if (not self.data[z][y][x]) then self.data[z][y][x] = 0 end
			end
		end
	end
	self.w = w
	self.h = h
	self.z = z
end
-- z = 1
function Grid:load(data)
	self.data = {{}}
	local h = #data
	local w = #data[h]
	for y = 1, h do 
		self.data[1][y] = {}
		for x = 1, h do 	
			self.data[1][y][x] = data[y][x]
		end	
	end
end
--
function Grid:toGlobal(tx, ty)
	return 
		self.margin + self.offsetX / 2 + self.tileW * self.ax + (self.tileW + self.offsetX) * (tx - 1),
		self.margin + self.offsetY / 2 + self.tileH * self.ay + (self.tileH + self.offsetY) * (ty - 1)
end
-- NOTE: (x,y) MUST be in Grid local coordinate system
function Grid:toLocal(x, y)
	return 
		1 + (x - self.margin) // (self.tileW + self.offsetX),
		1 + (y - self.margin) // (self.tileH + self.offsetY)
end
--
function Grid:isValid(tx, ty, tz)
	tz = tz or 1
	return not (
	tx < 1 or tx > self.w or 
	ty < 1 or ty > self.h or 
	tz < 1 or tz > self.z)
end
--
function Grid:getAt(tx, ty, tz)
	tz = tz or 1
	if (self:isValid(tx, ty, tz)) then 
		return self.data[tz][ty][tx]
	end
end
--
function Grid:add(tx, ty, tz, obj)
	assert(self:isValid(tx, ty, tz), Grid.OUT_OF_RANGE)
	obj:setPosition(self:toGlobal(tx, ty))
	self:addChild(obj)
	self.data[tz][ty][tx] = obj
end
--
function Grid:remove(tx, ty, tz)
	assert(self:isValid(tx, ty, tz), Grid.OUT_OF_RANGE)
	local v = self.data[tz][ty][tx]
	if (v and v ~= 0) then 
		v:removeFromParent()
	end
	self.data[tz][ty][tx] = 0
	return v
end
--
function Grid:iterate2D(z, callback)
	local flag = false
	for y = 1, self.h do 
		for x = 1, self.w do 
			flag = callback(x, y, self:getAt(x, y, z))
			if (flag) then break end
		end
		if (flag) then break end
	end
end
--
function Grid:iterate(callback)
	for z = 1, self.z do 
		for y = 1, self.h do 
			for x = 1, self.w do 
				flag = callback(x, y, self:getAt(x, y, z))
				if (flag) then break end
			end
			if (flag) then break end
		end
		if (flag) then break end
	end
end
--
function Grid:getWidth()
	return self.margin * 2 + self.w * (self.tileW + self.offsetX)
end
--
function Grid:getHeight()
	return self.margin * 2 + self.h * (self.tileH + self.offsetY)
end