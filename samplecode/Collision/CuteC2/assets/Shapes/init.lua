--!NOEXEC
local PATH = ...

SHAPE_FILL = true
SHAPE_FILL_ALPHA = 0.5
SHAPE_DRAW_ALPHA = 1
POLY_LINES_VISIBLE = false
DRAG_POINT_VISIBLE = true
DRAG_POINT_RADIUS = 16
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
end

function drawCircle(list, x, y, r, color)
	if (SHAPE_FILL) then 
		list:addCircleFilled(x, y, r, color, SHAPE_DRAW_ALPHA, SEGMENTS)
		list:addCircle(x, y, r, 0, 1, SEGMENTS)
	else
		list:addCircle(x, y, r, color, SHAPE_DRAW_ALPHA, SEGMENTS)
	end
end

function drawRect(list, x1,y1,x2,y2, color)
	if (SHAPE_FILL) then 
		list:addRectFilled(x1,y1,x2,y2, color, SHAPE_DRAW_ALPHA)
		list:addRect(x1,y1,x2,y2, 0, 1)
	else
		list:addRect(x1,y1,x2,y2, color, 1)
	end
end

function drawCapsule(list, x, y, h, r, color)
	local hh = h * 0.5
	
	list:pathArcTo(x, y - hh, r, 0, -3)
	list:pathArcTo(x, y + hh, r, 3, 0)
	
	if (SHAPE_FILL) then 
		list:pathFillConvex(color, SHAPE_DRAW_ALPHA)
		
		list:pathArcTo(x, y - hh, r, 0, -3)
		list:pathArcTo(x, y + hh, r, 3, 0)
		list:pathStroke(0, 1, true)
	else
		list:pathStroke(color, SHAPE_DRAW_ALPHA, true)
	end	
end

function drawPoly(list, x, y, points, color)
	for i, pt in ipairs(points) do 
		list:pathLineTo(pt.x, pt.y)
	end
	
	if (SHAPE_FILL) then 
		list:pathFillConvex(color, SHAPE_DRAW_ALPHA)
		list:pathStroke(color, SHAPE_DRAW_ALPHA, true)
	else
		list:pathStroke(color, SHAPE_DRAW_ALPHA, true)
	end
	
	
	if (POLY_LINES_VISIBLE) then 
		list:addCircle(x, y, DRAG_POINT_RADIUS, 0x00ff00, 1)
		
		for i, pt in ipairs(points) do 
			list:addLine(x, y, pt.x, pt.y, 0, 1)
		end
	end
end