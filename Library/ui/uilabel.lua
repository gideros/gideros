--!NEEDS:uipanel.lua
local function initLayout(layout)
	local flags = FontBase.TLF_REF_LINETOP | FontBase.TLF_VCENTER
	layout = layout or {}
	layout.flags=layout.flags or flags
	if layout.flags&FontBase.TLF_BREAKWORDS==FontBase.TLF_BREAKWORDS then layout.breakChar = layout.breakChar or "…"
	else layout.flags = layout.flags | FontBase.TLF_NOWRAP end
	layout.flags=layout.flags|(FontBase.TLF_DISPOSELAYOUT or 0)
	return layout
end

UI.Label=Core.class(UI.Panel,function (text,layout) return nil end)
function UI.Label:init(text,layout,font,lightweight)
	lightweight=lightweight and math.fft
	self._label=TextField.new(self._style.font,text or "",initLayout(layout))
	self._label:setWorldAlign(not Oculus)
	self:addChild(self._label)
	if lightweight then
		self:setLayoutConstraints({ group=true })
	else
		self:setLayoutParameters({columnWeights={1},rowWeights={1},})
	end
	self._label:setLayoutConstraints({gridRelative=lightweight, fill=Sprite.LAYOUT_FILL_BOTH,  insetLeft="label.szInset", insetRight="label.szInset"})
	self._label:setTextColor(self.color or "label.color")
	self:setFont(font)
end

function UI.Label:setText(text)
	self._label:setText(text or "")
end
function UI.Label:getText()
	return self._label:getText()
end
function UI.Label:setTextLayout(tl)
	self._label:setLayout(tl)
end

function UI.Label:setColor(c)
	self.color=c
	self._label:setTextColor(self.color or "label.color")
end

function UI.Label:setFont(f)
	self._label:setFont(f or "label.font")
end

UI.Label.Definition= {
	name="Label",
	class="UI.Label",
	icon="ui/icons/label.png",
	constructorArgs={"Text","TextLayout","Font","Lightweight"},
	properties={
		{ name="Text", type="string", setter=UI.Label.setText, getter=UI.Label.getText, ibvalue="Label" },
		{ name="Color", type="color", setter=UI.Label.setColor, getter=UI.Label.getColor },
		{ name="Font", type="font", setter=UI.Label.setFont },
		{ name="Lightweight", type="boolean" },
	},
}

UI.StatefulLabel=Core.class(UI.Label,function (text,layout,font,lightweight) return text,layout,font,lightweight end)

function UI.StatefulLabel:setFlags(changes)
	UI.Label.setFlags(self,changes)
	if type(changes.disabled)=="boolean" then
		self:setStateStyle(if self._flags.disabled then "label.styDisabled" else "label.styNormal")
	end
end
