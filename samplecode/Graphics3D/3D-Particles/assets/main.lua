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
gplane:updateMode(D3.Mesh.MODE_TEXTURE|D3.Mesh.MODE_LIGHTING,0)

--Configure light
Lighting.setLight(15,30,0,0.3)
Lighting.setLightTarget(0,0,0,40,120)

local scene=view:getScene()
scene:addChild(gplane)

local props=Glb.new(nil,"props.glb")
local propss=G3DFormat.buildG3D(props:getScene())
propss:updateMode(D3.Mesh.MODE_LIGHTING,D3.Mesh.MODE_SHADOW)
propss:setScale(1,1,1)
scene:addChild(propss)

-- We will embed our particles in a 3D mesh, to enable depth testing
local sc=Mesh.new(true) scene:addChild(sc)

-- A particle set showing a bubble, for a fountain
local fountain=Particles.new(true,true)
fountain:setTexture(Texture.new("Bubble.png",true))
sc:addChild(fountain)
fountain:setX(-1)
fountain:setY(.5)

-- A particle set showing a noisy pattern for fire
local fire=Particles.new(true,true)
fire:setTexture(Texture.new("noise.png",true))
sc:addChild(fire)
fire:setX(1)
fire:setY(.4)

--Our emitters, for fountain and fire
function Particles:fountain()
	local da=math.random()*6.28
	local dh=math.cos(os:clock()/2)
	local dr=math.random()*.001+.008*math.sin(os:clock()/2)
	self:addParticles({{x=0,y=0,size=.03,ttl=200,
		speedY=.02+.03*math.abs(dh),speedX=dr*math.sin(da),speedZ=dr*math.cos(da),_decay=vector(1,1,1),acceleration=vector(0,-.0005,0),
		color=vector(0.3+math.random()*.1,.7,.7+math.random()*.3,1)}})
end

function Particles:fire()
	local ag=math.random()*6.282
	local rd=math.cos(math.random()*3.1415/2)*.3
	local dx=math.sin(ag)*rd
	local dz=math.cos(ag)*rd
	
	self:addParticles({{x=dx,y=0,z=dz,size=.2,ttl=200,
		speedY=.02+.03*math.random(),speedX=.02*(math.random()-.5),speedZ=.02*(math.random()-.5),decay=vector(1,0.99-math.random()*0.01,1),		
		color=vector(1,1,1,1), angularSpeed=math.random(), angle=math.random()*360,decayAlpha=0.98}})
end


local r=90
-- Game loop
stage:addEventListener(Event.ENTER_FRAME,function(e)
	for i=1,50 do
		fountain:fountain()
	end
	for i=1,20 do
		fire:fire()
	end
	--Look at it
	r+=.1
	view:lookAt(math.cos(^<r)*5,3,math.sin(^<r)*5,0,2,0)
	--Lighting.computeShadows(scene)
end) 

-- Lua definition for Gideros standard 3D particle shader
-- Not necessary here but useful to get started with custom shaders
local function stdPS3VShader(vVertex,vColor,vTexCoord) : Shader
    local rad=hF2(-0.5,-0.5)+vTexCoord.xy
    local angle=^<vTexCoord.w
    local ca=cos(angle)
    local sa=sin(angle)
    local rot=hF22(ca,sa,-sa,ca)
    rad=rad*rot
    fInColor=hF4(vColor)*fColor
    local xpsize=vWorldMatrix*hF4(vTexCoord.z,0.0,0.0,0.0)
    local xpl=length(xpsize.xyz)
    if (xpl==0.0) then xpl=1.0 end
    local vertex = vViewMatrix*(vWorldMatrix*hF4(vVertex.xyz,1.0))
    vertex.xy+=rad*xpl
    fStepRot=hF2(sign(vTexCoord.z)/100.0,vTexCoord.w)
    fTexCoord=vTexCoord.xy
	return vProjMatrix*vertex
end
local function stdPSFShader() : Shader
	if fStepRot.x==0.0 then 
		discard()
	elseif fStepRot.x<0.0 then
	else
		local rad=hF2(-0.5,-0.5)+fTexCoord
		if fTexInfo.x<=0.0 then
			local alpha=lF1(1.0-smoothstep(0.5-fStepRot.x,0.5+fStepRot.x,length(rad)))
			return lF4(fInColor*alpha)
		else
			if (rad.x<-0.5) or (rad.y<-0.5) or (rad.x>0.5) or (rad.y>0.5) then
				discard()
			end		
			return lF4(fInColor)*texture2D(fTexture, (rad+hF2(0.5,0.5))*fTexInfo.xy)
		end
	end
	return lF4(fInColor)
end

local pshader=Shader.lua(stdPS3VShader,stdPSFShader,0,
{
	{name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=true},
	{name="vWorldMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WORLD,vertex=true},
	{name="vViewMatrix",type=Shader.CMATRIX,sys=Shader.SYS_VIEW,vertex=true},
	{name="vProjMatrix",type=Shader.CMATRIX,sys=Shader.SYS_PROJECTION,vertex=true},
	{name="fTexture",type=Shader.CTEXTURE,vertex=false},
	{name="fTexInfo",type=Shader.CFLOAT4,sys=Shader.SYS_TEXTUREINFO,vertex=false},
	},
	{
	{name="vVertex",type=Shader.DFLOAT,mult=4,slot=0,offset=0},
	{name="vColor",type=Shader.DUBYTE,mult=4,slot=1,offset=0},
	{name="vTexCoord",type=Shader.DFLOAT,mult=4,slot=2,offset=0},
	},
	{
	{name="fInColor",type=Shader.CFLOAT4},
	{name="fStepRot",type=Shader.CFLOAT2},
	{name="fTexCoord",type=Shader.CFLOAT2},
	}
)

--fountain:setShader(pshader)

-- Our fire will need a specific fragment shader, to apply our color gradient
local function fireFShader() : Shader
	if fStepRot.x==0.0 then 
		discard()
	end
	local rad=hF2(-0.5,-0.5)+fTexCoord
	local c=texture2D(fTexture2,hF2(1-fInColor.a,0))
	c*=fInColor.a
	c=c*texture2D(fTexture, (rad+hF2(0.5,0.5))*fTexInfo.xy)
	return c
end

local fireShader=Shader.lua(stdPS3VShader,fireFShader,0,
{
	{name="fColor",type=Shader.CFLOAT4,sys=Shader.SYS_COLOR,vertex=true},
	{name="vWorldMatrix",type=Shader.CMATRIX,sys=Shader.SYS_WORLD,vertex=true},
	{name="vViewMatrix",type=Shader.CMATRIX,sys=Shader.SYS_VIEW,vertex=true},
	{name="vProjMatrix",type=Shader.CMATRIX,sys=Shader.SYS_PROJECTION,vertex=true},
	{name="fTexture",type=Shader.CTEXTURE,vertex=false},
	{name="fTexture2",type=Shader.CTEXTURE,vertex=false},
	{name="fTexInfo",type=Shader.CFLOAT4,sys=Shader.SYS_TEXTUREINFO,vertex=false},
	},
	{
	{name="vVertex",type=Shader.DFLOAT,mult=4,slot=0,offset=0},
	{name="vColor",type=Shader.DUBYTE,mult=4,slot=1,offset=0},
	{name="vTexCoord",type=Shader.DFLOAT,mult=4,slot=2,offset=0},
	},
	{
	{name="fInColor",type=Shader.CFLOAT4},
	{name="fStepRot",type=Shader.CFLOAT2},
	{name="fTexCoord",type=Shader.CFLOAT2},
	}
)

fire:setShader(fireShader)
fire:setTexture(Texture.new("fire.png",true),1)
