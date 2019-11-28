r3d=require "reactphysics3d"
-- We'll use 3D support library in Library/3dbase folder of gideros installation

-- Create physics world
local world=r3d.World.new(0,-9.8,0)

--Set up a fullscreen 3D viewport
local sw,sh=application:getContentWidth(),application:getContentHeight()
local view=D3.View.new(sw,sh,45,0.1,1000) -- 45 is the Field Of View, 0.1 is the near plane and 1000 the far plane

-- Build a generic CUBE Mesh with normals
--  Unit cube, extending from -1 to +1 in each direction
local GENCUBE=D3.Cube.new() 
--  Map our carte texture onto it
GENCUBE:mapTexture(Texture.new("box.png",true))
--  Enable lighting and shadows
GENCUBE:updateMode(D3.Mesh.MODE_LIGHTING|D3.Mesh.MODE_SHADOW)
--	Build the corresponding reactphysics3d collision shape
GENCUBE.shape=r3d.BoxShape.new(1,1,1)

--Build a 100x100 floor plane (with normals)
local gplane=D3.Mesh.new()
gplane:setVertexArray{-100,1,-100, 100,1,-100, 100,1,100, -100,1,100}
-- Large texture coordinates, the texture will repeat
local tw,th=3200,3200 
gplane:setTextureCoordinateArray{0,0,tw,0,tw,th,0,th}
-- This is the normal array
gplane:setGenericArray(3,Shader.DFLOAT,3,4,{
0,1,0,0,1,0,0,1,0,0,1,0,
})
gplane:setIndexArray{1,2,3,1,3,4}
-- Set the grass texture
gplane:setTexture(Texture.new("grass.png",true,{wrap=TextureBase.REPEAT}))
-- Enable lighting, shadows, and texture (since setTexture doesn't automatically do that)
gplane:updateMode(D3.Mesh.MODE_LIGHTING|D3.Mesh.MODE_SHADOW|D3.Mesh.MODE_TEXTURE)

-- Build a reactphysics3d body for the plane
gplane.body=world:createBody()
-- It will be a static body
gplane.body:setType(r3d.Body.STATIC_BODY)
gplane.body:createFixture(r3d.BoxShape.new(100,1,100),nil,1000)


--Function to build a cube instance
world.bodies={}
function build_cube(size)
	-- We actually build a viewport onto the generic cube we set up earlier
	local v=Viewport.new()
	v:setContent(GENCUBE)
	-- And randomly place/rotate it
	v:setPosition(math.random()*9-5,math.random()*10+10,math.random()*9-5)
	v:setRotation(math.random()*20-10)
	v:setRotationX(math.random()*20-10)
	v:setRotationY(math.random()*20-10)
	-- Create body accordingly
	local body=world:createBody(v:getMatrix())
	body:createFixture(GENCUBE.shape,nil,1)
	v.body=body
	-- Add the body to our moving bodies list
	world.bodies[v]=v.body
	return v
end

--Setup our scene: add our plane
local scene=view:getScene()
scene:addChild(gplane)
--Configure light
Lighting.setLight(15,30,0,0.3)
Lighting.setLightTarget(0,0,0,40,120)
--Setup camera
view:lookAt(0,10,-20,0,5,0)

--Setup the stage
-- Add a blue gradient background for the sky
local sky=Pixel.new(0xFFFFFF,1,320*3,480*3)
sky:setColor(0x00FFFF,1,0x0040FF,1,90) sky:setPosition(-320,-240-480)
stage:addChild(sky)
-- Add our 3D view on top of that
stage:addChild(view)

-- Main loop: 
--  add falling cubes every 100 frames
--  tick the physics engine
--  update sprites with the new bodies transforms

local gen=0
stage:addEventListener(Event.ENTER_FRAME,function(e)
	Lighting.computeShadows(scene)
	gen=gen+1
	if gen==100 then
		scene:addChild(build_cube(1))	
		gen=0
	end
	world:step(e.deltaTime)
	for s,b in pairs(world.bodies) do
		s:setMatrix(b:getTransform())
	end
end)
