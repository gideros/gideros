--!NEEDS:uipanel.lua
--!NEEDS:uilabel.lua
UI.WeekSchedule=Core.class(UI.Panel,function() return nil end)

function UI.WeekSchedule:init(inverted)
	self.inverted=inverted
	self.bitmap={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	self.rset={}
	self:setStyleInheritance("state")
	local cs="weekschedule.szCell"
	self:setLayoutParameters({
		columnWeights={0,1,1,1,1,1,1,1},
		columnWidths={0,cs,cs,cs,cs,cs,cs,cs},
		cellSpacingX="weekschedule.szCellSpacing",
		rowWeights={0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		rowHeights={0,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs,cs},
		equalizeCells=true
	})
	self.top={}
	local l=UI.Panel.new("weekschedule.colGridFirst")
	l:setLayoutConstraints{ fill=Sprite.LAYOUT_FILL_HORIZONTAL, gridx=0, gridwidth=8, gridy=1, minHeight="weekschedule.szCellSpacing", anchory=0,originy=-1}
	self:addChild(l)
	table.insert(self.top,l)
	for i=1,24 do
		l=UI.Panel.new()
		l:setLayoutConstraints{ gridx=0, gridy=i, anchorx=1, anchory=0, insetRight="weekschedule.szCellSpacing", insetTop="weekschedule.szCellSpacing" }
		l:setLayoutParameters{}
		l:setStyle("weekschedule.styLabel")
		self:addChild(l)
		local lt=UI.Label.new(("%02d:00"):format(i-1),nil,"weekschedule.fontLabel")
		lt:setLayoutConstraints{fill=1 }
		l:addChild(lt)
		
		l=UI.Panel.new("weekschedule.colGrid")
		l:setLayoutConstraints{ fill=Sprite.LAYOUT_FILL_HORIZONTAL, gridx=0, gridwidth=8, gridy=i+1, minHeight=1, anchory=0,originy=-.5}
		self:addChild(l)
		table.insert(self.top,l)
	end
	for i=1,7 do
		l=UI.Panel.new()
		l:setLayoutConstraints{ gridx=i, fill=1 }
		l:setLayoutParameters{}
		l:setStyle("weekschedule.styHeader")
		self:addChild(l)
		local lt=UI.Label.new(UI._LO("DAY"..i))
		lt:setLayoutConstraints{ gridx=i, gridy=0, insets=4}
		l:addChild(lt)
		
		l=UI.Panel.new("weekschedule.colCell")
		l:setLayoutConstraints{ fill=Sprite.LAYOUT_FILL_BOTH, gridx=i, gridheight=24, gridy=1}
		self:addChild(l)
	end
--[[	local l=UI.Panel.new("weekschedule.colGrid")
	l:setLayoutConstraints{ fill=Sprite.LAYOUT_FILL_VERTICAL, gridx=8, gridheight=24, gridy=1, minWidth=1, anchorx=0, originx=-.5}
	self:addChild(l)]]
	self:redraw()
	--Control
	UI.Control.onMouseClick[self]=self
	UI.Control.onDragStart[self]=self
	UI.Control.onDrag[self]=self
	UI.Control.onDragEnd[self]=self
end

function UI.WeekSchedule:newClone() assert(false,"Cloning not supported") end

function UI.WeekSchedule:onDragStart(x,y,ed,ea,change,long)
	if self:getFlags().disabled then return end
	if long then return end
	UI.Focus:request(self)
	local fillCtx=self:getLayoutInfo()
	local function getFillCell(x,y)
		local col=0
		while col<8 and x>fillCtx.minWidth[col+1] do
			x-=fillCtx.minWidth[col+1]	
			col+=1
		end
		if col==0 or col==8 then return end
		local row=0
		while row<25 and y>fillCtx.minHeight[row+1] do
			y-=fillCtx.minHeight[row+1]	
			row+=1
		end
		if row==0 or row==25 then return end
		return row,col-1
	end
	local sOff,sBit=getFillCell(x,y)
	if sOff==nil then return end
	local fillValue=1-((self.bitmap[sOff]>>sBit)&1)
	self.dragFill=function(x,y)
		local dOff,dBit=getFillCell(x,y)
		for k,v in ipairs(self.dragBitmap) do self.bitmap[k]=v end	
		if dOff~=nil then
			if math.abs(dOff-sOff)>math.abs(dBit-sBit) then
				for r=dOff><sOff,dOff<>sOff do
					self.bitmap[r]=(self.bitmap[r]&(0xFF7F>>(7-sBit)))|(fillValue<<sBit)
				end
			else
				for r=dBit><sBit,dBit<>sBit do
					self.bitmap[sOff]=(self.bitmap[sOff]&(0xFF7F>>(7-r)))|(fillValue<<r)
				end
			end
		end
		self:redraw()
	end
	self.dragBitmap={}
	for k,v in ipairs(self.bitmap) do self.dragBitmap[k]=v end	
	self.dragFill(x,y)
	return true
end
function UI.WeekSchedule:onMouseClick(x,y)
	if self:getFlags().disabled then return end
	self:onDragStart(x,y)
	self:onDragEnd(x,y)
	return true
end
function UI.WeekSchedule:onDragEnd(x,y)
	self.dragFill=nil
	local changed=false
	if self.dragBitmap then
		for k,v in ipairs(self.bitmap) do changed=changed or (self.dragBitmap[k]~=self.bitmap[k]) end	
		self.dragBitmap=nil
	end
	if changed then
		UI.dispatchEvent(self,"WidgetChange")				
	end
end
function UI.WeekSchedule:onDrag(x,y)
	if self.dragFill then 
		self.dragFill(x,y) 
		return true
	end
end

function UI.WeekSchedule:setFlags(changes)
	UI.Panel.setFlags(self,changes)
	if changes.disabled~=nil then
		self:setStateStyle(if changes.disabled then "weekschedule.styDisabled" else "weekschedule.styNormal")
	end
end

function UI.WeekSchedule:redraw()
	for _,s in ipairs(self.rset) do s:removeFromParent() end
	self.rset={}
	local function emit(d,hs,he)
		local p=UI.Panel.new()
		p:setStyle("weekschedule.styBars")
		p:setLayoutConstraints{ gridx=d, gridy=hs, gridheight=1+he-hs, insets=1, fill=1}
		self:addChild(p)
		table.insert(self.rset,p)
	end
	for d=1,7 do
		local rs=nil
		for h=1,24 do
			local on=(self.bitmap[h]&(1<<(d-1)))>0
			if self.inverted then on=not on end
			if on then
				if not rs then rs=h end
			else
				if rs then emit(d,rs,h-1) end	
				rs=nil			
			end
		end
		if rs then emit(d,rs,24) end
	end
	for _,s in ipairs(self.top) do self:addChild(s) end
end

--Utilities/Helpers
local function get_timezone_offset(ts)
    local utcdate   = os.date("!*t", ts)
    local localdate = os.date("*t", ts)
    localdate.isdst = false -- this is the trick
    return os.difftime(os.time(localdate), os.time(utcdate))
end

function UI.WeekSchedule:setBitmap(bm)
	local o=get_timezone_offset(os.time())//3600
	for k=0,23 do
		local b=bm:byte(k+1) or 0
		for d=0,6 do
			local bp=(d*24+k+168+o)%168
			local bpd=bp//24
			local bph=bp%24
			self.bitmap[bph+1]=self.bitmap[bph+1]|(((b>>d)&1)<<bpd)
		end
	end
	self:redraw()	
end

function UI.WeekSchedule:getBitmap()
	local o=get_timezone_offset(os.time())//3600
	local bm=""
	for k=0,23 do
		local b=0
		for d=0,6 do
			local bp=(d*24+k+168+o)%168
			local bpd=bp//24
			local bph=bp%24
			if ((self.bitmap[bph+1]>>bpd)&1)>0 then
				b=b|(1<<d)
			end
		end
		bm=bm..string.char(b)
	end
	return bm
end

UI.WeekSchedule.Definition= {
	name="WeekSchedule",
	class="UI.WeekSchedule",
	icon="ui/icons/label.png",
	constructorArgs={"Inverted"},
	properties={
		{ name="Inverted", type="boolean", setter=UI.WeekSchedule.setInverted },
	},
}