--!NEEDS:ui/uipanel.lua

AUI=AUI or {}
AUI.TreeItem=Core.class(UI.Panel,function (item) return end)

AUI.TreeItem.Template={
	class="AUI.TreeItem", 
	layoutModel={ insets="aui_asitem.szSpacing", cellSpacingX="aui_asitem.szSpacing", columnWeights={0,1}, rowWeights={1}, },
	children={
		{ class="UI.Image", name="img", layout={gridx=0, fill=Sprite.LAYOUT_FILL_BOTH, width="1em", height="1em" } },
		{ class="UI.Label", name="lbl", layout={gridx=1, fill=Sprite.LAYOUT_FILL_BOTH }, 
			--Font="aui_asitem.font",
			TextLayout = { flags = FontBase.TLF_REF_LINETOP | FontBase.TLF_VCENTER | FontBase.TLF_LEFT | FontBase.TLF_SINGLELINE },
		},
	}
}	

function AUI.TreeItem:init(item)
	UI.BuilderSelf(AUI.TreeItem.Template,self)
	if item.assetLib and item.assetName then
		self.img:setImage(item.assetLib:getThumbnail(item.assetName))		
	end
	local name=item.name
	if item.assetName then
		if name then 
			name=name.." ["..item.assetName.."]"
		else
			name="["..item.assetName.."]"
		end
	end
	self.lbl:setText(name or "GROUP")
end

function AUI.TreeItem:setFlags(changes)
	UI.Panel.setFlags(self,changes)
	if changes.selected~=nil then
		self:setStateStyle(if changes.selected then "aui_tree.stySelected" else nil)
	end
end
