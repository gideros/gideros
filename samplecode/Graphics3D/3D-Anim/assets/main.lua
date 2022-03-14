--Set up a fullscreen 3D viewport
--Note this is the almost the same setup as in 3D Physics exemple,
-- take a look at it for more comments in the code
local sw,sh=application:getContentWidth(),application:getContentHeight()
local view=D3.View.new(sw,sh,45,0.1,1000)
local scene=view:getScene()

--A blue sky background
local sky=Pixel.new(0xFFFFFF,1,sw*3,sh*3)
sky:setColor(0x00FFFF,1,0x0040FF,1,90) sky:setPosition(-sw,-sh)

--This is our stage
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

--Load our 1st model in gdx/g3dj format, and specify the folder for the textures (diffuse, normal)
local m=buildGdx("models/characterLargeFemale/characterLargeFemale.json",
	{ modelpath="models/characterLargeFemale/", }
)
--Scale it down (it is too big)
m:setScale(0.01,0.01,0.01)
--Load two animations from g3dj files
local animWalk=buildGdx("models/characterLargeFemale/walk.json",{})
local animIdle=buildGdx("models/characterLargeFemale/idle.json",{})
--Set default animation to idle
-- note that in our files, first animation in array the T-pose, the second is the real anim
D3Anim.setAnimation(m,animIdle.animations[2],"main",true)

--Load our 2nd more complex model (from mixamo)
local m2=buildGdx("models/theBoss/theBoss_mesh.json",
	{ modelpath="models/theBoss/", } -- don't forget the / at the end
)
m2:setScale(0.015,0.015,0.015)
local animIdle2=buildGdx("models/theBoss/theBoss_old_man_anim.json",{})
-- note that in our files, this is the first animation as there is no T-pose in the file
D3Anim.setAnimation(m2,animIdle2.animations[1],"main",true)

--We position our elements in the scene
m2:setPosition(0, 0, 1.3)
--We add our elements in this order
scene:addChild(gplane)
scene:addChild(m)
scene:addChild(m2)

--Setup light
Lighting.setLight(5,10,10,0.3)
Lighting.setLightTarget(0,2,0,30,45)

--React on mouse down/up to start walking
local walk,wang=false,0
stage:addEventListener(Event.MOUSE_DOWN,function()
	D3Anim.setAnimation(m,animWalk.animations[2],"main",true,0.5)
	walk=true
end)
stage:addEventListener(Event.MOUSE_UP,function()
	D3Anim.setAnimation(m,animIdle.animations[2],"main",true,0.5)
	walk=false
end)

--Game loop
stage:addEventListener(Event.ENTER_FRAME,function(e)
	--Position our model according to its path angle
	if walk then wang+=2/3 end
	local an,r=^<wang,3
	local as,ac=math.sin(an)*r,math.cos(an)*r
	m:setRotationY(wang+90)
	m:setPosition(as,0.2,ac)
	--Lighting.setLightTarget(as,0.2,ac,30,45)
	--Look at it
	view:lookAt(0,3,5,as,2,ac)
	--Animation engine tick
	D3Anim.tick()
	--Compute shadows
	Lighting.computeShadows(scene)
end)

-- Show a note on the top of screen to let the user know how to interact
local text=TextField.new(nil,"Click to walk!")
stage:addChild(text) text:setPosition(10,20)
