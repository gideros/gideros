--[[
Author of this easing functions for Lua is Josh Tynjala
https://github.com/joshtynjala/gtween.lua
Licensed under the MIT license.

Easing functions adapted from Robert Penner's AS3 tweening equations.

*09.12.2018 Modified by @Apollo14: replaced vanilla math functions with faster Gideros ^ operators
*21.12.2019 Modified to use local sin, cos, and pi ]]

local sin,cos=math.sin,math.cos
local pii=3.14159265358979324
local backS = 1.70158
easing = {};
easing.inBack = function(ratio)
	return ratio*ratio*((backS+1)*ratio-backS)
end
easing.outBack = function(ratio)
	ratio-=1
	return ratio*ratio*((backS+1)*ratio+backS)+1
end
easing.inOutBack = function(ratio)
	ratio*=2
	if ratio < 1 then
		return 0.5*(ratio*ratio*((backS*1.525+1)*ratio-backS*1.525))
	else 
		ratio-=2
		return 0.5*(ratio*ratio*((backS*1.525+1)*ratio+backS*1.525)+2)
	end
end
easing.inBounce = function(ratio)
	return 1-easing.outBounce(1-ratio,0,0,0)
end
easing.outBounce = function(ratio)
	if ratio < 1/2.75 then
		return 7.5625*ratio*ratio
	elseif ratio < 2/2.75 then
		ratio-=1.5/2.75
		return 7.5625*ratio*ratio+0.75
	elseif ratio < 2.5/2.75 then
		ratio-=2.25/2.75
		return 7.5625*ratio*ratio+0.9375
	else
		ratio-=2.625/2.75
		return 7.5625*ratio*ratio+0.984375
	end
end
easing.inOutBounce = function(ratio)
	ratio*=2
	if ratio < 1 then 
		return 0.5*easing.inBounce(ratio,0,0,0)
	else
		return 0.5*easing.outBounce(ratio-1,0,0,0)+0.5
	end
end
easing.inCircular = function(ratio)
	return -((1-ratio*ratio)^0.5-1)
end
easing.outCircular = function(ratio)
	return (1-(ratio-1)*(ratio-1))^0.5
end
easing.inOutCircular = function(ratio)
	ratio*=2
	if ratio < 1 then
		return -0.5*((1-ratio*ratio)^0.5-1)
	else
		ratio-=2
		return 0.5*((1-ratio*ratio)^0.5+1)
	end
end
easing.inCubic = function(ratio)
	return ratio*ratio*ratio
end
easing.outCubic = function(ratio)
	ratio-=1
	return ratio*ratio*ratio+1
end
easing.inOutCubic = function(ratio)
	if ratio < 0.5 then
		return 4*ratio*ratio*ratio
	else
		ratio-=1
		return 4*ratio*ratio*ratio+1
	end
end
local elasticA = 1;
local elasticP = 0.3;
local elasticS = elasticP/4;
easing.inElastic = function(ratio)
	if ratio == 0 or ratio == 1 then
		return ratio
	end
	ratio-=1
	return -(elasticA * 2^(10 * ratio) * sin((ratio - elasticS) * (2 * pii) / elasticP));
end
easing.outElastic = function(ratio)
	if ratio == 0 or ratio == 1 then
		return ratio
	end
	return elasticA * 2^(-10 * ratio) *  sin((ratio - elasticS) * (2 * pii) / elasticP) + 1;
end
easing.inOutElastic = function(ratio)
	if ratio == 0 or ratio == 1 then
		return ratio
	end
	ratio = ratio*2-1
	if ratio < 0 then
		return -0.5 * (elasticA * 2^(10 * ratio) * sin((ratio - elasticS*1.5) * (2 * pii) /(elasticP*1.5)));
	end
	return 0.5 * elasticA * 2^(-10 * ratio) * sin((ratio - elasticS*1.5) * (2 * pii) / (elasticP*1.5)) + 1;
end
easing.inExponential = function(ratio)
	if ratio == 0 then
		return 0
	end
	return 2^(10 * (ratio - 1))
end
easing.outExponential = function(ratio)
	if ratio == 1 then
		return 1
	end
	return 1-2^(-10 * ratio)
end
easing.inOutExponential = function(ratio)
	if ratio == 0 or ratio == 1 then 
		return ratio
	end
	ratio = ratio*2-1
	if 0 > ratio then
		return 0.5*2^(10 * ratio)
	end
	return 1-0.5*2^(-10 * ratio)
end
easing.linear = function(ratio)
	return ratio
end
easing.inQuadratic = function(ratio)
	return ratio*ratio
end
easing.outQuadratic = function(ratio)
	return -ratio*(ratio-2)
end
easing.inOutQuadratic = function(ratio)
	if ratio < 0.5 then
		return 2*ratio*ratio
	end
	return -2*ratio*(ratio-2)-1
end
easing.inQuartic = function(ratio)
	return ratio*ratio*ratio*ratio
end
easing.outQuartic = function(ratio)
	ratio-=1
	return 1-ratio*ratio*ratio*ratio
end
easing.inOutQuartic = function(ratio)
	if ratio < 0.5 then
		return 8*ratio*ratio*ratio*ratio
	end
	ratio-=1
	return -8*ratio*ratio*ratio*ratio+1
end
easing.inQuintic = function(ratio)
	return ratio*ratio*ratio*ratio*ratio
end
easing.outQuintic = function(ratio)
	ratio-=1
	return 1+ratio*ratio*ratio*ratio*ratio
end
easing.inOutQuintic = function(ratio)
	if ratio < 0.5 then
		return 16*ratio*ratio*ratio*ratio*ratio
	end
	ratio-=1
	return 16*ratio*ratio*ratio*ratio*ratio+1
end
easing.inSine = function(ratio)
	return 1-cos(ratio * (pii / 2))
end
easing.outSine = function(ratio)
	return sin(ratio * (pii / 2))
end
easing.inOutSine = function(ratio)
	return -0.5*(cos(ratio*pii)-1)
end
