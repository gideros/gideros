--!NEEDS:uiinit.lua

function UI.Color(c,g,b,a)
	local ct=type(c)
	if ct=="number" then
		if g and b then
			return vector(c,g,b,a or 1)
		else
			return vector((c&0xFF0000)/0xFF0000,(c&0xFF00)/0xFF00,(c&0xFF)/0xFF,g or 1)
		end
	elseif ct=="vector" then
		return vector(c.x,c.y,c.z,g or c.w)
	elseif ct=="userdata" then
		a=(c>>24)()
		c=(c&0xFFFFFF)()
		return vector((c&0xFF0000)/0xFF0000,(c&0xFF00)/0xFF00,(c&0xFF)/0xFF,(a&0xFF)/0xFF)
	elseif ct=="table" and c.color then
		a=c.alpha or 1
		c=c.color
		return vector((c&0xFF0000)/0xFF0000,(c&0xFF00)/0xFF00,(c&0xFF)/0xFF,a)		
	else
		assert(false,"unrecognized color type:"..ct)
	end
end

UI.Colors={
	transparent=vector(0,0,0,0),
	black=vector(0,0,0,1),
	white=vector(1,1,1,1),
}