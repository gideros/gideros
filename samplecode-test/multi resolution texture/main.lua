local texture = Texture.new("image.png")

-- test 1
local tm = TileMap.new(	10, 10,
						texture,
						16, 16,
						0, 0,
						0, 0,
						16, 16)
for i=1,4 do
	for j=1,4 do
		tm:setTile(i, j, i, j)
	end
end
tm:setPosition(0, 0)
stage:addChild(tm)

-- test 2
local shape = Shape.new()
shape:setFillStyle(Shape.TEXTURE, texture)
shape:beginPath()
shape:moveTo(0, 0)
shape:lineTo(100, 0)
shape:lineTo(100, 80)
shape:lineTo(0, 80)
shape:closePath()
shape:endPath()
shape:setPosition(0, 100)
stage:addChild(shape)

-- test 3
local mesh = Mesh.new()
mesh:setVertexArray(0, 0, 100, 0, 100, 80, 0, 80)
mesh:setIndexArray(1, 2, 3, 1, 3, 4) 
mesh:setTextureCoordinateArray(0, 0, 100, 0, 100, 80, 0, 80) 
mesh:setTexture(texture)
mesh:setPosition(0, 200)
stage:addChild(mesh)


