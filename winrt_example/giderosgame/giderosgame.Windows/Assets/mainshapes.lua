--[[

Shapes example

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

function createShape1()
	local shape = Shape.new()

	shape:setLineStyle(4, 0x000000)

	shape:beginPath()
	shape:moveTo(160, 50)
	shape:lineTo(74, 284)
	shape:lineTo(272, 130)
	shape:lineTo(48, 130)
	shape:lineTo(246, 284)
	shape:closePath()
	shape:endPath()

	local t1 = TextField.new(nil, "setLineStyle(4, 0x000000)")
	t1:setX(60)
	t1:setY(320)
	shape:addChild(t1)

	return shape
end



function createShape2()
	local shape = Shape.new()

	shape:setFillStyle(Shape.SOLID, 0xff0000, 0.5)

	shape:beginPath()
	shape:moveTo(160, 50)
	shape:lineTo(74, 284)
	shape:lineTo(272, 130)
	shape:lineTo(48, 130)
	shape:lineTo(246, 284)
	shape:closePath()
	shape:endPath()

	local t1 = TextField.new(nil, "setFillStyle(Shape.SOLID, 0xff0000, 0.5)")
	t1:setX(60)
	t1:setY(320)
	shape:addChild(t1)

	return shape
end


function createShape3()
	local shape = Shape.new()

	shape:setFillStyle(Shape.SOLID, 0xff0000, 0.5)

	shape:beginPath(Shape.NON_ZERO)
	shape:moveTo(160, 50)
	shape:lineTo(74, 284)
	shape:lineTo(272, 130)
	shape:lineTo(48, 130)
	shape:lineTo(246, 284)
	shape:closePath()
	shape:endPath()

	local t1 = TextField.new(nil, "setFillStyle(Shape.SOLID, 0xff0000, 0.5)")
	t1:setX(60)
	t1:setY(320)
	shape:addChild(t1)

	local t2 = TextField.new(nil, "winding = Shape.NON_ZERO")
	t2:setX(60)
	t2:setY(332)
	shape:addChild(t2)

	return shape
end

function createShape4()
	local shape = Shape.new()

	shape:setLineStyle(4, 0x000000)
	shape:setFillStyle(Shape.SOLID, 0xff0000, 0.5)

	shape:beginPath()
	shape:moveTo(160, 50)
	shape:lineTo(74, 284)
	shape:lineTo(272, 130)
	shape:lineTo(48, 130)
	shape:lineTo(246, 284)
	shape:closePath()
	shape:endPath()

	local t1 = TextField.new(nil, "setLineStyle(4, 0x000000)")
	t1:setX(60)
	t1:setY(320)
	shape:addChild(t1)

	local t2 = TextField.new(nil, "setFillStyle(Shape.SOLID, 0xff0000, 0.5)")
	t2:setX(60)
	t2:setY(332)
	shape:addChild(t2)

	return shape
end


function createShape5()
	local texture = Texture.new("spots.png")
	local matrix = Matrix.new(1, 0, 0, 1, 48, 50)

	local shape = Shape.new()

	shape:setFillStyle(Shape.TEXTURE, texture, matrix)

	shape:beginPath(Shape.NON_ZERO)
	shape:moveTo(160, 50)
	shape:lineTo(74, 284)
	shape:lineTo(272, 130)
	shape:lineTo(48, 130)
	shape:lineTo(246, 284)
	shape:closePath()
	shape:endPath()

	local t1 = TextField.new(nil, "setFillStyle(Shape.TEXTURE, texture, matrix)")
	t1:setX(60)
	t1:setY(320)
	shape:addChild(t1)

	local t2 = TextField.new(nil, "winding = Shape.NON_ZERO")
	t2:setX(60)
	t2:setY(332)
	shape:addChild(t2)

	return shape
end

local shapes = {
	createShape1(),
	createShape2(),
	createShape3(),
	createShape4(),
	createShape5(),
}

local current = 1
stage:addChild(shapes[current])

local leftup = Bitmap.new(Texture.new("left-up.png"))
local leftdown = Bitmap.new(Texture.new("left-down.png"))
local left = Button.new(leftup, leftdown)
left:setPosition(106, 400)
stage:addChild(left)


local rightup = Bitmap.new(Texture.new("right-up.png"))
local rightdown = Bitmap.new(Texture.new("right-down.png"))
local right = Button.new(rightup, rightdown)
right:setPosition(166, 400)
stage:addChild(right)

function onClick(event)
	local next = current
	
	if event:getTarget() == left then
		next = next - 1
		if next < 1 then
			next = #shapes
		end
	elseif event:getTarget() == right then
		next = next + 1
		if next > #shapes then
			next = 1
		end
	end
	
	stage:removeChild(shapes[current])
	stage:addChild(shapes[next])
	current = next
end

left:addEventListener("click", onClick)
right:addEventListener("click", onClick)
