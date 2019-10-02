r3d=require "reactphysics3d"
local world=r3d.World.new(0,-9.8,0)

--Set up a fullscreen 3D viewport
local sw,sh=application:getContentWidth(),application:getContentHeight()
local scene=Sprite.new()
local view=Viewport.new()
stage:addChild(view)
view:setPosition(sw/2,sh/2)
view:setContent(scene)
view:setScale(-sw/2,-sh/2,1)
local proj=Matrix.new()
proj:perspectiveProjection(45,sw/sh,0.1,1000)
view:setProjection(proj)

-- Build a generic CUBE Mesh with normals
local GENCUBE=Mesh.new(true)
GENCUBE:setVertexArray{
	-1,-1,-1, 1,-1,-1, 1,1,-1, -1,1,-1,
	-1,-1,1, 1,-1,1, 1,1,1, -1,1,1,
	
	-1,-1,-1, 1,-1,-1, 1,-1,1, -1,-1,1,
	-1,1,-1, 1,1,-1, 1,1,1, -1,1,1,
	
	-1,-1,-1, -1,1,-1, -1,1,1, -1,-1,1,
	1,-1,-1, 1,1,-1, 1,1,1, 1,-1,1,
}
local tw,th=80,80
GENCUBE:setTextureCoordinateArray{
	0,0,tw,0,tw,th,0,th,
	0,0,tw,0,tw,th,0,th,
	0,0,tw,0,tw,th,0,th,
	0,0,tw,0,tw,th,0,th,
	0,0,tw,0,tw,th,0,th,
	0,0,tw,0,tw,th,0,th,
}
GENCUBE:setGenericArray(3,Shader.DFLOAT,3,24,{
0,0,-1,0,0,-1,0,0,-1,0,0,-1,
0,0,1,0,0,1,0,0,1,0,0,1,
0,-1,0,0,-1,0,0,-1,0,0,-1,0,
0,1,0,0,1,0,0,1,0,0,1,0,
-1,0,0,-1,0,0,-1,0,0,-1,0,0,
1,0,0,1,0,0,1,0,0,1,0,0,
})
GENCUBE:setIndexArray{
1,2,3,1,3,4,
5,6,7,5,7,8,
9,10,11,9,11,12,
13,14,15,13,15,16,
17,18,19,17,19,20,
21,22,23,21,23,24}
GENCUBE:setTexture(Texture.new("box.png",true))
GENCUBE:setShader(Lighting.normal_shader_ts) --Use the textured with shadows shader
--GENCUBE shape
GENCUBE.shape=r3d.BoxShape.new(1,1,1)

--Build a 100x100 floor plane (with normals)
local gplane=Mesh.new(true)
gplane:setVertexArray{-100,1,-100, 100,1,-100, 100,1,100, -100,1,100}
local tw,th=3200,3200
gplane:setTextureCoordinateArray{0,0,tw,0,tw,th,0,th}
gplane:setGenericArray(3,Shader.DFLOAT,3,4,{
0,1,0,0,1,0,0,1,0,0,1,0,
})
gplane:setIndexArray{1,2,3,1,3,4}
gplane:setTexture(Texture.new("grass.png",true,{wrap=TextureBase.REPEAT}))
gplane:setShader(Lighting.normal_shader_ts) --Use the textured with shadows shader

--Plane body
gplane.body=world:createBody()
gplane.body:setType(r3d.Body.STATIC_BODY)
gplane.body:createFixture(r3d.BoxShape.new(100,1,100),nil,1000)
--Make it blue
--gplane:setColorTransform(0,0,1,1)


--Function to build a cube instance
world.bodies={}
function build_cube(size)
	local v=Viewport.new()
	v:setContent(GENCUBE)
	v:setPosition(math.random()*9-5,math.random()*10+10,math.random()*9-5)
	v:setRotation(math.random()*20-10)
	v:setRotationX(math.random()*20-10)
	v:setRotationY(math.random()*20-10)
	local body=world:createBody(v:getMatrix())
	body:createFixture(GENCUBE.shape,nil,1)
	v.body=body
	world.bodies[v]=v.body
	return v
end

Lighting.setLight(15,30,0,0.3)
scene:addChild(gplane)
scene:addChild(build_cube(1))
local sky=Pixel.new(0xFFFFFF,1,320,480)
sky:setColor(0x00FFFF,1,0x0040FF,1,90) sky:setY(-240)
stage:addChildAt(sky,1)
view:lookAt(0,10,-20,0,5,0,0,1,0)

--Kickoff shadows
Lighting.computeShadows(scene)
gplane:setTexture(Lighting.shadowrt,2)
GENCUBE:setTexture(Lighting.shadowrt,2)

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