--!NEEDS:uipanel.lua
local _hasFontStyle=true
local function initLayout(layout)
	local flags = FontBase.TLF_REF_LINETOP | FontBase.TLF_VCENTER
	layout = layout or {}
	layout.flags=layout.flags or flags
	if layout and layout.flags then
		if layout.flags&FontBase.TLF_BREAKWORDS==FontBase.TLF_BREAKWORDS then layout.breakChar = layout.breakChar or "…"
		else layout.flags = layout.flags | FontBase.TLF_NOWRAP end
	end
	return layout
end

UI.Label=Core.class(UI.Panel,function (text,layout) return nil end)
function UI.Label:init(text,layout,font)
	self._label=TextField.new(self._style.font,text or "",initLayout(layout))
	if self._label.setWorldAlign then self._label:setWorldAlign(true) end
	self:addChild(self._label)
	self:setLayoutParameters({columnWeights={1},rowWeights={1}, insetLeft="label.szInset", insetRight="label.szInset"})
	self._label:setLayoutConstraints({fill=Sprite.LAYOUT_FILL_BOTH})
	if not _hasFontStyle then self._label:setStyle(self._style) end
	self._label:setTextColor(self.color or "label.color")
	self:setFont(font)
end

if not _hasFontStyle then 
	function UI.Label:newClone()
		UI.Panel.newClone(self)
		self._label:setStyle(self._style)
	end
end
function UI.Label:setText(text)
	self._label:setText(text or "")
end
function UI.Label:getText()
	return self._label:getText()
end

function UI.Label:setColor(c)
	self.color=c
	self._label:setTextColor(self.color or "label.color")
end

function UI.Label:setFont(f)
	if _hasFontStyle then
		self._label:setFont(f or "label.font")
	else
		self.font=f
		self._label:setFont(self:resolveStyle(self.font or "label.font"))
	end
end

if not _hasFontStyle then
	function UI.Label:updateStyle(...)
		UI.Panel.updateStyle(self,...)
		if not self._label then return end --Should never happen, but DOES happen. Remove this when fixed
		local font=self:resolveStyle(self.font or "label.font")
		if font then
			self._label:setFont(font)
		end
	end
end

UI.Label.Definition= {
	name="Label",
	class="UI.Label",
	icon="ui/icons/label.png",
	constructorArgs={"Text","TextLayout","Font"},
	properties={
		{ name="Text", type="string", setter=UI.Label.setText, getter=UI.Label.getText, ibvalue="Label" },
		{ name="Color", type="color", setter=UI.Label.setColor, getter=UI.Label.getColor },
		{ name="Font", type="font", setter=UI.Label.setFont },
	},
}
