--!NEEDS:uiimagetext.lua
--!NEEDS:uibehavior.lua
--!NEEDS:uistyle.lua

UI.Button=Core.class(UI.Panel,function () return  end)
UI.Button.Template={
	class="UI.Panel",
	InternalStyle="button.styBack",
	layoutModel={ rowWeights={1},columnWeights={1}},
	model=UI.ImageText.Fallback,
	ImageTextInheritance="state",
}

function UI.Button:init(params)
	UI.BuilderSelf(UI.Button.Template,self)
	UI.Behavior.Button.new(self,params)
	UI.ImageText.includeSetters(self)
	self.ict:setStyle("button.styInside")
end

UI.Button.Definition= {
  name="Button",
  icon="ui/icons/panel.png",
  class="UI.Button",
  constructorArgs={ "BehaviorParams" },
  properties={
    { name="Border", type="border", setter=UI.Panel.setBorder, getter=UI.Panel.getBorder },
    { name="Image", type="image", setter=UI.ImageText.setImage },
    { name="Text", type="string", setter=UI.ImageText.setText },
    { name="ImageSize", type="table", setter=UI.ImageText.setImageSize },
    { name="Vertical", type="boolean", setter=UI.ImageText.setVertical },
	{ name="BehaviorParams", type="number" },
  },
}

UI.ToggleButton=Core.class(UI.Panel,function () return  end)
UI.ToggleButton.Template={
	class="UI.Panel",
	InternalStyle="button.styBack",
	layoutModel={ rowWeights={1},columnWeights={1}},
	behavior=UI.Behavior.ToggleButton,
	model=UI.ImageText.Fallback,
	ImageTextInheritance="state",
}

function UI.ToggleButton:init()
	UI.BuilderSelf(UI.ToggleButton.Template,self)
	UI.ImageText.includeSetters(self)
	self.ict:setStyle("button.styInside")
end

function UI.ToggleButton:setFlags(c)
	UI.Panel.setFlags(self,c)
	local fl=self:getFlags()
	if fl.disabled then
		self:setStateStyle("button.styDisabled")
	elseif fl.error then
		self:setStateStyle("button.styError")
	elseif fl.ticked then
		self:setStateStyle(if fl.focused then "button.stySelectedFocused" else "button.stySelected")
	else
		self:setStateStyle(if fl.focused then "button.styFocused" else {})
	end
end

UI.ToggleButton.Definition= {
  name="ToggleButton",
  icon="ui/icons/panel.png",
  class="UI.ToggleButton",
  constructorArgs={ },
  properties={
    { name="Image", type="image", setter=UI.ImageText.setImage },
    { name="Text", type="string", setter=UI.ImageText.setText },
    { name="ImageSize", type="table", setter=UI.ImageText.setImageSize },
    { name="Vertical", type="boolean", setter=UI.ImageText.setVertical },
  },
}

UI.PopupButton=Core.class(UI.ToggleButton)
UI.PopupButton.DOWN=1
UI.PopupButton.UP=2
UI.PopupButton.LEFT=4
UI.PopupButton.RIGHT=8
function UI.PopupButton:init()
	self.popupOffset={0,0}
	self.popupDirection=UI.PopupButton.DOWN|UI.PopupButton.UP
	self.autoClose=true
end

function UI.PopupButton:setContent(sprite) 
	if self.content then self.content.__GidUi_PopupLink=nil end
	self.content=sprite 
	if self.content then self.content.__GidUi_PopupLink=self end
end
function UI.PopupButton:setContentOffset(t) self.popupOffset=t or {0,0} end
function UI.PopupButton:setContentDirection(t) self.popupDirection=t or UI.PopupButton.DOWN|UI.PopupButton.UP end
function UI.PopupButton:setAutoClose(t) self.autoClose=t end

function UI.PopupButton:getMinimumContentSize()
	return 0,0
end

function UI.PopupButton:setFlags(c)
	UI.ToggleButton.setFlags(self,c)
	local ticked=self:getFlags().ticked
	if ticked then
		local px,py=self.popupOffset[1],self.popupOffset[2]
		local cw,ch=self:getDimensions()
		local mw,mh=self:getMinimumContentSize()
		local sites={}
		if (self.popupDirection&UI.PopupButton.DOWN)>0 then
			table.insert(sites,{x=px,y=py+ch,dx=1,dy=1,mvtx=true,w=mw, h=mh})
		end
		if (self.popupDirection&UI.PopupButton.UP)>0 then
			table.insert(sites,{x=px,y=py,dx=1,dy=-1,mvtx=true,w=mw, h=mh})
		end
		if (self.popupDirection&UI.PopupButton.LEFT)>0 then
			table.insert(sites,{x=px,y=py,dx=-1,dy=1,mvty=true,w=mw, h=mh})
		end
		if (self.popupDirection&UI.PopupButton.RIGHT)>0 then
			table.insert(sites,{x=px+cw,y=py,dx=1,dy=1,mvty=true,w=mw, h=mh})
		end
		if UI.Screen.popupAt(self,self.content,sites) then
			UI.Control.stopPropagation[self.content]=self.content
			if self.autoClose then UI.Focus.onFocus[self]=self end
		end
	else
		UI.Control.stopPropagation[self.content]=nil
		self.content:removeFromParent()
		UI.Focus.onFocus[self]=nil
	end
end

function UI.PopupButton:onFocus(w)
	-- Check if w is linked to this popup button somehow
	while w do
		if w==self then return end
		w=w.__GidUi_PopupLink or w:getParent()
	end
	self:setFlags({ticked = false})
end

UI.PopupButton.Definition= {
  name="PopupButton",
  icon="ui/icons/panel.png",
  class="UI.PopupButton",
  constructorArgs={ },
  properties={
    { name="Image", type="image", setter=UI.ImageText.setImage },
    { name="Text", type="string", setter=UI.ImageText.setText },
    { name="ImageSize", type="table", setter=UI.ImageText.setImageSize },
    { name="Vertical", type="boolean", setter=UI.ImageText.setVertical },
    { name="Content", type="sprite", setter=UI.PopupButton.setContent },
    { name="ContentOffset", type="table", setter=UI.PopupButton.setContentOffset },
    { name="ContentDirection", type="number", setter=UI.PopupButton.setContentDirection },
  },
}

