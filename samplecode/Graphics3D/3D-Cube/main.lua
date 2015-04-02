
application:configureFrustum(45)

local function face(color,rx,ry)
	c=Sprite.new()
	s=Shape.new()
	s:setFillStyle(Shape.SOLID, color,0.8)
	s:beginPath()
	s:moveTo(-1,-1)
	s:lineTo(-1,1)
	s:lineTo(1,1)
	s:lineTo(1,-1)
	s:lineTo(-1,-1)
	s:endPath()

	--[[mesh:setVertexArray(-1,-1,-1,
	                    -1, 1,-1,
						 1, 1,-1,
						 1,-1,-1)
	mesh:setColorArray(color,0.5,color,1,color,0.5,color,1)
	mesh:setIndexArray(1,2,3,1,3,4)]]
	s:setZ(-1)
	c:addChild(s)
	c:setRotationX(rx)
	c:setRotationY(ry)
	return c;
end

cube=Mesh.new(true)
cube:addChild(face(0xFF0000,0,0))
cube:addChild(face(0xFFFF00,90,0))
cube:addChild(face(0xFF00FF,-90,0))
cube:addChild(face(0x00FF00,180,0))
cube:addChild(face(0x00FFFF,0,90))
cube:addChild(face(0x0000FF,0,-90))

cube:setScale(100)
cube:setPosition(150,150,-250)

base=Sprite.new()
base:addChild(cube)
--cube:setClip(-0.5,-0.5,1,1)

stage:addChild(base)
stage:addChild(Sprite.new())

cube:addEventListener(Event.ENTER_FRAME,function ()
	cube:setRotationX(cube:getRotationX()+1)
	cube:setRotationY(cube:getRotationY()+1.2)
	cube:setRotation(cube:getRotation()+1.3)
end)

