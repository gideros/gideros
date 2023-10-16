--!NEEDS:uipanel.lua
--!NEEDS:uibehavior.lua
--!NEEDS:uistyle.lua

UI.Splitpane=Core.class(UI.Panel,function () return UI.Colors.transparent end)
UI.Splitpane.Template={
	class="UI.Panel", 
	children={
		{ 	class="UI.Panel", name="knobBackground", ParentStyleInheritance="local",
			Color="splitpane.colKnobBackground",
			layout={fill=Sprite.LAYOUT_FILL_BOTH},
			children={},
		},
		{ 	class="UI.Panel", name="knob", ParentStyleInheritance="local",
			LocalStyle="splitpane.styKnobH",
			layout={fill=Sprite.LAYOUT_FILL_BOTH},
			children={
				{	class="UI.Panel", name="knobHandle",
					LocalStyle="splitpane.styKnobHandleH",
					layout={fill=Sprite.LAYOUT_FILL_BOTH},
					children={},
				}
			},
		},
		{	class="UI.Panel", name="knobCustom", ParentStyleInheritance="local",
			layoutModel={ columnWeights={1},rowWeights={1},columnWidths={0},rowHeights={0} },
			layout={fill=Sprite.LAYOUT_FILL_BOTH},
			children={},
		}
	}
}

function UI.Splitpane:newClone() assert(false,"Cloning not supported") end

function UI.Splitpane:init(vertical)
	UI.BuilderSelf(UI.Splitpane.Template,self)
	self.ratio=0.5
	self.minratio=0
	self.maxratio=1
	self:setVertical(vertical)
	UI.Control.onDrag[self.knob]=self.knob
	UI.Control.onDragStart[self.knob]=self.knob
    if UI.Control.HAS_CURSOR then 
		UI.Control.onMouseMove[self.knob]=self.knob
		self.knob.onMouseMove=function(s,x,y,h) return self:onKnobMove(x,y,h) end
	end
	self.knob.onMouseClick=function(s,x,y) return self:onKnobClick(x,y) end
	self.knob.onDragStart=function(s,x,y,ed,ea,change,long) return self:onDragStart(x,y,ed,ea,change,long) end
	self.knob.onDragEnd=function(s,x,y) return self:onDragEnd(x,y) end
	self.knob.onDrag=function(s,x,y) return self:onDrag(x,y) end
	self.knobCustom:setVisible(false)
end
local function setBorder(self)
	if self and self.knob and not self.knob.customized then
		if self.vertical then
			self.knob:setLocalStyle("splitpane.styKnobV")
			self.knobHandle:setLocalStyle("splitpane.styKnobHandleV")
		else
			self.knob:setLocalStyle("splitpane.styKnobH")
			self.knobHandle:setLocalStyle("splitpane.styKnobHandleH")
		end
	end
end
function UI.Splitpane:setVertical(vertical)
	self.vertical=vertical
	if self.vertical then
		self.knobBackground:setLayoutConstraints({
			gridy=2,gridx=0,gridheight=1,gridwidth=1,fill=Sprite.LAYOUT_FILL_BOTH,
		})
		self.knob:setLayoutConstraints({
			gridy=1,gridx=0,gridheight=3,gridwidth=1,fill=Sprite.LAYOUT_FILL_BOTH,
		})
		self.knobCustom:setLayoutConstraints({
			gridy=1,gridx=0,gridheight=3,gridwidth=1,fill=Sprite.LAYOUT_FILL_BOTH,
		})
		setBorder(self)
	else
		self.knobBackground:setLayoutConstraints({
			gridy=0,gridx=2,gridheight=1,gridwidth=1,fill=Sprite.LAYOUT_FILL_BOTH,
		})
		self.knob:setLayoutConstraints({
			gridy=0,gridx=1,gridheight=1,gridwidth=3,fill=Sprite.LAYOUT_FILL_BOTH,
		})
		self.knobCustom:setLayoutConstraints({
			gridy=0,gridx=1,gridheight=1,gridwidth=3,fill=Sprite.LAYOUT_FILL_BOTH,
		})
		setBorder(self)
	end
	self:setFirst(self.first)
	self:setSecond(self.second)
end

function UI.Splitpane:setFirst(s)
	if self.first and self.first~=s then self.first:removeFromParent() end
	self.first=s
	if s then
		self:addChildAt(s,1)
		local lc=self.first:getLayoutConstraints() or {}
		lc.gridx=0 lc.gridy=0 lc.weightx=0 lc.weighty=0
		if self.vertical then
			lc.gridheight=2 lc.gridwidth=0
			if self.knob.customized then lc.gridheight=1 end
		else 
			lc.gridwidth=2 lc.gridheight=0
			if self.knob.customized then lc.gridwidth=1 end
		end
		self.first:setLayoutConstraints(lc)
	end
	self:update()
end

function UI.Splitpane:setSecond(s)
	if self.second and self.second~=s then self.second:removeFromParent() end
	self.second=s
	if s then
		self:addChildAt(s,1)
		local lc=self.second:getLayoutConstraints() or {}
		lc.weightx=0 lc.weighty=0
		if self.vertical then
			lc.gridx=0 lc.gridy=3
			lc.gridheight=2 lc.gridwidth=0
			if self.knob.customized then lc.gridy=4 lc.gridheight=1 end --on force le non chevauchement
		else
			lc.gridx=3 lc.gridy=0
			lc.gridwidth=2 lc.gridheight=0
			if self.knob.customized then lc.gridx=4 lc.gridwidth=1 end --on force le non chevauchement
		end
		self.second:setLayoutConstraints(lc)
	end
	self:update()
end

function UI.Splitpane:fixupTabs(tabs)
	if not tabs then return nil end
	local t={}
	local dimw,dimh=self:getDimensions()
	local m=if self.vertical then dimh else dimw
	for i,r in ipairs(tabs) do
		if type(r)=="string" then
			r=self:resolveStyle(r)/m
			if i==#tabs then
				r=1-r 
			end
		end
		t[i]=r
	end
	for i,r in ipairs(t) do
		if i==#tabs then
			if t[i-1] and r<=t[i-1] then r=t[i-1] end
		else
			if t[i+1] and r>t[i+1] then r=t[i-1] or 0 end
		end
		t[i]=r
	end
	return t
end
	
function UI.Splitpane:setRatio(ratio,tabs)	
	if tabs and #tabs>=2 and self.first and self.second then
		if tabs[1] and ratio<=tabs[1] then
			ratio=0
		elseif tabs[#tabs] and ratio>=tabs[#tabs] then
			ratio=1
		end
	end
	ratio=(ratio<>0)><1
	self.ratio=ratio
	self:update()
end

function UI.Splitpane:getRatio()	
	return self.ratio
end

function UI.Splitpane:setAbsoluteSplit(a)
	local r
	local dx,dy=self:getParent():getDimensions()
	if self.vertical then
		r=a/dy
	else
		r=a/dx
	end
	r=(r<>self.minratio)><self.maxratio
	self:setRatio(r)
end

function UI.Splitpane:getAbsoluteSplit()
	local dx,dy=self:getParent():getDimensions()
	if self.vertical then
		return self.ratio*dy
	else
		return self.ratio*dx
	end
end

function UI.Splitpane:setTabs(tabs)
	self.tabs=tabs
	UI.Control.onMouseClick[self.knob]=if self.tabs and #self.tabs>2 then self.knob else nil
	self:setRatio(self.ratio,self:fixupTabs(tabs))
end

function UI.Splitpane:setOpenDirection(odr)
	self.openDirection=odr
end

function UI.Splitpane:update()
	local wth={0,0,0,0,0}
	local dual=self.first and self.second
	self.knob:setVisible(dual)
	if self.knob.customized then self.knobCustom:setVisible(dual) end
	local ratio=self.ratio
	if dual then
		self.first:setVisible(ratio>0)
		self.second:setVisible(ratio<1)
		local wts = self.tblKnobSizes or self:resolveStyle("splitpane.tblKnobSizes") --le chevauchement sera nul si KnobCustom
		if wts and #wts>=3 then wth={0,self:resolveStyle(wts[1]),self:resolveStyle(wts[2]),self:resolveStyle(wts[3]),0} end
	elseif self.first then
		ratio=1
		self.first:setVisible(true)
	elseif self.second then
		ratio=0
		self.second:setVisible(true)
	end
	if self.vertical then
		self:setLayoutParameters({ rowWeights={ratio,0,0,0,1-ratio},columnWeights={1},rowHeights=wth,columnWidths={0},equalizeCells=true})
	else
		self:setLayoutParameters({ rowWeights={1},columnWeights={ratio,0,0,0,1-ratio},rowHeights={0},columnWidths=wth,equalizeCells=true})
	end
end

function UI.Splitpane:updateStyle(...)
	UI.Panel.updateStyle(self,...)
	self:update()
end

function UI.Splitpane:setKnobCustom(s)
	if self and self.knob then
		if s and s.getLayoutConstraints then self.knob.customized=true
			local lc = s:getLayoutConstraints() or {}
			if not lc.prefWidth then lc.prefWidth=-1 end --uisplitpane knobCustom children layout default
			if not lc.prefHeight then lc.prefHeight=-1 end --uisplitpane knobCustom children layout default
			lc.fill=Sprite.LAYOUT_FILL_BOTH  --uisplitpane knobCustom children layout default
			local wts = self:resolveStyle("splitpane.tblKnobSizes")
			if wts and #wts>=3 then self.tblKnobSizes={0,0,wts[2],0,0} end --le chevauchement sera nul si KnobCustom	
			s:setLayoutConstraints(lc)
			self.knob:setColor(UI.Colors.transparent)
			self.knobHandle:setVisible(false)
			self.knobCustom:setVisible(true)
			self.knobCustom:addChild(s)
			self.knobCustom.child=s
			self:setVertical(self.vertical)
		elseif self.knobCustom.child then
			self.knobCustom.child:removeFromParent()
		end
	end
end

function UI.Splitpane:getKnob()
	return self.knob
end

function UI.Splitpane:isOnKnob(x,y)
	return self.knob.customized or self.knobHandle:hitTestPoint(x,y,false,self.knob) or self.knobBackground:hitTestPoint(x,y,false,self.knob)
end

function UI.Splitpane:onKnobMove(x,y)
	if self:isOnKnob(x,y) then
		UI.Control.setLocalCursor(if self.vertical then "splitV" else "splitH")
	end
end

function UI.Splitpane:onDragStart(x,y,ed,ea,change,long)
	if long then return end
	UI.Focus:request(self)
	self.dragging=self:isOnKnob(x,y)
	if self.dragging then 
		self.dragTabs=self:fixupTabs(self.tabs) 
	end
	return self.dragging
end

function UI.Splitpane:onDragEnd(x,y)
	self.dragging=nil
	self.dragTabs=nil
end

function UI.Splitpane:onKnobClick(x,y)
	if not self.tabs or #self.tabs<3 then return end
	if not self:isOnKnob(x,y) then return end
	UI.Focus:request(self)
	local tabs=self:fixupTabs(self.tabs) 
	if self.ratio<=(tabs[1] or 0) or self.ratio>=(tabs[#tabs] or 1) then
		local r = tabs[2]
		self:setRatio(r)
		UI.dispatchEvent(self,"WidgetChange",r)
	elseif self.openDirection then
		local r
		if self.openDirection<0 then
			r=0
		elseif self.openDirection>0 then
			r=1
		else
			r=if self.ratio>.5 then 1 else 0			
		end
		self:setRatio(r)
		UI.dispatchEvent(self,"WidgetChange",r)
	end
	return true
end

function UI.Splitpane:onDrag(x,y)
	if not self.dragging then return end
	local r
	local dx,dy=self:getParent():getDimensions()
	local px,py=self.knob:getPosition()
	local rng
	if self.vertical then
		r=(py+y)/dy
		rng=dy
	else
		r=(px+x)/dx
		rng=dx
	end
	local extraInfo={ rawRatio=r, range=rng }
	r=(r<>self.minratio)><self.maxratio
	self:setRatio(r,self.dragTabs)
	UI.dispatchEvent(self,"WidgetChange",r,extraInfo)
	return true
end

UI.Splitpane.Definition= {
  name="Splitpane",
  icon="ui/icons/panel.png",
  class="UI.Splitpane",
  constructorArgs={ "Vertical" },
  properties={
	{ name="First", type="sprite", setter=UI.Splitpane.setFirst},
	{ name="Second", type="sprite", setter=UI.Splitpane.setSecond},
    { name="Border", type="border", setter=UI.Panel.setBorder, getter=UI.Panel.getBorder },
    { name="Vertical", type="boolean", setter=UI.Splitpane.setVertical },
	{ name="Ratio", type="number", setter=UI.Splitpane.setRatio },
	{ name="Tabs", type="table" },
	{ name="OpenDirection", type="number" },
	{ name="KnobCustom", type="sprite", setter=UI.Splitpane.setKnobCustom },
  },
}
