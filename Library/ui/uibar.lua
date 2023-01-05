--!NEEDS:uipanel.lua
UI.Bar=Core.class(UI.Panel,function() return nil end)

local function setSizeProgress(self)
	if self and self.params and self.progress then
		local val 	= self.params.value or 0
		local max 	= self.params.maximum or 100
		if val and max then
			local p = ( val*100 ) / max
			local _,h 	= self.progress:getDimensions()
			local W	 	= self.width
			if p>=0 and p<=100 and h and W then 
				self.progress:setDimensions(( W*p ) / 100,h)
			end
		end
	end
end
function UI.Bar:setValue(value)
	if self and self.params then
		self.params.value = tonumber(value)
		setSizeProgress(self)
	end
end
function UI.Bar:setMaximum(maximum)
	if self and self.params then
		self.params.maximum = tonumber(maximum)
		setSizeProgress(self)
	end
end
function UI.Bar:setSize(w,h)
	if self then
		self.width=w
		self.height=h
		setSizeProgress(self)
	end
end
function UI.Bar:init(maximum)
	self.params = params or {}
	local lh=self:resolveStyle("1em")
	self:setLayoutParameters{ columnWeights={1}, rowWeights={0}, columnWidths={0}, rowHeights={lh} }
	local fgcolor = "bar.colForeground"
	local bgcolor = "bar.colBackground"
	self:setColor(bgcolor)
	self.progress=Pixel.new(UI.Utils.colorVector(fgcolor,self._style))
	self.progress:setLayoutConstraints{ gridy=0, gridx=0, fill=Sprite.LAYOUT_FILL_VERTICAL, anchor=Sprite.LAYOUT_ANCHOR_WEST, prefHeight=lh, minHeight=lh }
	self:addChild(self.progress)
	self:addEventListener(Event.LAYOUT_RESIZED,function (self,e)
		self:setSize(e.width,e.height)
	end,self)
	self:setMaximum(maximum)
end

UI.Bar.Definition= {
	name="Bar",
	icon="ui/icons/panel.png",
	class="UI.Bar",
	constructorArgs={ "Maximum" },
	properties={
		{ name="Maximum", type="number", setter=UI.Bar.setMaximum },
		{ name="Value", type="number", setter=UI.Bar.setValue },
	},
}
