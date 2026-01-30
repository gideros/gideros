--!NEEDS:uiinit.lua

local Color=ColorValue or vector
if not ColorValue then vector=function(a,b,c,d) return Color(a,b,c,d or 0) end end
function UI.Color(c,g,b,a)
	local ct=type(c)
	if ct=="number" then
		if g and b then
			return Color(c,g,b,a or 1)
		else
			return Color((c&0xFF0000)/0xFF0000,(c&0xFF00)/0xFF00,(c&0xFF)/0xFF,g or 1)
		end
	elseif ct=="color" then
		return Color(c.r,c.g,c.b,g or c.a)
	elseif ct=="vector" then
		return Color(c.x,c.y,c.z,g or c.w)
	elseif ct=="userdata" then
		a=(c>>24)()
		c=(c&0xFFFFFF)()
		return Color((c&0xFF0000)/0xFF0000,(c&0xFF00)/0xFF00,(c&0xFF)/0xFF,(a&0xFF)/0xFF)
	elseif ct=="table" and c.color then
		a=c.alpha or 1
		c=c.color
		return Color((c&0xFF0000)/0xFF0000,(c&0xFF00)/0xFF00,(c&0xFF)/0xFF,a)		
	else
		assert(false,"unrecognized color type:"..ct)
	end
end

UI.Colors={
	transparent=Color(0,0,0,0),
	black=Color(0,0,0,1),
	white=Color(1,1,1,1),
}