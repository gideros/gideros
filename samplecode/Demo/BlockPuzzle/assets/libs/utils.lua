local random,sqrt=math.random,math.sqrt
local function copyKeys(src, dst, ...)
	for _,v in pairs({...}) do 
		local value = src[v]
		if (value ~= nil and dst[v] == nil) then 
			dst[v] = value
		end
	end
end

-- copy missing values from source table to dst table
local function append(dst, source)
	for k,v in pairs(source) do 
		local _t = type(v)
		if (k == "owner") then 
			dst[k] = v
		elseif (_t == "table") then
			if (not dst[k]) then dst[k] = {} end
			append(dst[k], v)
		else
			if (dst[k] == nil) then 
				dst[k] = v
			end
		end
	end
end

local function clamp(v, min, max)
	return (v <> min) >< max
end
local function dist(x1,y1, x2,y2)
	return sqrt((x2-x1)^2+(y2-y1)^2)
end
local function dist2(x1,y1, x2,y2)
	return (x2-x1)^2+(y2-y1)^2
end
local function lerp(a, b, t)
    return a + t * (b - a)
end

local function round(number, places) --use -ve places to round to tens, hundreds etc
    local mult = 10^(places or 0)
    return ((number * mult + 0.5) // 1) / mult
end

local function map(v, minC, maxC, minD, maxD, clampValue)
	local newV = (v - minC) / (maxC - minC) * (maxD - minD) + minD
	return not clampValue and newV or (minD < maxD and clamp(newV, minD, maxD) or clamp(newV, maxD, minD))
end

local function frandom(min, max)
	if (max == nil) then 
		if (min == 0) then return min end
		return random()*min
	end
	return min + (random() * (max-min))
end

local function rgb2hex(r, g, b)
	return (r << 16) + (g << 8) + b
end

local function hex2rgb(hex)
	local r = hex >> 16
	local g = hex >> 8 & 0xff
	local b = hex & 0xff
	return r/255,g/255,b/255
end

local function randomColor()
	local r = random(255)
	local g = random(255)
	local b = random(255)
	return rgb2hex(r,g,b)
end

local function rect(w, h, c, a)
	return Pixel.new(c, a or 1, w, h)	
end

local function chooseTable(t)
	return t[random(#t)]
end
local function choose(...)
	return chooseTable({...})
end
return {
	clamp = clamp,
	append = append,
	lerp = lerp,
	round = round,
	map = map,
	frandom = frandom,
	rgb2hex = rgb2hex,
	hex2rgb = hex2rgb,
	randomColor = randomColor,
	rect = rect,
	copyKeys = copyKeys,
	choose = choose,
	chooseTable = chooseTable,
	dist = dist,
	dist2= dist2,
}