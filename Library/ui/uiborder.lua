--!NEEDS:uiinit.lua
UI.Border={}

if _PRINTER then print("uiborder.lua") end

UI.Border.Basic=Core.class(Path2D)
function UI.Border.Basic:init(params)
	self.params=params or {}
	self.params.thickness=self.params.thickness or 1
	self.params.insets=self.params.insets or 0
	self.params.radius=self.params.radius or 0
	self.params.bgColor=self.params.bgColor
	self.params.fgColor=self.params.fgColor or 0
	self.insets=self.params.thickness+self.params.insets+self.params.radius
	self:setFillColor(UI.Utils.colorVector(self.params.bgColor))
	self:setLineColor(UI.Utils.colorVector(self.params.fgColor))
end

function UI.Border.Basic:remove()
	self:getParent():removeEventListener(Event.LAYOUT_RESIZED,self.onResize,self)
	self:removeFromParent()
end

function UI.Border.Basic:apply(c)
	c:addChild(self)
	c:addEventListener(Event.LAYOUT_RESIZED,self.onResize,self)
	local w,h=c:getSize()
	self:setSize(w,h)
end

function UI.Border.Basic:onResize(e)
	self:setSize(e.width,e.height)
end

function UI.Border.Basic:setBgColor(c)
	self:setFillColor(UI.Utils.colorVector(c))
end
function UI.Border.Basic:setFgColor(c)
	self:setLineColor(UI.Utils.colorVector(c))
end

function UI.Border.Basic:setSize(w,h)
	local p=self.params
	if p.radius>0 then
		--Rounded
		local pb=p.thickness/2
		local pa=p.radius
		local pw=w-2*pb
		local ph=h-2*pb
		local r=1
		if p.reverse then r=0 end
		self:setPath("MLALALALAZ",
			pa,0,pw-pa,0,
			pa,pa,90,0,r,pw,pa, pw,ph-pa,
			pa,pa,90,0,r,pw-pa,ph, pa,ph,
			pa,pa,90,0,r,0,ph-pa, 0,pa,
			pa,pa,90,0,r,pa,0)
		self:setAnchorPosition(-pb,-pb)
	else
		--Square
	end
	self:setLineThickness(p.thickness)
end

UI.Border.NinePatch=Core.class()
function UI.Border.NinePatch:init(params)
	self.params=params or {}
	self.params.insets=self.params.insets or 0
	assert(self.params.corners,"NinePatch border needs corners params")
	assert(self.params.texture,"NinePatch border needs texture param")
	self.insets=self.params.insets
end

function UI.Border.NinePatch:remove(c)
	c:setNinePatch(0)
	c:setTexture(nil)
end

function UI.Border.NinePatch:apply(c)
	local tcorners={}
	for k,v in ipairs(self.params.corners) do
		tcorners[k]=c:resolveStyle(v)
	end
	c:setNinePatch(unpack(tcorners))
	local tx=c:resolveStyle(self.params.texture)
	c:setTexture(tx)
end

UI.Border.Tiled=Core.class()
function UI.Border.Tiled:init(params)
	self.params=params or {}
	self.params.insets=self.params.insets or 0
	assert(self.params.texture,"Tiled border needs texture param")
	self.insets=self.params.insets
end

function UI.Border.Tiled:remove(c)
	c:setNinePatch(0)
	c:setTexture(nil)
	c:setTextureScale(1,1)
end

function UI.Border.Tiled:apply(c)
	c:setNinePatch()
	local tx=c:resolveStyle(self.params.texture)
	c:setTexture(tx)
	local scale=if self.params.scale then c:resolveStyle(self.params.scale) else 1	
	c:setTextureScale(scale/tx:getWidth(),scale/tx:getHeight())
end
