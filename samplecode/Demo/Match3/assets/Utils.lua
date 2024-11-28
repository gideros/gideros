--!NOEXEC
local atan2,random = math.atan2,math.random
function angle(x1,y1, x2,y2)
	return atan2(y2-y1,x2-x1)
end
function distance(x1,y1, x2,y2)
	return (x2-x1)^2 + (y2-y1)^2
end
function clamp(v,mn,mx) 
	return (v><mx)<>mn	
end
function choose(...)
	local t = {...}
	return rawget(t,random(#t))
end