--!NEEDS:uiinit.lua
local debugClick = _DEBUG_CLICK
if debugClick then print("UI.Behavior debugClick !!!!!!!!!!!!!!!!!!!!!!!!!!") end

--[[
Class			Params			Desc
-----------------------------------------------------------------------------
Button			.Repeat			Repeat speed
				.RepeatSpeedup	Speedup factor (<1 to increase actual speed)
				.NoDoubleClick  Disable double clicking ability (faster response)
ToggleButton	-
RadioButton		Group			Radio group handle/name
LongClick		.szIndicator	Indictor size
				.clsIndicator	Indicator class (responding to UI.Progress)
-----------------------------------------------------------------------------
]]

-- PUSH BUTTON
local b=Core.class(UI.Behavior)
UI.Behavior.Button=b
function b:init(widget,params)
	assert(widget and widget.setFlags and widget.getFlags,"Widget must be a descendant of UI.Panel")
	widget.behavior=self
	self.params=params or {}
	self:clone(widget)
end
function b:clone(widget)	
	self.widget=widget
	self.clickHandler={ handler=self,target=widget }
	if self.params.NoDoubleClick then
		UI.Control.onMouseDown[widget]=self.clickHandler
	else
		UI.Control.onMouseClick[widget]=self.clickHandler
	end
	self:setRepeat(self.params.Repeat)
	self.RepeatSpeedup=self.params.RepeatSpeedup
end
function b:setRepeat(r)
	UI.Control.onLongClick[self.widget]=nil
	UI.Control.onLongDown[self.widget]=nil
	self.Repeat=r
	if r then
		UI.Control.onLongClick[self.widget]=self.clickHandler
		UI.Control.onLongDown[self.widget]=self.clickHandler
	else
		UI.Control.onEnterFrame[self]=nil
	end	
end
function b:onMouseClick(w,x,y,c)
	local wflags=w:getFlags()
	if wflags.disabled then return end
	if debugClick then print("UI.Behavior.Button","onMouseClick",self) end
	if wflags.focusable then UI.Focus:request(w) end
	if self.widget.onClick then self.widget:onClick(c) end
	UI.dispatchEvent(self.widget,"WidgetAction",c)
	return true --stopPropagation !
end
function b:onMouseDown(w,x,y)
	local wflags=w:getFlags()
	if wflags.disabled then return end
	if debugClick then print("UI.Behavior.Button","onMouseDown",self) end
	if wflags.focusable then UI.Focus:request(w) end
	if self.widget.onClick then self.widget:onClick() end
	UI.dispatchEvent(self.widget,"WidgetAction")
	return true --stopPropagation !
end
function b:onLongClick(w,x,y)
	self.repeatNext=nil
	UI.Control.onEnterFrame[self]=nil
	return true 
end
function b:onLongDown(w,x,y)
	UI.Control.onEnterFrame[self]=self
	self.repeatSpd=self.Repeat
end
function b:onEnterFrame()
	local tm=os:timer()
	local w=self.widget
	if self.Repeat and self.repeatNext and tm<self.repeatNext then return true end
	local wflags=w:getFlags()
	if wflags.disabled then return end
	self.repeatNext=tm+self.repeatSpd
	self.repeatSpd*=(self.RepeatSpeedup or 1)
	if wflags.focusable then UI.Focus:request(w) end
	if self.widget.onClick then self.widget:onClick() end
	UI.dispatchEvent(self.widget,"WidgetAction")
	return true --stopPropagation !
end

function b:destroy()
	UI.Control.onMouseClick[self.widget]=nil
	self:setRepeat(nil)
	UI.Focus:relinquish(self.widget)
	self.widget.behavior=nil
	self.widget=nil
end

-- TOGGLE BUTTON
UI.Behavior.ToggleButton=Core.class(UI.Behavior)
function UI.Behavior.ToggleButton:init(widget)
	assert(widget and widget.setFlags and widget.getFlags,"Widget must be a descendant of UI.Panel")
	widget.behavior=self
	self:clone(widget)
end
function UI.Behavior.ToggleButton:clone(widget)
	self.widget=widget
	self.clickHandler={ handler=self,target=widget }
	UI.Control.onMouseClick[widget]=self.clickHandler
end
function UI.Behavior.ToggleButton:onMouseClick(w,x,y) 
	local wflags=w:getFlags()
	if wflags.disabled then return end
	if debugClick then print("UI.Behavior.ToggleButton","onMouseClick",self) end
	UI.Focus:request(w)
	w:setFlags({ticked=not (wflags.ticked)},true)
	UI.dispatchEvent(self.widget,"WidgetAction")
	return true --stopPropagation !
end
function UI.Behavior.ToggleButton:destroy()
	UI.Control.onMouseClick[self.widget]=nil
	UI.Focus:relinquish(self.widget)
	self.widget.behavior=nil
	self.widget=nil
end

-- RADIO BUTTON
UI.Behavior.RadioButton=Core.class(UI.Behavior.ToggleButton,function (widget,group) return widget end)
UI.Behavior.RadioButton.Groups={}
function UI.Behavior.RadioButton:init(widget,group)
	self.group=group or {}
	UI.Behavior.RadioButton.Groups[self.group]=UI.Behavior.RadioButton.Groups[self.group] or {}
	self:clone(widget)
end
function UI.Behavior.RadioButton:clone(widget)
	UI.Behavior.ToggleButton.clone(self,widget)
	UI.Behavior.RadioButton.Groups[self.group][widget]=true
end
function UI.Behavior.RadioButton:onMouseClick(w,x,y) 
	if debugClick then print("UI.Behavior.RadioButton","onMouseClick",self) end
	local kw = nil
	local t = {}
	for k,_ in pairs(UI.Behavior.RadioButton.Groups[self.group]) do
		if k==w then kw=k
		else table.insert(t,k) end
	end
	if kw then
		local wflags=w:getFlags()
		if wflags.disabled then return end
		for _,k in pairs(t) do k:setFlags({ticked=false},true) end --uncheck first --!sure
		kw:setFlags({ticked=true},true) --then check  --!sure
	end
	return true --stopPropagation !
end
function UI.Behavior.RadioButton:destroy()
	UI.Behavior.RadioButton.Groups[self.group][self.widget]=nil
	if not next(UI.Behavior.RadioButton.Groups[self.group]) then
		UI.Behavior.RadioButton.Groups[self.group]=nil
	end
	UI.Behavior.ToggleButton.destroy(self)
end

--Long click
UI.Behavior.LongClick=Core.class(UI.Behavior)
function UI.Behavior.LongClick.makeIndicator(widget,params)
	local prepare=(params.clsIndicator or UI.CircularProgress).new()
	function prepare:indicate(widget,x,y,r)
		local sz=self.size or widget:resolveStyle(params.szIndicator or "progress.szCircular")
		self.size=sz
		if r>0 then
			UI.Screen.addToScreenGlass(widget,self,x,y,sz*2,sz*2,0,0,true,true)
			self:setProgress(r)
		else
			self:removeFromParent()
		end
	end
	return prepare
end

function UI.Behavior.LongClick:init(widget,params)
	assert(widget and widget.setFlags and widget.getFlags,"Widget must be a descendant of UI.Panel")
	--! NO --widget.behavior=self --need register
	self.widget=widget
	self.clickHandler={ handler=self,target=widget }
	self.params=params
	UI.Control.onLongClick[self.widget]=self.clickHandler
	UI.Control.onLongPrepare[self.widget]=self.clickHandler
end
function UI.Behavior.LongClick:onLongClick(w,x,y)
	local wflags=w:getFlags()
	if wflags.disabled then return end
	UI.Focus:request(w)
	if self.widget.onLongClick then self.widget:onLongClick() end
	UI.dispatchEvent(self.widget,"WidgetLongAction")
	return true 
end
function UI.Behavior.LongClick:onLongPrepare(w,x,y,r)
	local wflags=w:getFlags()
	if wflags.disabled then return end
	if not self.prepare then 
		self.prepare=self.makeIndicator(self.widget,self.params) 
	end
	self.prepare:indicate(self.widget,x,y,r)
	return true
end
function UI.Behavior.LongClick:destroy()
	UI.Control.onLongClick[self.widget]=nil
	UI.Control.onLongPrepare[self.widget]=nil
	UI.Focus:relinquish(self.widget)
	--! NO --self.widget.behavior=nil
	self.widget=nil
end


--Linger click
UI.Behavior.Linger=Core.class(UI.Behavior)
function UI.Behavior.Linger:init(widget,params)
	assert(widget and widget.setFlags and widget.getFlags,"Widget must be a descendant of UI.Panel")
	widget.behavior=self
	self.widget=widget
	self.clickHandler={ handler=self,target=widget }
	self.params=params
	UI.Control.onLingerStart[self.widget]=self.clickHandler
	UI.Control.onLingerEnd[self.widget]=self.clickHandler
	UI.Control.onMouseDown[self.widget]=self.clickHandler
	UI.Control.onMouseUp[self.widget]=self.clickHandler
end
function UI.Behavior.Linger:clone(widget)	
	self.widget=widget
	self.clickHandler={ handler=self,target=widget }
	UI.Control.onLingerStart[self.widget]=self.clickHandler
	UI.Control.onLingerEnd[self.widget]=self.clickHandler
	UI.Control.onMouseDown[self.widget]=self.clickHandler
	UI.Control.onMouseUp[self.widget]=self.clickHandler
end
function UI.Behavior.Linger:onLingerStart(w,x,y)
	local wflags=w:getFlags()
	if wflags.disabled then return end
	UI.dispatchEvent(self.widget,"WidgetLingerStart",x,y)
	return true 
end
function UI.Behavior.Linger:onLingerEnd(w,x,y)
	local wflags=w:getFlags()
	if wflags.disabled then return end
	UI.dispatchEvent(self.widget,"WidgetLingerEnd",x,y)
	return true
end
UI.Behavior.Linger.onMouseDown=UI.Behavior.Linger.onLingerStart
UI.Behavior.Linger.onMouseUp=UI.Behavior.Linger.onLingerEnd
function UI.Behavior.Linger:destroy()
	UI.Control.onLingerStart[self.widget]=nil
	UI.Control.onLingerEnd[self.widget]=nil
	UI.Control.onMouseDown[self.widget]=nil
	UI.Control.onMouseUp[self.widget]=nil
	self.widget.behavior=nil
	self.widget=nil
end