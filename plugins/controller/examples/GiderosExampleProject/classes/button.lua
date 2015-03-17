--[[
A generic button class

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 
]]

Button = gideros.class(Sprite)

function Button:init(upState, downState)
	self.upState = upState
	self.downState = downState
		
	self.focus = false

	self:updateVisualState(false)

	self:addEventListener(Event.MOUSE_DOWN, self.onMouseDown, self)
	self:addEventListener(Event.MOUSE_MOVE, self.onMouseMove, self)
	self:addEventListener(Event.MOUSE_UP, self.onMouseUp, self)

	self:addEventListener(Event.TOUCHES_BEGIN, self.onTouchesBegin, self)
	self:addEventListener(Event.TOUCHES_MOVE, self.onTouchesMove, self)
	self:addEventListener(Event.TOUCHES_END, self.onTouchesEnd, self)
	self:addEventListener(Event.TOUCHES_CANCEL, self.onTouchesCancel, self)
end

function Button:onMouseDown(event)
	if self:hitTestPoint(event.x, event.y) then
		self.focus = true
		self:updateVisualState(true)
		event:stopPropagation()
	end
end

function Button:onMouseMove(event)
	if self.focus then
		if not self:hitTestPoint(event.x, event.y) then
			self.focus = false;
			self:updateVisualState(false)
		end
		event:stopPropagation()
	end
end

function Button:onMouseUp(event)
	if self.focus then
		self.focus = false;
		self:updateVisualState(false)
		self:dispatchEvent(Event.new("click"))
		event:stopPropagation()
	end
end

-- if button is on focus, stop propagation of touch events
function Button:onTouchesBegin(event)
	if self.focus then
		event:stopPropagation()
	end
end

-- if button is on focus, stop propagation of touch events
function Button:onTouchesMove(event)
	if self.focus then
		event:stopPropagation()
	end
end

-- if button is on focus, stop propagation of touch events
function Button:onTouchesEnd(event)
	if self.focus then
		event:stopPropagation()
	end
end

-- if touches are cancelled, reset the state of the button
function Button:onTouchesCancel(event)
	if self.focus then
		self.focus = false;
		self:updateVisualState(false)
		event:stopPropagation()
	end
end

-- if state is true show downState else show upState
function Button:updateVisualState(state)
	if state then
		if self:contains(self.upState) then
			self:removeChild(self.upState)
		end
		
		if not self:contains(self.downState) then
			self:addChild(self.downState)
		end
	else
		if self:contains(self.downState) then
			self:removeChild(self.downState)
		end
		
		if not self:contains(self.upState) then
			self:addChild(self.upState)
		end
	end
end
