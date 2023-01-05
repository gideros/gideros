--!NEEDS:uipanel.lua
--!NEEDS:uibehavior.lua
UI.Checkbox=Core.class(UI.Panel,function() return nil end)

--[[
	d: data
	mapper: uielement <- (context,data,flags)
]]

local function genericFactory(d,p)
	return UI.Utils.makeWidget(p.data,p,nil,nil)
end

UI.Checkbox.Template={
	class="UI.Checkbox", 
	layoutModel={ columnWeights={0,1}, rowWeights={0}, },
	children={
		{ class="UI.Panel", name="cbIcon",layout={gridx=0, fill=Sprite.LAYOUT_FILL_NONE, width="checkbox.szIcon", height="checkbox.szIcon" }, Style="checkbox.styTickbox" },
		{ factory=genericFactory,layout={gridx=1, fill=1, }, name="cbLabel"},
	}}

-- Generic controls
function UI.Checkbox:init(d,mapper,mappercontext,behaviorClass,behaviorParam)
	self.d=d
	self.cbmapper=mapper
	self.cbmappercontext=mappercontext
	UI.BuilderSelf(UI.Checkbox.Template,self,nil,{ data=d, cbmapper=mapper, cbmappercontext=mappercontext });
	(behaviorClass or UI.Behavior.ToggleButton).new(self,behaviorParam)
	self:setTicked(d and d.checked)
end

function UI.Checkbox:update()
	local s="checkbox.styUnticked"
	if self._flags.disabled then
		s=if self._flags.ticked then "checkbox.styDisabledTicked" elseif self._flags.third then "checkbox.styDisabledThird" else "checkbox.styDisabled"
	elseif self._flags.ticked then
		s="checkbox.styTicked"	
	elseif self._flags.third then
		s="checkbox.styThird"	
	end
	self.cbIcon:setLocalStyle(s)
end

function UI.Checkbox:setFlags(changes,event)
	local ot=self._flags.ticked
	local ot3=self._flags.third
	if self.tristate and event then
		--Toggle doesn't know about third state, change flags here
		if ot3 then
			changes.third=false
			changes.ticked=false
		elseif ot then
			changes.third=true			
		end
	end
	UI.Panel.setFlags(self,changes,event)
	self.cbLabel:setFlags(changes)
	self:update()
	if ot~=self._flags.ticked or ot3~=self._flags.third then
		ot=self._flags.ticked
		if event then
			UI.dispatchEvent(self,"WidgetChange",self.d,ot,self._flags.third)
		end
	end
end

function UI.Checkbox:setTicked(c)
	self:setFlags({ticked=c, third=false})
end

function UI.Checkbox:setThird()
	self:setFlags({ticked=false, third=true})
end

function UI.Checkbox:setTristate(t)
	self.tristate=t
	if not t and self:getFlags().third then
		self:setFlags({third=false})
	end
end

function UI.Checkbox:isTicked()
	return self._flags.ticked
end

function UI.Checkbox:isThird()
	return self._flags.third
end

function UI.Checkbox:updateStyle(...)
	UI.Panel.updateStyle(self,...)
	self:update()
end

UI.Checkbox.Definition= {
	name="Checkbox",
	icon="ui/icons/checkbox.png",
	class="UI.Checkbox",
	constructorArgs={ "Label" },
	properties={
		{ name="Ticked", type="boolean", setter=UI.Checkbox.setTicked, getter=UI.Checkbox.isTicked },
		{ name="Third", type="boolean", setter=UI.Checkbox.setThird, getter=UI.Checkbox.isThird },
		{ name="Tristate", type="boolean"},
	},
}

UI.Radio=Core.class(UI.Checkbox,function(d,group,mapper,mappercontext) return d,mapper,mappercontext,UI.Behavior.RadioButton,group end)

function UI.Radio:init(d,group,mapper,mappercontext)
	self.cbIcon:setStyle("radio.styTickbox")
end

function UI.Radio:destroy()
end

function UI.Radio:update()
	local s="radio.styUnticked"
	if self._flags.disabled then
		s=if self._flags.ticked then "radio.styDisabledTicked" else "radio.styDisabled"
	elseif self._flags.ticked then
		s="radio.styTicked"
	end
	self.cbIcon:setLocalStyle(s)
end

function UI.Radio:_getIcon()
	local ic=self._style.radio.icUnsel
	if self._flags.ticked then
		ic=self._style.radio.icSel
	end
	return ic
end

UI.Radio.Definition= {
	name="Radio button",
	icon="ui/icons/radio-sel.png",
	class="UI.Radio",
	constructorArgs={ "Label", "Group" },
	properties={
		{ name="Ticked", type="boolean", setter=UI.Radio.setTicked, getter=UI.Radio.isTicked },
	},
}
