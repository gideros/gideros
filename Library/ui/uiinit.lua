--!LIBRARY:GiderosUI

if _PRINTER then print("uiinit.lua") end

UI=UI or {}
if not UI.Behavior then
	UI.Behavior=Core.class(Object)
	function UI.Behavior:init(widget)
	end
	function UI.Behavior:destroy()
	end
end
if not table.clone then
	local function clone(d,u)
		u=u or {}
		for k,p in pairs(d) do
			u[k]=p 
		end
		return u
	end
	table.clone=clone
end
function UI.instanceOf(i,cls)
	while i and i~=cls do
		i=getmetatable(i)
	end
	return i==cls
end

--[[
:: UI Display flags
* ticked: for checkbox or radio buttons: item is ticked
* selected: item is currently selected
* disabled: item is currently disabled
* focused: item has input focus
* expanded: item is expanded (Tree/Accordion)
Widgets that need to be visually updated on state changes should implement uiUpdate(data,flags)

:: UI Selection modes
* none: can't select
* click: single mechanism triggers an instant click
* single: single selection mode
* multiple: multiple selection mode
To use, the Sprite must implement uiSelect(x,y) function, which returns hit Sprite and data
]]