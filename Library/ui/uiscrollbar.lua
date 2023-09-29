--!NEEDS:uipanel.lua
--!NEEDS:uibehavior.lua
--!NEEDS:uistyle.lua

UI.Scrollbar=Core.class(UI.Panel,function() return nil end)
UI.Scrollbar.Template={
	class="UI.Panel",
	BaseStyle="scrollbar.styBar",
	children={
		{ class="UI.Panel", name="knob", 
		BaseStyle="scrollbar.styKnob", 
		layout={fill=Sprite.LAYOUT_FILL_BOTH}}
	}
}

function UI.Scrollbar:init(vertical,decorative)
	UI.BuilderSelf(UI.Scrollbar.Template,self)
	self.offset=0
	self.page=1
	self:setVertical(vertical)
	self:setDecorative(vertical)
end

function UI.Scrollbar:newClone() assert(false,"Cloning not supported") end

function UI.Scrollbar:setVertical(vertical)
	self.vertical=vertical
	if self.vertical then
		self.knob:setLayoutConstraints({gridy=2,gridx=0})
	else
		self.knob:setLayoutConstraints({gridy=0,gridx=2})
	end
	self:update()
end

function UI.Scrollbar:setDecorative(deco)
	local s=if deco then nil else self
	UI.Control.onDragStart[self]=s
	UI.Control.onDrag[self]=s
	UI.Control.onDragEnd[self]=s
	UI.Control.onMouseClick[self]=s
end

-- Sets the bar position: offset is the amount of document invisible at the top, page is the amount of document visible
function UI.Scrollbar:setScrollPosition(offset,page)
	self.offset=offset or self.offset
	self.page=page or self.page
	self:update()
end

function UI.Scrollbar:update()
	local pre=self.offset<>0
	local post=1-self.page-self.offset
	if self.vertical then
		self:setLayoutParameters({ rowWeights={0,pre,self.page,post,0},columnWeights={1}})
	else
		self:setLayoutParameters({ rowWeights={1},columnWeights={0,pre,self.page,post,0}})
	end
end

function UI.Scrollbar:onMouseClick(x,y)
	if long then return end
	local pos,po,ps
	if self.vertical then
		local s=self:getHeight()
		pos=y/s
		po=self.knob:getY()/s
		ps=self.knob:getHeight()/s
	else
		local s=self:getWidth()
		pos=x/s
		po=self.knob:getX()/s
		ps=self.knob:getWidth()/s
	end
	if pos<po then
		self:pageMove(-1)
	elseif pos>=(po+ps) then
		self:pageMove(1)
	end
	return true
end

function UI.Scrollbar:pageMove(dir)
	local pos=((self.offset+(self.page*dir))<>0)><(1-self.page)
	self:setScrollPosition(pos)
	UI.dispatchEvent(self,"WidgetChange",self.offset,self.page)
end

function UI.Scrollbar:onDragStart(x,y,ed,ea,change,long)
	if long then return end
	local pos,po,ps
	if self.vertical then
		local s=self:getHeight()
		pos=y/s
		po=self.knob:getY()/s
		ps=self.knob:getHeight()/s
	else
		local s=self:getWidth()
		pos=x/s
		po=self.knob:getX()/s
		ps=self.knob:getWidth()/s
	end
	if pos>=po and pos<(po+ps) then
		self._dragStart=self.offset-pos
	end
	return true
end

function UI.Scrollbar:onDrag(x,y)
  if self and self._dragStart then
	local pos
	if self.vertical then
		pos=y/self:getHeight()
	else
		pos=x/self:getWidth()
	end
	pos+=self._dragStart
	pos=(pos<>0)><(1-self.page)
	self:setScrollPosition(pos)
	UI.dispatchEvent(self,"WidgetChange",self.offset,self.page)
    return true
  end
end

function UI.Scrollbar:onDragEnd(x,y)
	self._dragStart=nil
end

UI.Scrollbar.Definition= {
  name="Scrollbar",
  icon="ui/icons/panel.png",
  class="UI.Scrollbar",
  constructorArgs={ "Vertical", "Decorative" },
  properties={
    { name="Border", type="border", setter=UI.Panel.setBorder, getter=UI.Panel.getBorder },
    { name="Vertical", type="boolean", setter=UI.Scrollbar.setVertical },
	{ name="Decorative", type="boolean", setter=UI.Scrollbar.setDecorative },
  },
}
