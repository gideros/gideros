-- set up a fullscreen 3D viewport
local sw,sh=application:getContentWidth(),application:getContentHeight()
local view=D3.View.new(sw,sh,45,0.1,1000)
local scene=view:getScene() -- we will add all 3D models to it to build our 3D "scene"

-- a skybox
local skybox=D3.Sphere.new(64,128) -- steps, radius
skybox:mapTexture(Texture.new("textures/envbox.jpg"))
--skybox:setAnchorPosition(0,2,0)
skybox:setPosition(0,2,0)
scene:addChild(skybox) -- we add the skybox to the scene since it is 3D

-- build a 100x100 floor plane (with normals)
local gplane=D3.Mesh.new()
gplane:setVertexArray{-100,0,-100, 100,0,-100, 100,0,100, -100,0,100}
local tw,th=3200,3200
gplane:setTextureCoordinateArray{0,0,tw,0,tw,th,0,th}
gplane:setGenericArray(3,Shader.DFLOAT,3,4,{
	0,1,0,0,1,0,0,1,0,0,1,0,
})
gplane:setIndexArray{1,2,3,1,3,4}
gplane:setTexture(Texture.new("textures/grass.png",true,{wrap=TextureBase.REPEAT}))
gplane:updateMode(D3.Mesh.MODE_TEXTURE|D3.Mesh.MODE_LIGHTING|D3.Mesh.MODE_SHADOW,0)

---- some 3D .obj models
local tree = loadObj("models/objs/tree", "trees02.obj") -- model folder, model name
local bench = loadObj("models/objs/bench", "bench.obj")
-- some glb models too
local vegetation = {}
local mushr01f = Glb.new("models/glbs", "shrooms.glb") -- model folder, model name
local grass01f = Glb.new("models/glbs", "grass.glb")
local pickvegetal, vegetal
for i = 1, 64 do
	if i<=16 then pickvegetal=mushr01f vegetal="mush01"
	else pickvegetal=grass01f vegetal="grass"
	end
	vegetation[i] = G3DFormat.buildG3D(pickvegetal:getScene())
	vegetation[i].vegetal = vegetal
	vegetation[i]:updateMode(D3.Mesh.MODE_LIGHTING)
end

-- our 1st animated character (gdx/g3dj format with materials)
local m=buildGdx("models/characterLargeFemale/characterLargeFemale.json",
	{ modelpath="models/characterLargeFemale/", }
)
-- with two animations
local animWalk=buildGdx("models/characterLargeFemale/walk.json",{})
local animIdle=buildGdx("models/characterLargeFemale/idle.json",{})
-- set default animation to idle
-- note: in our files, the first animation in array is the T-pose, the second is the real anim
D3Anim.setAnimation(m,animIdle.animations[2],"main",true)

-- a 2nd animated character (from mixamo)
local m2=buildGdx("models/theBoss/theBoss_mesh.json",
	{ modelpath="models/theBoss/", }
)
local animIdle2=buildGdx("models/theBoss/theBoss_old_man_anim.json",{})
D3Anim.setAnimation(m2,animIdle2.animations[1],"main",true)

-- TIME TO SET UP OUR SCENE
-- we scale our models
local scale = 1.5
tree:setScale(scale,scale,scale) -- obj
bench:setScale(scale,scale,scale) -- obj
for _, v in ipairs(vegetation) do -- glb
	if v.vegetal == "grass" then
		v:setScale(scale*math.random(1,4)*0.1,scale*math.random(2,7)*0.1,scale*math.random(1,4)*0.1)
	else -- mushrooms
		v:setScale(scale*math.random(5,8)*0.1,scale*math.random(5,9)*0.1,scale*math.random(5,8)*0.1)
	end
end
m:setScale(0.01, 0.01, 0.01) -- json (from fbx)
m2:setScale(0.016, 0.016, 0.016) -- json (from fbx)
-- we rotate some models
bench:setRotationY(110)
for _, v in ipairs(vegetation) do v:setRotationY(math.random(0,360)) end
-- we position our models
bench:setPosition(-3.8, 0, 2)
for _, v in ipairs(vegetation) do
	if v.vegetal == "grass" then v:setPosition(math.random(-10,10), 0, math.random(-7,1))
	else v:setPosition(math.random(-15,15), 0, math.random(-15,1))
	end
end
m:setPosition(0, 0, 0)
m2:setPosition(1, 0, 1.5)

-- now we add the models to the scene
scene:addChild(gplane)
scene:addChild(tree)
scene:addChild(bench)
for _, v in ipairs(vegetation) do
	scene:addChild(v)
end
scene:addChild(m)
scene:addChild(m2)
-- and finally everything to stage
stage:addChild(view)

-- light setup
Lighting.setLight(5,10,10,0.3)
Lighting.setLightTarget(0,2,0,30,45)

-- INPUT: mouse down/up to start/stop walking
local walk=false
stage:addEventListener(Event.MOUSE_DOWN,function()
	--function D3Anim.setAnimation(model,anim,track,loop,transitionTime,speed)
	D3Anim.setAnimation(m,animWalk.animations[2],"main",true,0.5,1)
	walk=true
end)
stage:addEventListener(Event.MOUSE_UP,function()
	D3Anim.setAnimation(m,animIdle.animations[2],"main",true,0.5,0.6)
	walk=false
end)

-- THE GAME LOOP
local wang=0
stage:addEventListener(Event.ENTER_FRAME,function(e)
	-- Position our model according to its path angle
	if walk then wang+=2/3 end
	local an,r=^<wang,3
	local as,ac=math.sin(an)*r,math.cos(an)*r
	m:setRotationY(wang+90)
	m:setPosition(as,0.2,ac)
	-- Look at it
	view:lookAt(0,3,5,as,2,ac)
	-- Animation engine tick
	D3Anim.tick()
	-- Compute shadows
	Lighting.computeShadows(scene)
end) 

-- show a note on the top of screen to let the user know how to interact
local text=TextField.new(nil,"Click to walk!")
text:setTextColor(0xffff00)
stage:addChild(text) text:setPosition(10,20)
