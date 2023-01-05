--!NEEDS:uipanel.lua
--!NEEDS:uilabel.lua
--!NEEDS:uicombobox.lua
--!NEEDS:uilocale.lua
UI.Calendar=Core.class(UI.Panel,function() return nil end)

UI.Calendar.Template={
	class="UI.Calendar", 
	layoutModel={ cellSpacingX="calendar.szMargin", columnWeights={1,0} },
	LocalStyle="calendar.styBase",
	children={
		{ class="UI.Spinner", name="spMonth", layout={gridx=0, fill=Sprite.LAYOUT_FILL_BOTH },
			LocalStyle="calendar.stySpinnersLocal",
			Style="calendar.stySpinners",
			Range={ Looping=true, Set={ UI._LO("MONTH1"), UI._LO("MONTH2"), UI._LO("MONTH3"), UI._LO("MONTH4"), 
			UI._LO("MONTH5"), UI._LO("MONTH6"), UI._LO("MONTH7"), UI._LO("MONTH8"), 
			UI._LO("MONTH9"), UI._LO("MONTH10"), UI._LO("MONTH11"), UI._LO("MONTH12")}}, },
		{ class="UI.Spinner", name="spYear", layout={gridx=1, fill=Sprite.LAYOUT_FILL_BOTH, width="5em" },
			LocalStyle="calendar.stySpinnersLocal",
			Style="calendar.stySpinners",
			Range={Min=2000,Max=2060, Step=1}},
		{ class="UI.Panel",
			name="pnDays",
			layoutModel={ },
			layout={gridwidth=2, fill=Sprite.LAYOUT_FILL_BOTH, gridy=1}, 
			--TextLayout = { flags = FontBase.TLF_REF_LINETOP | FontBase.TLF_VCENTER | FontBase.TLF_CENTER },
			},
	}}
	

function UI.Calendar:init()
	UI.BuilderSelf(UI.Calendar.Template,self)
	self.days={}
	self.daysMap={}
	local cs="calendar.szDay"
	self.pnDays:setLayoutParameters({
		columnWidths={cs,cs,cs,cs,cs,cs,cs},
		cellSpacingX="calendar.szCellSpacing",
		rowHeights={0,cs,cs,cs,cs,cs,cs},
	})
	for i=1,7 do
		local lt=UI.Label.new(UI._LO("DAY"..i))
		lt:setLocalStyle("calendar.styDayHeader")
		lt:setLayoutConstraints{ gridx=i-1, gridy=0}
		self.pnDays:addChild(lt)
	end
	for i=1,42 do
		local lt=UI.Label.new("",{ flags = FontBase.TLF_REF_LINETOP | FontBase.TLF_VCENTER | FontBase.TLF_CENTER })
		
		lt:setLayoutConstraints{ gridx=(i-1)%7, gridy=1+(i-1)//7, fill=Sprite.LAYOUT_FILL_BOTH}
		self.pnDays:addChild(lt)
		self.days[i]=lt
		self.daysMap[lt]=i
	end
	self:setDate(os.time())
	--Control
	UI.Control.onMouseClick[self]=self
end

function UI.Calendar:newClone() assert(false,"Cloning not supported") end

function UI.Calendar:onWidgetChange(w,v,loop)
	if self:getFlags().disabled then return end
	if w==self.spMonth then
		self.month=v
		if loop then			
			self.spYear:setValue(self.year+loop)
			self.year=self.spYear:getValue()
		end
		self:updateMonth()
		return true
	elseif w==self.spYear then
		self.year=v
		self:updateMonth()
		return true
	end
end

function UI.Calendar:onMouseClick(x,y)
	UI.Focus:request(self)
    x,y=self:localToGlobal(x,y)
    local eb=self.pnDays:getChildrenAtPoint(x,y,true,true)
	for _,spr in ipairs(eb) do
		local dayn=self.daysMap[spr]
		if dayn then			
			local stime=os.time({day=1,month=self.month,year=self.year,hour=0})
			local fday=os.date("*t",stime)
			fday.wday=if fday.wday==1 then 7 else fday.wday-1
			stime=os.time({day=1+dayn-fday.wday,month=self.month,year=self.year,hour=0})
			self:setDate(stime)
			UI.dispatchEvent(self,"WidgetChange",self.date)
		end
	end
	return true
end

--Utilities/Helpers
local function get_timezone_offset(ts)
    local utcdate   = os.date("!*t", ts)
    local localdate = os.date("*t", ts)
    localdate.isdst = false -- this is the trick
    return os.difftime(os.time(localdate), os.time(utcdate))
end

function UI.Calendar:updateMonth()
	local stime=os.time({day=1,month=self.month,year=self.year,hour=0})
	local tz=get_timezone_offset(stime)
	local sntime=os.time({day=1,month=self.month+1,year=self.year,hour=0})
	local tzn=get_timezone_offset(sntime)
	local fday=os.date("*t",stime)
	local lmdays=os.date("*t",stime-tz-3600).day
	local cmdays=os.date("*t",sntime-tzn-3600).day
	local cday=os.date("*t",self.date)
	if cday.month==self.month and cday.year==self.year then cday=cday.day else cday=nil end
	--Assume first day of week is monday, not sunday
	fday.wday=if fday.wday==1 then 7 else fday.wday-1
	if fday.wday>1 then
		for d=1,fday.wday-1 do
			local di=self.days[d]
			di:setStyle("calendar.styDaysOther")
			di:setText(d+1+lmdays-fday.wday)
		end
	end
	for d=1,cmdays do
		local di=self.days[d-1+fday.wday]
		di:setStyle(if cday==d then "calendar.styDaySelected" else "calendar.styDays")
		di:setText(d)
	end
	for d=cmdays+fday.wday,42 do
		local di=self.days[d]
		di:setStyle("calendar.styDaysOther")
		di:setText(1+d-cmdays-fday.wday)
	end
end
	
function UI.Calendar:setDate(bm)
	local o=get_timezone_offset(os.time())//3600
	self.date=bm
	local dm=os.date("*t",bm)
	self.month=dm.month
	self.year=dm.year
	self.spMonth:setValue(self.month)
	self.spYear:setValue(self.year)
	self:updateMonth()	
end

function UI.Calendar:getDate()
	local o=get_timezone_offset(os.time())//3600
	return self.date
end

UI.Calendar.Definition= {
	name="Calendar",
	class="UI.Calendar",
	icon="ui/icons/label.png",
	constructorArgs={},
	properties={
		{ name="Date", type="date" },
	},
}


--- Date picker
UI.DatePicker=Core.class(UI.PopupButton,function (allowNil) return nil end)
UI.DatePicker.Template={
	class="UI.PopupButton",	
	BaseStyle="combobox.styBase",
	children={
		{ class="UI.ButtonTextFieldCombo", 
		layout={fill=Sprite.LAYOUT_FILL_BOTH, width="datepicker.szWidth",}, 
		TextLayout={flags=FontBase.TLF_SINGLELINE|FontBase.TLF_REF_LINETOP | FontBase.TLF_VCENTER}, 
		Editable=false,
		name="editor",
		Style={ ["buttontextfieldcombo.icButton"]="datepicker.icPicker" },
		}
	},
	--ContentOffset={-10,20}, --Placement margin account for border and spacing
	Content={
		class="UI.Panel",
		name="popup",
		layoutModel={ },
		children={{
			class="UI.Calendar",
			name="clCalendar",
			layout={ fill=Sprite.LAYOUT_FILL_BOTH}
		}}
	},
}

--- Combobox object

function UI.DatePicker:init(allowNil)
	UI.BuilderSelf(UI.DatePicker.Template,self)
	function self.popup.onWidgetChange(s,w,date)
		if w==self.clCalendar then
			self:setDate(date)
			return true
		end
	end
	self.editor:setLocalStyle("datepicker.styNormal")
	self.editor:setTipText(UI._LO("UI.DatePicker.Format"))
	self.allowNil=allowNil
	if not allowNil then
		self:setDate(os.time())
	end
end

function UI.DatePicker:setDate(date) 
	if not date and not self.allowNil then date=os.time() end
	if date then
		self.clCalendar:setDate(date)
		self.editor:setText(os.date(UI._LO("UI.DatePicker.IFormat"),date))
	else
		self.editor:setText("")
	end
	self:setFlags({ticked=false})
	self.editor.button:setFlags({ticked=false})
	UI.Focus:relinquish(self)
end

function UI.DatePicker:getDate()
	local t=self.editor:getText()
	return if #t>0 and self:checkDate(t) then self.clCalendar:getDate() else nil
end

function UI.DatePicker:onFocusChange(w,v)
	if w==self.editor then 
		if self.editor:getFlags().focused then
			self:setFlags({ticked=false})
			self.editor.button:setFlags({ticked=false})
		else
			local t=self.editor:getText()
			local et=self:checkDate(t)
			if et then
				if #t==0 then et=nil end
				self:setDate(et) 
				UI.dispatchEvent(self,"WidgetChange",et)
			end
		end
	end
end 


function UI.DatePicker:setFlags(c)
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
		self.preEdit=self:getDate()
	--self.editor:focus() 
	--self.editor.button:setFlags({ticked=false})
	else
		self.editor.button:setFlags({ticked=false})
		local value=self:getDate()
		if value~=self.preEdit then
			UI.dispatchEvent(self,"WidgetChange",value)
		end
		self.preEdit=nil
	end
end

function UI.DatePicker:onWidgetAction(w)
	if w==self.editor.button then
		local ticked = w:getFlags().ticked
		self:setFlags({ticked=ticked})
	end
end

function UI.DatePicker:checkDate(d)
	if self.allowNil and #d==0 then return 0 end
	local f1,f2,f3=d:match("(%d+)/(%d+)/(%d+)")
	local n1,n2,n3=UI._LO("UI.DatePicker.IFormat"):match("(%%%a)/(%%%a)/(%%%a)")
	local t={}
	local tmap={
		["%m"]="month",
		["%d"]="day",
		["%Y"]="year",
	}
	t[tmap[n1]]=tonumber(f1) or 1
	t[tmap[n2]]=tonumber(f2) or 1
	t[tmap[n3]]=tonumber(f3) or 1
	local etime=os.time(t)
	local ot=os.date("*t",etime)
	local valid = false
	if ot and t and ot.day and ot.month and ot.year and t.day and t.month and t.year then
		valid = (ot.day==t.day and ot.month==t.month and ot.year==t.year)
	end
	if not valid then return end
	return etime
end
	
function UI.DatePicker:onTextChange(w,t)
	if w==self.editor then
		self.editor:setLocalStyle(if not self:checkDate(t) then "datepicker.styError" else "datepicker.styNormal")
	end
end

function UI.DatePicker:onTextValid(w,t)
	if w==self.editor then
		local et=self:checkDate(t)
		if et then
			if #t==0 then et=nil end
			self:setDate(et) 
			UI.dispatchEvent(self,"WidgetChange",et)
		end
	end
end


UI.DatePicker.Definition= {
  name="DatePicker",
  icon="ui/icons/panel.png",
  class="UI.DatePicker",
  constructorArgs={ "AllowNil"},
  properties={
	  { name="AllowNil", type="boolean" },
  },
}