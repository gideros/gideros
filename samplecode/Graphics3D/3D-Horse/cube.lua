local function face(color,angleY,angleX)
	local mesh=Mesh.new(true)
	mesh:setVertexArray(-1,-1,-1, -1,1,-1, 1,1,-1, 1,-1,-1)
	mesh:setColors(1,color,1.0,2,color,1.0,3,color,0.5,4,color,0.5)
	mesh:setIndices(1,1,2,2,3,3,4,1,5,3,6,4)
	mesh:setRotationY(angleY)
	mesh:setRotationX(angleX)
	return mesh	
end

function buildCube()
local cube=Sprite.new()
cube:addChild(face(0xff0000,0,0))
cube:addChild(face(0xffff00,90,0))
cube:addChild(face(0xff00ff,-90,0))
cube:addChild(face(0x00ffff,180,0))
cube:addChild(face(0x0000ff,0,-90))
cube:addChild(face(0x00ff00,0,90))
cube:setScale(100)
cube:setPosition(150,150,-200)
cube:setRotationY(20)
cube:setRotationX(40)

return cube;
end
