--!NEEDS:uipanel.lua
--!NEEDS:uistyle.lua

UI.ImageText=Core.class(UI.Panel,function() return nil end)
UI.ImageText.Template={
	class="UI.Panel",
	layoutModel={ },
	children={
		{ class="UI.Image", name="icImage", visible=false,layout={fill=Sprite.LAYOUT_FILL_BOTH},},
		{ class="UI.Label", name="lbLabel", layout={gridx=1},},
	}
}
function UI.ImageText:init()
	UI.BuilderSelf(UI.ImageText.Template,self)
	self.ict=self
	self.iconsz=nil
	self:update()
end

function UI.ImageText:setImage(img)
	self.ict.icon=img
	self.ict:update()
end

function UI.ImageText:setText(txt)
	self.ict.text=txt
	self.ict:update()
end

function UI.ImageText:setImageSize(sz)
	self.ict.iconsz=sz
	self.ict:update()
end

function UI.ImageText:setVertical(v)
	self.ict.vertical=v
	self.ict:update()
end

function UI.ImageText:update()
	self.icImage:setVisible(self.icon)
	self.icImage:setImage(self.icon)
	self.lbLabel:setText(self.text)
	local imw,imh=0,0
	if self.icon then
		local lh=self:resolveStyle("1em")
		imw=(self.iconsz and (self.iconsz.w or self.iconsz[1])) or lh
		imh=(self.iconsz and (self.iconsz.h or self.iconsz[2])) or lh
	end
	local lm=self:getLayoutParameters()
	local lt=self.lbLabel:getLayoutConstraints()
	if self.vertical then
		lm.columnWeights={1}
		lm.rowWeights=if self.text then {0,1} else {1,0}
		lm.columnWidths={imw}
		lm.rowHeights={imh,0}
		lt.gridx=0
		lt.gridy=1
	else
		lm.columnWeights=if self.text then {0,1} else {1,0}
		lm.rowWeights={1}
		lm.columnWidths={imw,0}
		lm.rowHeights={imh}
		lt.gridx=1
		lt.gridy=0
	end
	if self.icon and self.text then
		lm.cellSpacingX=2
		lm.cellSpacingY=2
	end
	self:setLayoutParameters(lm)
	self.lbLabel:setLayoutConstraints(lt)
end

function UI.ImageText.includeSetters(self,ict)
	if ict or self._UI_ICT_Fallback then
		self.ict=ict or self._UI_ICT_Fallback
	end
	self.setImage=UI.ImageText.setImage
	self.setText=UI.ImageText.setText
	self.setImageSize=UI.ImageText.setImageSize
	self.setVertical=UI.ImageText.setVertical
end

function UI.ImageText.Fallback(d)
	if not d.children then 
		d=table.clone(d)
		d.children={{ class="UI.ImageText", name="_UI_ICT_Fallback",ParentStyleInheritance=d.ImageTextInheritance,layout={fill=Sprite.LAYOUT_FILL_BOTH}}}
		return d
	end
end

UI.ImageText.Definition= {
  name="ImageText",
  icon="ui/icons/panel.png",
  class="UI.ImageText",
  constructorArgs={ },
  properties={
    { name="Image", type="image", setter=UI.ImageText.setImage },
    { name="Text", type="string", setter=UI.ImageText.setText },
    { name="ImageSize", type="table", setter=UI.ImageText.setImageSize },
    { name="Vertical", type="boolean", setter=UI.ImageText.setVertical },
  },
}
