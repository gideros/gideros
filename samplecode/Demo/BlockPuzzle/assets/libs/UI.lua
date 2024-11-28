local Utils = Game.Utils

UI = Core.class(Sprite)
UI.null = function() end

function UI:init(overrideEvents)
	if (not overrideEvents) then 
		self:addEventListener(Event.TOUCHES_BEGIN, self.touch, self)
		self:addEventListener(Event.TOUCHES_MOVE, self.touch, self)
		self:addEventListener(Event.TOUCHES_END, self.touch, self)
		self:addEventListener(Event.TOUCHES_CANCEL, self.touch, self)
	end
end
--
function UI:touch(e)
	local n = self:getNumChildren()
	for i = n, 1, -1 do 
		local child = self:getChildAt(i)
		
		if (child.touch and child:touch(e)) then 
			return true
		end
	end
	return false
end
------------------------------------------------------------------------
------------------------------------------------------------------------
local Base = Core.class(Sprite)
UI.Base = Base

function Base:init(t)
	self.callback = t.callback or UI.null
	self:createGFX(t)
end
--
function Base:createGFX(t)
	if (not t.r) then 
		assert(t.w and t.h, "[Base]: Size is not set")
		self.w = t.w
		self.h = t.h
	else
		self.w = t.r * 2
		self.h = self.w
	end
	
	if (t.image) then 
		if (t.ninePatch) then 
			self.iType = "pixel"
			self.gfx = Pixel.new(t.image, self.w, self.h)
			self.gfx:setNinePatch(unpack(t.ninePatch))
		else
			self.iType = "btm"
			self.gfx = Bitmap.new(t.image)
		end
	else
		self.iType = "pixel"
		self.gfx = Pixel.new(0xffffff, 1, self.w, self.h)
	end
	self:addChild(self.gfx)
	
	if (t.icon) then 
		self.icon = Bitmap.new(t.icon)
		self:addChild(self.icon)
	end
end
--
function Base:setColor(c, a)
	if (self.iType == "pixel") then 
		self.gfx:setColor(c,a)
	else
		local r,g,b = Utils.hex2rgb(c)
		self.gfx:setColorTransform(r / 255, g / 255, b / 255)
		self.gfx:setAlpha(a or 1)
	end
end
--
function Base:setIconColor(c, a)
	assert(self.icon, "[Base]: icon is not set")
	local r,g,b = Utils.hex2rgb(c)
	self.icon:setColorTransform(r / 255, g / 255, b / 255)
	self.icon:setAlpha(a or 1)
end
--
function Base:setAnchorPoint(x, y)
	local n = self:getNumChildren()
	for i = n, 1, -1 do 
		local child = self:getChildAt(i)
		child:setAnchorPoint(x, y)
	end
end
------------------------------------------------------------------------
------------------------------------------------------------------------
local Button = Core.class(Base)
UI.Button = Button

function Button:init(t)
	self.form = t.form or "r" -- r - rectangle, c - circle
	if (self.form == "c") then 
		assert(t.r, "[Button]: radius must be a number")
		self.r = t.r
	end
end
--
function Button:touch(e)
	local x, y = e.touch.x, e.touch.y
	if (e.type == "touchesEnd") then 
		if (self.form == "r") then 
			if (self:hitTestPoint(x, y)) then 
				self:callback()
				return true
			end
		else
			x, y = self:globalToLocal(x, y)
			--if (x*x+y*y <= self.r*self.r) then 
			if (Utils.dist2(0,0,x,y) <= self.r*self.r) then 
				self:callback()
				return true
			end
		end
	end
end
------------------------------------------------------------------------
------------------------------------------------------------------------
local TextButton = Core.class(Button)
UI.TextButton = TextButton

function TextButton:init(t)
	self.text = Label.new(t)
	self:setTextColor(t.color)
	self:addChild(self.text)
end
--
function TextButton:setTextColor(c)
	self.text:setTextColor(c or 0)
end
------------------------------------------------------------------------
------------------------------------------------------------------------
local Panel = Core.class(Base)
UI.Panel = Panel

function Panel:init(t)
	self.margin = t.margin or 0
	self.offset = t.offset or 0
	self.orientation = t.orientation or "v" -- v - vertical, h - horizontal
	self.container = Sprite.new()
	self.container:setPosition(self.margin, self.margin)
	self.container:setClip(0,0,self.w - self.margin*2,self.h - self.margin*2)
	self:addChild(self.container)
end
--
function Panel:add(obj)
	local n = self.container:getNumChildren()
	if (n == 0) then 
		self.container:addChild(obj)
	else
		local last = self.container:getChildAt(n)
		local newX = last:getX()
		local newY = last:getY() + last:getHeight() + self.offset
		if (self.orientation == "h") then 
			newX = last:getX() + last:getWidth() + self.offset
			newY = last:getY()
		end
		
		obj:setPosition(newX, newY)
		self.container:addChild(obj)
	end
end
--
function Panel:hit(x, y)
	return not (x < self.margin or x > self.w - self.margin or y < self.margin or y > self.h - self.margin)
end
--
function Panel:touch(e)
	local n = self.container:getNumChildren()
	local x, y = self:globalToLocal(e.touch.x, e.touch.y)
	if (self:hit(x, y)) then
		for i = n, 1, -1 do 
			local child = self.container:getChildAt(i)
			
			if (child.touch and child:touch(e)) then 
				return true
			end
		end
	end
	return false
end