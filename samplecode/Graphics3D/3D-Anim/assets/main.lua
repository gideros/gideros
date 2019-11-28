--Set up a fullscreen 3D viewport
-- Note this is the almost the same setup as in 3D Physics exemple, 
-- take a look at it for more comments in the code
local sw,sh=application:getContentWidth(),application:getContentHeight()
local view=D3.View.new(sw,sh,45,0.1,1000)

--Add a blue sky background
local sky=Pixel.new(0xFFFFFF,1,sw*3,sh*3)
sky:setColor(0x00FFFF,1,0x0040FF,1,90) sky:setPosition(-sw,-sh)
stage:addChild(sky)
stage:addChild(view)

--Build a 100x100 floor plane (with normals)
local gplane=D3.Mesh.new()
gplane:setVertexArray{-100,0,-100, 100,0,-100, 100,0,100, -100,0,100}
local tw,th=3200,3200
gplane:setTextureCoordinateArray{0,0,tw,0,tw,th,0,th}
gplane:setGenericArray(3,Shader.DFLOAT,3,4,{
0,1,0,0,1,0,0,1,0,0,1,0,
})
gplane:setIndexArray{1,2,3,1,3,4}
gplane:setTexture(Texture.new("grass.png",true,{wrap=TextureBase.REPEAT}))
gplane:updateMode(D3.Mesh.MODE_TEXTURE|D3.Mesh.MODE_LIGHTING|D3.Mesh.MODE_SHADOW,0)

local scene=view:getScene()
scene:addChild(gplane)

-- Load our model in gdx/g3dj format, and specify the format
local m=buildGdx("Models/characterLargeFemale.json",{
 ["skin"]={
	textureFile="Skins/fantasyFemaleB.png",
 },
})

--Scale it down (it is too big) and add it to the scene
m:setScale(0.01,0.01,0.01)
scene:addChild(m)

--Setup light
Lighting.setLight(5,10,10,0.3)
Lighting.setLightTarget(0,2,0,30)

-- Load two animations from g3dj files
local animWalk=buildGdx("Animations/walk.json",{})
local animIdle=buildGdx("Animations/idle.json",{})
-- Default animation is idle
-- note that in our files, first animation in array the T-pose, the second is the real anim
D3Anim.setAnimation(m,animIdle.animations[2],"main",true)

-- React on mouse down/up to start walking
local walk,wang=false,0
stage:addEventListener(Event.MOUSE_DOWN,function()
	D3Anim.setAnimation(m,animWalk.animations[2],"main",true,0.5)
	walk=true
end)
stage:addEventListener(Event.MOUSE_UP,function()
	D3Anim.setAnimation(m,animIdle.animations[2],"main",true,0.5)
	walk=false
end)

-- Game loop
stage:addEventListener(Event.ENTER_FRAME,function(e)
	-- Position our model according to its path angle
	if walk then wang+=2/3 end
	local an,r=^<wang,3
	local as,ac=math.sin(an)*r,math.cos(an)*r
	m:setRotationY(wang+90)
	m:setPosition(as,0.2,ac)

	--Look at it
	view:lookAt(0,3,5,as,2,ac)

	--Compute shadows
	Lighting.computeShadows(scene)
	--Animation engine tick
	D3Anim.tick()
end) 

-- Show a note on the top of screen to let the user know how to interact
local text=TextField.new(nil,"Click to walk!")
stage:addChild(text) text:setPosition(10,20)
