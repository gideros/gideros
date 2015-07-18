local function dot(ux, uy, vx, vy)
	return ux * vx + uy * vy
end

local function inside(p, cp1, cp2)
  return (cp2.x - cp1.x) * (p.y - cp1.y) > (cp2.y - cp1.y) * (p.x - cp1.x)
end

local function intersection(cp1, cp2, s, e)
  local dcx, dcy = cp1.x - cp2.x, cp1.y - cp2.y
  local dpx, dpy = s.x - e.x, s.y - e.y
  local n1 = cp1.x * cp2.y - cp1.y * cp2.x
  local n2 = s.x * e.y - s.y * e.x
  local n3 = 1 / (dcx * dpy - dcy * dpx)
  local x = (n1 * dpx - n2 * dcx) * n3
  local y = (n1 * dpy - n2 * dcy) * n3
  return {x = x, y = y}
end

function rayPolygonIntersection(vertices, ox, oy, dx, dy)
	local tmin = 0
	local tmax = 1
	
	local v1 = vertices[#vertices]
	for i=1,#vertices do
		v2 = vertices[i]
		local ex = v2.x - v1.x
		local ey = v2.y - v1.y
		local nx = ey
		local ny = -ex
		local n = dot(nx, ny, ox - v1.x, oy - v1.y)
		local d = dot(nx, ny, dx, dy)
		local t = -(n / d)
		if d < 0 then
			tmin = math.max(tmin, t)
		else
			tmax = math.min(tmax, t)
		end

		if tmin > tmax then
			return false
		end
		
		v1 = v2
	end

	return true
end


function splitPolygon(vertices, x1, y1, x2, y2)
	local vertices1 = {}
	local vertices2 = {}

	local cp1 = {x = x1, y = y1}
	local cp2 = {x = x2, y = y2}

	local inside1 = true
	local inside2 = true

	local s = vertices[#vertices]
	for i=1,#vertices do
		e = vertices[i]
				
		if inside(s, cp1, cp2) then
			if inside(e, cp1, cp2) then
				vertices1[#vertices1 + 1] = e
			else
				local p = intersection(cp1, cp2, s, e)
				vertices1[#vertices1 + 1] = p
				vertices2[#vertices2 + 1] = p
				vertices2[#vertices2 + 1] = e
			end
			inside2 = false
		else
			if inside(e, cp1, cp2) then
				local p = intersection(cp1, cp2, s, e)
				vertices1[#vertices1 + 1] = p
				vertices1[#vertices1 + 1] = e
				vertices2[#vertices2 + 1] = p				
			else
				vertices2[#vertices2 + 1] = e
			end
			inside1 = false
		end
		
		s = e
	end

	if inside1 == false and inside2 == false then
		return true, vertices1, vertices2
	end

	return false
end

function polygonArea(vertices)
	local area = 0
	local v1 = vertices[#vertices]
	for i=1,#vertices do
		local v2 = vertices[i]
		area = area + (v1.x * v2.y - v2.x * v1.y)
		v1 = v2
	end
	return area
end
