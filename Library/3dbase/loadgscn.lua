local Files={}

function Files.loadJson(file)
	local js=Files.load(file)
	if js:sub(1,1)=="x" then js=zlib.decompress(js) end
	local ok,ret=pcall(json.decode,js)
	if not ok then return end
	return ret
end

function Files.load(file)
	local fd=io.open(file,"rb")
	if not fd then return end
	local js=fd:read("*all")
	fd:close()
	return js
end

local function makeCylinderShape(steps,r,h)
	--For collision, ensure closed/CCW shape
	local va,ca,fa={},{},{}
	local nc,nf,idx=1,1,1
	local rs=(2*math.pi)/steps
	for ix=0,steps-1 do
		local x=math.cos(ix*rs)*r
		local z=-math.sin(ix*rs)*r
		--EDGE-TOP
		va[idx]=x idx+=1
		va[idx]=h idx+=1
		va[idx]=z idx+=1
		--EDGE-BOTTOM
		va[idx]=x idx+=1
		va[idx]=-h idx+=1
		va[idx]=z idx+=1
	end

	
	for i=1,steps*2-1,2 do ca[nc]=i nc+=1 end --TOP SURFACE	
	fa[nf]=steps nf+=1
	for i=1,steps*2-1,2 do
		ca[nc]=i nc+=1 ca[nc]=i+1 nc+=1 
		ca[nc]=i+3 nc+=1 ca[nc]=i+2 nc+=1 
		fa[nf]=4 nf+=1
	end		
	ca[nc-2]=2 ca[nc-1]=1
	for i=steps*2,2,-2 do ca[nc]=i nc+=1 end
	fa[nf]=steps nf+=1
	return r3d.ConvexMeshShape.new(va,ca,fa)
end

local GScene={}
function GScene.makePhysicsShape(shapetype,dimx,dimy,dimz)
	local shape
	if shapetype=="sphere" then
		shape=r3d.SphereShape.new(dimx<>dimy<>dimz)
	elseif shapetype=="capsule" then
		shape=r3d.CapsuleShape.new(dimx<>dimz,dimy)
	elseif shapetype=="cylinder" then
		shape=makeCylinderShape(10,(dimx<>dimz),dimy)
	else
		shape=r3d.BoxShape.new(dimx,dimy,dimz)
	end
	return shape
end

local function MakeBody(self,spec,world)
	if spec.bodytype=="ignore" then return end
	local r3d=require "reactphysics3d"
	local body=world:createBody()
	if spec.bodytype=="kinetic" then
		body:setType(r3d.Body.KINETIC_BODY)
	elseif spec.bodytype=="dynamic" then
		body:setType(r3d.Body.DYNAMIC_BODY)
		local dynb=world.dynamicBodies or {}
		world.dynamicBodies=dynb
		dynb[body]=self
	else
		body:setType(r3d.Body.STATIC_BODY)
	end

	local m=self
	local ft=spec and spec.transform
	if not ft then
		ft=Matrix.new()
		ft:setPosition(m.center[1],m.center[2],m.center[3])
	end
	local sx,sy,sz=ft:getScale()
	local stx,sty,stz=m:getScale()
	local dimx,dimy,dimz=(m.max[1]-m.min[1])/2,(m.max[2]-m.min[2])/2,(m.max[3]-m.min[3])/2 --Those are half dimensions
	local shapedim=vector(dimx,dimy,dimz)
	local shape
	local shapetype=spec and spec.shape
	dimx=dimx*sx*stx
	dimy=dimy*sy*sty
	dimz=dimz*sz*stz
	shape=GScene.makePhysicsShape(shapetype,dimx,dimy,dimz)
	if shape then
		body.shapedim=shapedim
		body.shape=shape
		body.shapetype=shapetype
		local fft=ft:duplicate()
		local tx,ty,tz=fft:getPosition()
		fft:setPosition(tx*stx,ty*sty,tz*stz)
		fft:setScale(1,1,1)
		body.fixture=body:createFixture(shape,fft,1000)
		local mat = body.fixture:getMaterial()
		mat.bounciness = 0 -- 0 = no bounciness, 1 = max bounciness
		body.fixture:setMaterial(mat)
		body.fixtureTransform=ft
	end
	self.body=body
	body.model=self
	local ft=m:getMatrix()
	ft:setScale(1,1,1)
	self.body:setTransform(ft)
end

function LoadGScene(libpath,file,world)
	local loadGroup
	local scene=Sprite.new()
	scene.refs={}
	function loadGroup(g,p)
		p.name=g.name
		p.tag=g.tag
		if p.name then scene.refs[p.name]=p end
		if g.transform then
			if g.transform.position then
				p:setPosition(g.transform.position[1],g.transform.position[2],g.transform.position[3])
			end
			if g.transform.rotation then
				p:setRotationX(g.transform.rotation[1])
				p:setRotationY(g.transform.rotation[2])
				p:setRotation(g.transform.rotation[3])
			end
			if g.transform.scale then
				p:setScale(g.transform.scale[1],g.transform.scale[2],g.transform.scale[3])
			end
		end
		for _,s in ipairs(g.children) do
			if s.asset then	
				local data=Files.loadJson(libpath.."/"..s.asset.lib.."/"..s.asset.name..".g3d")
				G3DFormat.computeG3DSizes(data)
				local m=G3DFormat.buildG3D(data,nil,nil,{
					resolvePath=function(path,type)
						return libpath.."/"..s.asset.lib.."/"..path
					end
				})
				m.name=s.name
				m.tag=s.tag
				m.asset=s.asset
				if s.transform then
					local t=m
					if s.transform.position then
						t:setPosition(s.transform.position[1],s.transform.position[2],s.transform.position[3])
					end
					if s.transform.rotation then
						t:setRotationX(s.transform.rotation[1])
						t:setRotationY(s.transform.rotation[2])
						t:setRotation(s.transform.rotation[3])
					end
					if s.transform.scale then
						t:setScale(s.transform.scale[1],s.transform.scale[2],s.transform.scale[3])
					end
				end
				if s.physics and world then
					local s=s.physics
					local pspec={ shape=s.shapetype, bodytype=s.bodytype }
					if s.transform then
						local t=Matrix.new()
						pspec.transform=t
						if s.transform.position then
							t:setPosition(s.transform.position[1],s.transform.position[2],s.transform.position[3])
						end
						if s.transform.rotation then
							t:setRotationX(s.transform.rotation[1])
							t:setRotationY(s.transform.rotation[2])
							t:setRotationZ(s.transform.rotation[3])
						end
						if s.transform.scale then
							t:setScale(s.transform.scale[1],s.transform.scale[2],s.transform.scale[3])
						end
					end
					MakeBody(m,pspec,world)
				end
				if m.name then scene.refs[m.name]=m end
				p:addChild(m)
			else
				local m=Sprite.new()
				loadGroup(s,m)
				p:addChild(m)
			end
		end
	end
	local project=Files.loadJson(file)
	loadGroup(project.scene,scene)
	return scene
end
D3=D3 or {}
D3.GScene=GScene
