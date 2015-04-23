--[[
Demonstrate 3D cube environnement mapping with Gideros 3D features.
Imagery from google Street View, for demo purposes only.
]]

local lat=48.484421 -- Latitude of the location to view
local lon=-4.771717 -- Longitude of the location to view
local hdg=0 -- Initial heading

application:setLogicalDimensions(320,480)

-- Confiure a 3D frustum with 90° field of view
application:configureFrustum(90,10000)

local LoadingImg=Texture.new("Hourglass.png") -- Texture used while loading the tiles

-- Make a 3D cube with 6 bitmap faces
local ihdg={ 0, 180, 270, 90, 0, 0 }
local pitch = { 0, 0, 0, 0, 90, -90 }
local e=Mesh.new(true) -- The mesh containing the faces is a 3D one
for f=1,6,1 do
	-- Build a google URL for this face
	local url = "http://maps.googleapis.com/maps/api/streetview?size=512x512&location="
				..lat..","..lon.."&fov=90&heading="..tostring((ihdg[f] + hdg) % 360).."&pitch="..tostring(pitch[f]).."&sensor=true"
	c=Sprite.new()
	b=Bitmap.new(LoadingImg)
	local ldr=UrlLoader.new(url)
	b.lname="img"..f
	ldr:addEventListener(Event.COMPLETE, function (tgt,evt)
			--Tile loaded, assign it to the bitmap
			local fn="|T|tile"..tgt.lname..".jpg"
			local out = io.open(fn, "wb")
			out:write(evt.data)
			out:close()
			local txt=Texture.new(fn)
			tgt:setTexture(txt)
			os.remove(fn)
		end,b)
	--Overscale a little in X and Y, to avoid seeing the edges of the cube
	b:setScale(1.001,1.001,1) 
	-- Tiles are 512x512, centers them on X and Y and push them along Z axis
	b:setPosition(-256,-256,-256)
	-- We use a second enclosing sprite to rotate the pre-translated face
	c:addChild(b)
	c:setRotationX(-pitch[f])
	c:setRotationY(-ihdg[f])
	e:addChild(c)
end

-- Enlarge the cube and center it on the screen
e:setScale(10)
e:setPosition(160,240,0)
stage:addChild(e)

-- Make it rotate
e:addEventListener(Event.ENTER_FRAME,function (cube)
	--cube:setRotationX(cube:getRotationX()+1)
	cube:setRotationY(cube:getRotationY()+0.4)
	--cube:setRotation(cube:getRotation()+1.3)
end,e)