--!NEEDS:uipanel.lua
UI.Spinner=Core.class(UI.Panel,function(format) return end)

UI.Spinner.Template={
	class="UI.Spinner", 
	layoutModel={ columnWeights={0,1,0}, rowWeights={0}, columnWidths={"spinner.szIcon",1,"spinner.szIcon"}, rowHeights={"spinner.szIcon"} },
	children={
		{ class="UI.Image", name="btMinus", behavior=UI.Behavior.Button, behaviorParams={Repeat=0.2,RepeatSpeedup=0.95}, layout={gridx=0, fill=Sprite.LAYOUT_FILL_BOTH, },},
		{ class="UI.Label", Text="", name="label",layout={gridx=1, fill=0, insets=2}, TextLayout = { flags = FontBase.TLF_REF_LINETOP | FontBase.TLF_VCENTER | FontBase.TLF_CENTER },},
		{ class="UI.Image", name="btPlus", behavior=UI.Behavior.Button, behaviorParams={Repeat=0.2,RepeatSpeedup=0.95}, layout={gridx=2, fill=Sprite.LAYOUT_FILL_BOTH, },},
	}}
	
function UI.Spinner:init(range)
	UI.BuilderSelf(UI.Spinner.Template,self)
	self:setRange(range)
end

function UI.Spinner:updateStyle(...)
	UI.Panel.updateStyle(self,...)
end

function UI.Spinner:setFlags(changes)
	UI.Panel.setFlags(self,changes)
	if changes.disabled~=nil then
		self:setValue(self.value)
		self.label:setStateStyle(if changes.disabled then "spinner.styTextDisabled" else "spinner.styTextNormal")
		self:setStateStyle(if changes.disabled then "spinner.styDisabled" else "spinner.styNormal")
	end
end

function UI.Spinner:setRange(range)
	assert(type(range)=="table","Spinner range must be a table and not nil")
	if not range.Step then --Numeric
		self.btMinus:setImage(self:resolveStyle("spinner.icLstPrev"))
		self.btPlus:setImage(self:resolveStyle("spinner.icLstNext"))
		if range.Set then			
			self.range={Min=1,Max=#range.Set,Step=1,Set=range.Set}
			self.range.Looping=range.Looping
		else
			self.range={Min=1,Max=#range,Step=1,Set=range}
		end
	else
		self.range={}
		self.range.Min=range.Min or 0
		self.range.Max=range.Max or 1
		self.range.Step=range.Step
		self.range.Format=range.Format
		self.range.Check=range.Check
		self.btMinus:setImage(self:resolveStyle("spinner.icNumPrev"))
		self.btPlus:setImage(self:resolveStyle("spinner.icNumNext"))
		self.range.Looping=range.Looping
	end
	self:setValue(self.value or 0)
end

function UI.Spinner:setValue(val)
	local looped
	if self.range.Looping then
		local rs=1+self.range.Max-self.range.Min
		if val>self.range.Max then val=self.range.Min+(val-self.range.Min)%rs
			looped=1
		elseif val<self.range.Min then val=self.range.Min+(val-self.range.Min)%rs
			looped=-1
		end
	end
	if self.range.Step then
		val=((val+self.range.Step*.5)//self.range.Step)*self.range.Step
		if val>self.range.Max then val=self.range.Max
		elseif val<self.range.Min then val=self.range.Min
		end
	end
	if self.range.Check then
		val=self.range.Check(val)
	else
		val=(val<>self.range.Min)><self.range.Max
	end
	self.value=val
	local str
	if self.range.Set then 
		str=self.range.Set[self.value]
	elseif self.range.Format then
		if type(self.range.Format)=="function" then
			str=self.range.Format(self.value)
		else
			str=self.range.Format:format(self.value)
		end
	else
		str=self.value
	end
	self.label:setText(str)
	local fl=self:getFlags()
	local pdis,ndis=fl.disabled,fl.disabled
	if not self.range.Looping then
		if self.range.Min and self.value==self.range.Min then pdis=true end
		if self.range.Max and self.value==self.range.Max then ndis=true end
	elseif self.range.Min==self.range.Max then
		pdis=true
		ndis=true
	end
	self.btMinus:setStateStyle(if pdis then "spinner.styButtonDisabled" else "spinner.styButtonNormal")
	self.btPlus:setStateStyle(if ndis then "spinner.styButtonDisabled" else "spinner.styButtonNormal")
	return looped
end

function UI.Spinner:onWidgetAction(w,c)
	if self:getFlags().disabled then return end
	local ov=self.value
	local looped
	if w==self.btPlus then
		looped=self:setValue(self.value+(self.range.Step*(c or 1)))
	elseif w==self.btMinus then
		looped=self:setValue(self.value-(self.range.Step*(c or 1)))
	end
	if self.value~=ov then
		UI.dispatchEvent(self,"WidgetChange",self.value,looped)
	end
	return true
end

function UI.Spinner:getValue()
	return self.value
end

UI.Spinner.Definition= {
	name="Spinner",
	icon="ui/icons/panel.png",
	class="UI.Spinner",
	constructorArgs={ "Range" },
	properties={
		{ name="Value", type="number", setter=UI.Spinner.setValue, getter=UI.Spinner.getValue },
		{ name="Range", type="SpinnerRange", setter=UI.Spinner.setRange },
	},
}
