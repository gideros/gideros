local game = Game

Tile = Core.class(Pixel, function(t,w,h,c,a) return t,w,h end)

function Tile:init(tex, w, h, color, alpha)
	self.color = color or 0
	
	self:setNinePatch(16)
	self:setAnchorPoint(.5, .5)
	--self:setColor(self.color, alpha)
	self:setAlpha(alpha or 1)
end