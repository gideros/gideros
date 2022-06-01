--!NOEXEC
local PATH = ...

DRAG_POINT_RADIUS = 14
SHAPE_GLOBAL_ID = 1

require(PATH .. "/CollisionShape")
require(PATH .. "/Rect")
require(PATH .. "/Circle")
require(PATH .. "/Capsule")
require(PATH .. "/Poly")
require(PATH .. "/Ray")
require(PATH .. "/RayCone")

local SEGMENTS = 0

function drawVec(list, x, y, nx, ny, r, len, color, alpha, thickness)
	list:addLine(x, y, x + nx * len, y + ny * len, color, alpha, thickness)
	--list:addCircle(x, y, r, color, alpha)
end

function drawVecT(list, x, y, transform, len, color, alpha, thickness)
	local cosa, sina = transform:getCosSin()
	list:addLine(x, y, x + cosa * len, y + sina * len, color, alpha, thickness)
	list:addCircle(x, y, 4, color, alpha)
end

function drawCircle(list, x, y, r, isFilled, color, alpha)
	if (isFilled) then 
		list:addCircleFilled(x, y, r, color, alpha, SEGMENTS)
		list:addCircle(x, y, r, 0, 1, SEGMENTS)
	else
		list:addCircle(x, y, r, color, alpha, SEGMENTS)
	end
end

function drawRect(list, x1,y1,x2,y2, isFilled, color, alpha)
	if (isFilled) then 
		list:addRectFilled(x1,y1,x2,y2, color, alpha)
		list:addRect(x1,y1,x2,y2, 0, 1)
	else
		list:addRect(x1,y1,x2,y2, color, alpha)
	end
end

function drawCapsule(list, x, y, h, r, isFilled, color, alpha)
	local hh = h * 0.5
	
	list:pathArcTo(x, y - hh, r, 0, -3)
	list:pathArcTo(x, y + hh, r, 3, 0)
	
	if (isFilled) then 
		list:pathFillConvex(color, alpha)
		
		list:pathArcTo(x, y - hh, r, 0, -3)
		list:pathArcTo(x, y + hh, r, 3, 0)
		list:pathStroke(0, 1, true)
	else
		list:pathStroke(color, alpha, true)
	end	
end

function drawPoly(list, points, transform, isFilled, color, alpha)
	local x, y = transform:getPosition()
	
	for i, pt in ipairs(points) do 
		local px = x + pt.x
		local py = y + pt.y
		
		list:pathLineTo(px, py)
	end
	
	if (isFilled) then 
		list:pathFillConvex(color, alpha)
		list:pathStroke(color, alpha, true)
	else
		list:pathStroke(color, alpha, true)
	end
	
	list:addCircle(x, y, DRAG_POINT_RADIUS, 0x00ff00, 1)
	
	for i, pt in ipairs(points) do 
		local px = x + pt.x
		local py = y + pt.y
		
		list:addLine(x, y, px, py, 0, 1)
	end
	
	drawVecT(list, x, y, transform, 51, 0, 1, 3)
	drawVecT(list, x, y, transform, 50, 0xffffff, 1)
end