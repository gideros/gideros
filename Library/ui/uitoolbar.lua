--!NEEDS:uitable.lua
UI.Toolbox=Core.class(UI.Table,function (header,direction) return nil,direction end)

function UI.Toolbox:init(header,direction)
	self:setLocalStyle("toolbox.styToolbox")
	self:setStyleInheritance("local")
	self:setBaseStyle("toolbox.styContainer")
	self:setColumns({
		{ 
		field=function(d)
			if self.builder then return self.builder(d) end			
		end, 
		name=function()
			return header			
		end
		}
	})
	if self.cheader then 
		self.cheader:setBaseStyle(if direction then "toolbox.styHeaderHorizontal" else "toolbox.styHeaderVertical")
	end
end

UI.Toolbox.Definition= {
  name="Toolbox",
  icon="ui/icons/panel.png",
  class="UI.Toolbox",
  constructorArgs={ "Header", "Direction" },
  properties={
    { name="Header", type="sprite" },
  },
}

