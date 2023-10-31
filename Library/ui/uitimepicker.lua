--!NEEDS:uipanel.lua
--!NEEDS:uilabel.lua
--!NEEDS:uicombobox.lua
--!NEEDS:uilocale.lua
UI.EditableClock=Core.class(UI.Panel,function() return nil end)

UI.EditableClock.Template={
	class="UI.EditableClock", 
	layoutModel={ rowWeights={1}, columnWeights={1}, columnWidths={ "editableclock.szClock" }, rowHeights={ "editableclock.szClock" } },
	LocalStyle="editableclock.styBase",
	children={

	}}
	

local function currentTime()
    local localdate = os.date("*t", os.time())
    return localdate.hour*3600+localdate.min*60+localdate.sec
end
	
function UI.EditableClock:init()
	UI.BuilderSelf(UI.EditableClock.Template,self)
	
	self.pback=Particles.new()
	self:addChild(self.pback)
	self.labels={}
	for i=1,12 do
		local lt=UI.Label.new(tostring(i),nil,"editableclock.fontLabels")
		lt:setStyle("editableclock.styLabel")
		self:addChild(lt)
		self.labels[i]=lt
	end
	
	self.handH=self:makeHandle("editableclock.styHandH")
	self.handM=self:makeHandle("editableclock.styHandM")
	self.pfront=Particles.new()
	self:addChild(self.pfront)
	
	--Particles
	self.pback:addParticles({
			{ x=0,y=0, size=1, color=0, alpha=0 },
			{ x=0,y=0, size=1, color=0, alpha=0 },
		})
	self.pfront:addParticles({
			{ x=0,y=0, size=1, color=0, alpha=0 },
		})


	self:setTime(currentTime())
	
	self:addEventListener(Event.LAYOUT_RESIZED,self.onLayout,self)
	self:onLayout()
	
	--Control
	UI.Control.onDragStart[self]=self
	UI.Control.onDragEnd[self]=self
	UI.Control.onDrag[self]=self
end

function UI.EditableClock:onLayout(e)
	local w,h=self:getDimensions()
	self.pback:setPosition(w/2,h/2)
	self.pfront:setPosition(w/2,h/2)
end

function UI.EditableClock:setLabels(first,step,mod)
	local n=first
	for i=1,12 do
		self.labels[i]:setText(n)
		n+=step
		n=n%mod
	end	
end
	
function UI.EditableClock:makeHandle(style)
	local hand=UI.Panel.new()
	hand.style=style
	hand:setStyle("editableclock.styHandBase")
	hand:setBaseStyle(style)
	hand:setStyleInheritance("state")
	hand:setLayoutConstraints({
			height="hand.szHand",
			width="hand.szWidth",
			originy=.5,
			originx=.5,
		})
	local handle=UI.Panel.new()
	handle:setStyle("handle.styBase")
	handle:setAnchorPoint(.5,.5)
	hand.handle=handle
	hand:addChild(handle)
	
	local arrow=UI.Panel.new()
	arrow:setStyle("handle.styArrows")
	arrow:setAnchorPoint(.5,.5)
	handle.arrow=arrow
	handle:addChild(arrow)
	
	hand.setAngle=function(self,a)
		self:setRotation(a+180)
		self.handle:setRotation(-a-180)
		self.handle.arrow:setRotation(a+180)
	end
	self:addChild(hand)
	return hand
end


function UI.EditableClock:updateStyle(fromParent)
	UI.Panel.updateStyle(self,fromParent)
	local function updateHandle(hand)
		local w=hand:resolveStyle("hand.szWidth")
		local o=hand:resolveStyle("hand.szOffset")
		hand:setAnchorPosition(w/2,o)
		local wh=hand:resolveStyle("handle.szHandle")
		local oh=hand:resolveStyle("handle.szOffset")
		hand.handle:setPosition(w/2,oh)
		hand.handle:setDimensions(wh,wh)
		hand.handle.arrow:setDimensions(wh,wh)
	end
	updateHandle(self.handH)
	updateHandle(self.handM)
	local szRing,szText,szCenter,szDot,szClock=
		self:resolveStyle("editableclock.szRing"),
		self:resolveStyle("editableclock.szText"),
		self:resolveStyle("editableclock.szCenter"),
		self:resolveStyle("editableclock.szDot"),
		self:resolveStyle("editableclock.szClock")
	self.pback:setParticleSize(1,szRing)
	self.pback:setParticleSize(2,szCenter)
	self.pfront:setParticleSize(1,szDot)
	self.pback:setParticleColor(1,UI.Utils.colorSplit("editableclock.colRing",self._style))
	self.pback:setParticleColor(2,UI.Utils.colorSplit("editableclock.colCenter",self._style))
	self.pfront:setParticleColor(1,UI.Utils.colorSplit("editableclock.colDot",self._style))
	
	local r=szText/szClock	
	for i=0,11 do
		local ang=^<(i*30)
		self.labels[i+1]:setLayoutConstraints({anchorx=.5+(math.sin(ang)/2)*r,anchory=.5-(math.cos(ang)/2)*r})
	end		
end

function UI.EditableClock:newClone() assert(false,"Cloning not supported") end

function UI.EditableClock:onWidgetChange(w,v)
	if self:getFlags().disabled then return end
end

function UI.EditableClock:onDragStart(x,y,ed,ea,change,long)
	if long then return end
    local eb=self:getChildrenAtPoint(x,y,true,false,self)
	local cw,ch=self:getDimensions()
	local ma=^>math.atan2(x-cw/2,-y+ch/2)
	for _,spr in ipairs(eb) do
		if spr==self.handH.handle then
			self.dragging={ h=spr:getParent(), div=3600, mod=12, ang=30, pa=ma, pm=true }
		elseif spr==self.handM.handle then
			self.dragging={ h=spr:getParent(), div=60, mod=60, ang=6, pa=ma }
			self:setLabels(0,5,60)
		end
	end
	if self.dragging then
		self.dragging.h:setStateStyle("editableclock.styHandSel")
		self.handH.handle.arrow:setVisible(false)
		self.handM.handle.arrow:setVisible(false)
	end
	return true
end
function UI.EditableClock:onDrag(x,y)
	if self.dragging then
		local cw,ch=self:getDimensions()
		
		local ma=^>math.atan2(x-cw/2,-y+ch/2)+360
		local v=(((ma%360)/self.dragging.ang+.5)//1)%self.dragging.mod
		local ov=(self.time//self.dragging.div)%self.dragging.mod
		local dif=(v-ov)/self.dragging.mod
		local cross=false
		if dif>.5 then cross=true end
		if dif<-.5 then cross=true end
		if self.dragging.pm and cross then
			if dif>0 then
				v+=12
			else
				v-=12
			end
			v=v%24
		end
		local nt=self.time+(v-ov)*self.dragging.div
		if self.time~=nt then
			self:setTime(nt)
			UI.dispatchEvent(self,"WidgetChange",self.time)
		end
	end
	return self.dragging
end
function UI.EditableClock:onDragEnd(x,y)
	if self.dragging then
		self.dragging.h:setStateStyle()
		self.dragging=nil
	end
	self.pm=nil
	self:setTime(self.time)
	self.handH.handle.arrow:setVisible(true)
	self.handM.handle.arrow:setVisible(true)
end

function UI.EditableClock:setTime(time)
	self.time=(time//1)%86400
	local m=time//60
	local h=(m//60)%24
	local pm=h>=12
	if pm~=self.pm and (not self.dragging or self.dragging.pm) then
		self.pm=pm
		if pm then
			self:setLabels(12,1,24)
		else
			self:setLabels(0,1,24)
		end
	end
	self.handH:setAngle((h%12)*30)
	self.handM:setAngle((m%60)*6)
end

function UI.EditableClock:getTime()
	return self.time
end

UI.EditableClock.Definition= {
	name="EditableClock",
	class="UI.EditableClock",
	icon="ui/icons/label.png",
	constructorArgs={},
	properties={
		{ name="Date", type="date" },
	},
}


--- Date picker
UI.TimePicker=Core.class(UI.PopupButton,function (allowNil) return nil end)
UI.TimePicker.Template={
	class="UI.PopupButton",	
	BaseStyle="combobox.styBase",
	children={
		{ class="UI.ButtonTextFieldCombo", 
		layout={fill=Sprite.LAYOUT_FILL_BOTH, width="timepicker.szWidth",}, 
		TextLayout={flags=FontBase.TLF_SINGLELINE|FontBase.TLF_REF_LINETOP | FontBase.TLF_VCENTER}, 
		Editable=false,
		name="editor",
		Style={ ["buttontextfieldcombo.icButton"]="timepicker.icPicker" },
		}
	},
	--ContentOffset={-10,20}, --Placement margin account for border and spacing
	Content={
		class="UI.Panel",
		name="popup",
		layoutModel={ },
		children={{
			class="UI.EditableClock",
			name="clEditableClock",
			layout={ fill=Sprite.LAYOUT_FILL_BOTH}
		}}
	},
}

--- Combobox object

function UI.TimePicker:init(allowNil)
	UI.BuilderSelf(UI.TimePicker.Template,self)
	function self.popup.onWidgetChange(s,w,time)
		if w==self.clEditableClock then
			self:setTime(time)
			return true
		end
	end
	self.editor:setLocalStyle("timepicker.styNormal")
	self.editor:setTipText(UI._LO("UI.TimePicker.Format"))
	self.allowNil=allowNil
	if not allowNil then
		self:setTime(os.time())
	end
end

function UI.TimePicker:setTime(time)
	if not time and not self.allowNil then time=currentTime() end
	if time then
		self.clEditableClock:setTime(time)
		local h=(time//3600)%24
		local m=(time//60)%60
		self.editor:setText(("%02d:%02d"):format(h,m))
	else
		self.editor:setText("")
	end
	--self:setFlags({ticked=false})
	--self.editor.button:setFlags({ticked=false})
	--UI.Focus:relinquish(self)
end

function UI.TimePicker:getTime()
	local t=self.editor:getText()
	return if #t>0 and self:checkTime(t) then self.clEditableClock:getTime() else nil
end

function UI.TimePicker:onFocusChange(w,v)
	if w==self.editor then 
		if self.editor:getFlags().focused then
			self:setFlags({ticked=false})
			self.editor.button:setFlags({ticked=false})
		else
			local t=self.editor:getText()
			local et=self:checkTime(t)
			if et then
				if #t==0 then et=nil end
				self:setTime(et) 
				UI.dispatchEvent(self,"WidgetChange",et)
			end
		end
	end
end 


function UI.TimePicker:setFlags(c)
	UI.PopupButton.setFlags(self,c)
	local readonly = self:getFlags().readonly
	if readonly then
		if c.disabled~=nil or c.readonly~=nil or c.focused~=nil then self.editor:setFlags({disabled=c.disabled, readonly=c.readonly, focused=c.focused}) end
	else --TOSEE focused? ticked?
		if c.disabled~=nil or c.readonly~=nil then self.editor:setFlags({disabled=c.disabled, readonly=c.readonly}) end
	end
	if c.ticked==nil then return end
	local ticked=self:getFlags().ticked
	if readonly then
		if not ticked then self.editor:setFlags({ticked=ticked}) end
	else --TOSEE ticked?
		
	end
	if ticked then
		self.preEdit=self:getTime()
	--self.editor:focus() 
	--self.editor.button:setFlags({ticked=false})
	else
		self.editor.button:setFlags({ticked=false})
		local value=self:getTime()
		if value~=self.preEdit then
			UI.dispatchEvent(self,"WidgetChange",value)
		end
		self.preEdit=nil
	end
end

function UI.TimePicker:onWidgetAction(w)
	if w==self.editor.button then
		local ticked = w:getFlags().ticked
		self:setFlags({ticked=ticked})
	end
end

function UI.TimePicker:checkTime(d)
	if self.allowNil and #d==0 then return 0 end
	local f1,f2=d:match("(%d+):(%d+)")
	local s=0
	local h=tonumber(f1)
	local m=tonumber(f2)
	if not h or h<0 or h>23 then return end
	if not m or m<0 or m>59 then return end
	if not s or s<0 or s>59 then return end
	return h*3600+m*60+s
end
	
function UI.TimePicker:onTextChange(w,t)
	if w==self.editor then
		self.editor:setLocalStyle(if not self:checkTime(t) then "timepicker.styError" else "timepicker.styNormal")
	end
end

function UI.TimePicker:onTextValid(w,t)
	if w==self.editor then
		local et=self:checkTime(t)
		if et then
			if #t==0 then et=nil end
			self:setTime(et) 
			UI.dispatchEvent(self,"WidgetChange",et)
		end
	end
end


UI.TimePicker.Definition= {
  name="TimePicker",
  icon="ui/icons/panel.png",
  class="UI.TimePicker",
  constructorArgs={ "AllowNil"},
  properties={
	  { name="AllowNil", type="boolean" },
  },
}