IsometricTileMap = Core.class(Sprite)

function IsometricTileMap:init(width, height, 
							   texture,
							   tilewidth, tileheight,
							   spacingx, spacingy, 
							   marginx, marginy,
							   displaywidth, displayheight)
	spacingx = spacingx or 0
	spacingy = spacingy or 0
	marginx = marginx or 0
	marginy = marginy or 0
	displaywidth = displaywidth or tilewidth
	displayheight = displayheight or tileheight

	self.width = width
	self.height = height
	self.texture = texture
	self.tilewidth = tilewidth
	self.tileheight = tileheight	
	self.spacingx = spacingx
	self.spacingy = spacingy
	self.marginx = marginx
	self.marginy = marginy
	self.displaywidth = displaywidth
	self.displayheight = displayheight

	local height2 = width + height - 1
	local width2 = math.min(width, height)

	local rows = {}
	self.rows = rows
	for j=1,height2 do
		local mesh = Mesh.new()
		mesh:setTexture(texture)
		self:addChild(mesh)
		rows[#rows + 1] = mesh
	end

	for j=1,height2 do
		local width3
		if j < width2 then
			width3 = j
		elseif j > height2 - width2 + 1 then
			width3 = height2 + 1 - j
		else
			width3 = width2
		end	

		local ind = 1
		local vert = 1
		local mesh = rows[j]
		
		mesh:resizeVertexArray(width3 * 4)
		mesh:resizeTextureCoordinateArray(width3 * 4)
		
		for i=1,width3 do
			mesh:setIndices(ind, vert,
							ind + 1, vert + 1,
							ind + 2, vert + 2,
							ind + 3, vert,
							ind + 4, vert + 2, 
							ind + 5, vert + 3)

			ind = ind + 6
			vert = vert + 4
		end
	end
end

function IsometricTileMap:setTile(x, y, tx, ty)
	local tilewidth = self.tilewidth
	local tileheight = self.tileheight
	local displaywidth = self.displaywidth
	local displayheight = self.displayheight
	
	local j = x + y - 1
	local mesh = self.rows[j]
	local index = math.min(0, self.height - j) + x
	local vert = (index - 1) * 4 + 1
	
	local px = (x - y - 1) * displaywidth / 2
	local py = (x + y - 2) * displayheight / 2 - (tileheight - displayheight)
	
	mesh:setVertices(vert, px, py, 
					 vert + 1, px + tilewidth, py,
					 vert + 2, px + tilewidth, py + tileheight,
					 vert + 3, px, py + tileheight)
					 
	local tx2 = self.marginx + (tx - 1) * (tilewidth + self.spacingx)
	local ty2 = self.marginy + (ty - 1) * (tileheight + self.spacingy)
	
	mesh:setTextureCoordinates(vert, tx2, ty2, 
							   vert + 1, tx2 + tilewidth, ty2,
							   vert + 2, tx2 + tilewidth, ty2 + tileheight,
							   vert + 3, tx2, ty2 + tileheight)
end

