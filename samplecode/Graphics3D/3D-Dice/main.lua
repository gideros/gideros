
application:configureFrustum(45)

function dot(s,dx,dy)
	s:beginPath()
	s:circle(dx,dy,20,false)
	s:fill()
end

local function face(d1,d2,d4,d6,rx,ry)
	c=Sprite.new()
	sb=NdShape.new()
	sb:setFillStyle(Shape.SOLID, 0xFF0000,1.0)
	sb:beginPath()
	sb:rect(0,0,200,200)
	sb:fill()
	s=NdShape.new()
	s:setFillStyle(Shape.SOLID, 0x000000,1.0)
	sb:addChild(s)
	local dspace=50
	if d1 then
		dot(s,0,0)
	end
	if d2 then
		dot(s,-dspace,-dspace)
		dot(s,dspace,dspace)
	end
	if d4 then
		dot(s,-dspace,dspace)
		dot(s,dspace,-dspace)
	end
	if d6 then
		dot(s,-dspace,0)
		dot(s,dspace,0)
	end
	s:setLineStyle(6,0x800000,1.0)
	s:beginPath()
	s:rect(0,0,200,200)
	s:stroke()

	sb:setZ(-100)
	s:setZ(-1)
	c:addChild(sb)
	c:setRotationX(rx)
	c:setRotationY(ry)
	return c;
end

dice=Mesh.new(true)
dice:addChild(face(true,false,false,false,0,0))
dice:addChild(face(false,true,false,false,90,0))
dice:addChild(face(true,true,false,false,-90,0))
dice:addChild(face(false,true,true,false,180,0))
dice:addChild(face(true,true,true,false,0,90))
dice:addChild(face(false,true,true,true,0,-90))

dice:setScale(1)
dice:setPosition(150,150,-250)

base=Sprite.new()
base:addChild(dice)
--base:setClip(150,150,100,100)

stage:addChild(base)
stage:addChild(Sprite.new())


dice:addEventListener(Event.ENTER_FRAME,function (cube)
	cube:setRotationX(cube:getRotationX()+1)
	cube:setRotationY(cube:getRotationY()+1.2)
	cube:setRotation(cube:getRotation()+1.3)
end,dice)

