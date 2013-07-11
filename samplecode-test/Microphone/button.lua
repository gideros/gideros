--[[
A generic button class

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 
]]

Event.CLICK = "click"

Button = Core.class(Sprite)

function Button:init(upState, downState, disabledState)
	self.upState = upState
	self.downState = downState
	self.disabledState = disabledState or upState
		
	self.focus = false
	self.disabled = false

	-- set the visual state as "up"
	self:updateVisualState(false)

	-- register to all mouse and touch events
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
			self.focus = false
			self:updateVisualState(false)
		end
		event:stopPropagation()
	end
end

function Button:onMouseUp(event)
	if self.focus then
		self.focus = false
		self:updateVisualState(false)
		if not self.disabled then
			self:dispatchEvent(Event.new(Event.CLICK))	-- button is clicked, dispatch "click" event
		end
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
		self.focus = false
		self:updateVisualState(false)
	end
end

-- if state is true show downState else show upState
function Button:updateVisualState(state)
	self.upState:removeFromParent()
	self.downState:removeFromParent()
	self.disabledState:removeFromParent()
	
	if self.disabled then
		self:addChild(self.disabledState)
	else
		if state then
			self:addChild(self.downState)
		else
			self:addChild(self.upState)
		end
	end
end

function Button:setDisabled(disabled)
	if self.disabled == disabled then
		return
	end

	self.disabled = disabled
	self.focus = false
	self:updateVisualState(false)
end

function Button:isDisabled()
	return self.disabled
end

