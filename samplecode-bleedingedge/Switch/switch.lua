--[[
A generic switch class

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 
]]

Switch = Core.class(Sprite)

function Switch:init(offState, onState)
	self.offState = offState
	self.onState = onState
		
	self.focus = false

	-- set the visual state as "off"
	self.state = false
	self:updateVisualState()

	-- register to all mouse and touch events
	self:addEventListener(Event.MOUSE_DOWN, self.onMouseDown, self)
	self:addEventListener(Event.MOUSE_MOVE, self.onMouseMove, self)
	self:addEventListener(Event.MOUSE_UP, self.onMouseUp, self)

	self:addEventListener(Event.TOUCHES_BEGIN, self.onTouchesBegin, self)
	self:addEventListener(Event.TOUCHES_MOVE, self.onTouchesMove, self)
	self:addEventListener(Event.TOUCHES_END, self.onTouchesEnd, self)
	self:addEventListener(Event.TOUCHES_CANCEL, self.onTouchesCancel, self)
end

function Switch:getState()
	return self.state
end

function Switch:setState(state)
	self.state = state
	self:updateVisualState()
	self.focus = false
end

function Switch:onMouseDown(event)
	if self:hitTestPoint(event.x, event.y) then
		self.focus = true
		event:stopPropagation()
	end
end

function Switch:onMouseMove(event)
	if self.focus then
		if not self:hitTestPoint(event.x, event.y) then	
			self.focus = false
		end
		event:stopPropagation()
	end
end

function Switch:onMouseUp(event)
	if self.focus then
		self.focus = false
		self.state = not self.state
		self:updateVisualState()
		local event = Event.new("change")
		event.state = self.state
		self:dispatchEvent(event)	-- state is changed, dispatch "change" event		
		event:stopPropagation()
	end
end

-- if button is on focus, stop propagation of touch events
function Switch:onTouchesBegin(event)
	if self.focus then
		event:stopPropagation()
	end
end

-- if button is on focus, stop propagation of touch events
function Switch:onTouchesMove(event)
	if self.focus then
		event:stopPropagation()
	end
end

-- if button is on focus, stop propagation of touch events
function Switch:onTouchesEnd(event)
	if self.focus then
		event:stopPropagation()
	end
end

-- if touches are cancelled, reset the state of the switch
function Switch:onTouchesCancel(event)
	if self.focus then
		self.focus = false
		self.state = false
		self:updateVisualState()
		event:stopPropagation()
	end
end

-- if self.state is true show onState else show offState
function Switch:updateVisualState()
	self.offState:removeFromParent()
	self.onState:removeFromParent()
	self:addChild(self.state and self.onState or self.offState)
end
