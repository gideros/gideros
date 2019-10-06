local Mesh3D=Core.class(Mesh,function() return true end)
Mesh3D.MODE_TEXTURE=1
Mesh3D.MODE_LIGHTING=2 --ie normals
Mesh3D.MODE_BUMP=4
Mesh3D.MODE_SHADOW=8
function Mesh3D:init()
	self.mode=0
end
function Mesh3D:updateMode(set,clear)
	local nm=(self.mode|(set or 0))&~(clear or 0)
	if nm~=self.mode then
		self.mode=nm
		local s=nil
		if (nm&Mesh3D.MODE_LIGHTING)>0 then
			local tc
			if (nm&Mesh3D.MODE_TEXTURE)>0 then
				tc="t"
			else
				tc="b"
			end
			if (nm&Mesh3D.MODE_BUMP)>0 then
				tc=tc.."n"
			end
			if (nm&Mesh3D.MODE_SHADOW)>0 then
				tc=tc.."s"
				self:setTexture(Lighting.getShadowMap(),2)
			end
			s=Lighting["normal_shader_"..tc]
		end
		self:setShader(s)
	end
end

--Unit Cube
local Box=Core.class(Mesh3D)
function Box:init()
	if not Box.va then
		Box.va={
			-1,-1,-1, 1,-1,-1, 1,1,-1, -1,1,-1,
			-1,-1,1, 1,-1,1, 1,1,1, -1,1,1,
			-1,-1,-1, 1,-1,-1, 1,-1,1, -1,-1,1,
			-1,1,-1, 1,1,-1, 1,1,1, -1,1,1,
			-1,-1,-1, -1,1,-1, -1,1,1, -1,-1,1,
			1,-1,-1, 1,1,-1, 1,1,1, 1,-1,1,
		}
		Box.ia={
			1,2,3,1,3,4,
			5,6,7,5,7,8,
			9,10,11,9,11,12,
			13,14,15,13,15,16,
			17,18,19,17,19,20,
			21,22,23,21,23,24}
		Box.na={
			0,0,-1,0,0,-1,0,0,-1,0,0,-1,
			0,0,1,0,0,1,0,0,1,0,0,1,
			0,-1,0,0,-1,0,0,-1,0,0,-1,0,
			0,1,0,0,1,0,0,1,0,0,1,0,
			-1,0,0,-1,0,0,-1,0,0,-1,0,0,
			1,0,0,1,0,0,1,0,0,1,0,0,
			}
	end
	self:setGenericArray(3,Shader.DFLOAT,3,24,Box.na)
	self:setVertexArray(Box.va)
	self:setIndexArray(Box.ia)
	self._va=Box.va self._ia=Box.ia
end
function Box:mapTexture(texture)
	self:setTexture(texture)
	if texture then
		local tw,th=texture:getWidth(),texture:getHeight()
		self:setTextureCoordinateArray{
				0,0,tw,0,tw,th,0,th,
				0,0,tw,0,tw,th,0,th,
				0,0,tw,0,tw,th,0,th,
				0,0,tw,0,tw,th,0,th,
				0,0,tw,0,tw,th,0,th,
				0,0,tw,0,tw,th,0,th,
			}
		self:updateMode(Mesh3D.MODE_TEXTURE,0)
	else
		self:updateMode(0,Mesh3D.MODE_TEXTURE)
	end
end
function Box:getCollisionShape()
	if not self._r3dshape then
		self._r3dshape=r3d.BoxShape.new(1,1,1)
	end
	return self._r3dshape
end

--Unit Sphere
local Sphere=Core.class(Mesh3D)
function Sphere:init(steps)
	local va,ia={},{}
	local rs=(2*3.141592654)/steps
	local i,ni=4,1
	--Vertices
	va[1]=0 va[2]=1 va[3]=0
	for iy=1,(steps//2)-1 do
		local y=math.cos(iy*rs)
		local r=math.sin(iy*rs)
		for ix=0,steps do
			local x=r*math.cos(ix*rs)
			local z=r*math.sin(ix*rs)
			va[i]=x i+=1
			va[i]=y i+=1
			va[i]=z i+=1
		end
	end
	va[i]=0	va[i+1]=-1 va[i+2]=0
	local lvi=i//3+1
	--Indices
	--a) top and bottom fans
	for i=1,steps do
		ia[ni]=1 ni+=1 ia[ni]=i+1 ni+=1 ia[ni]=i+2 ni+=1
		ia[ni]=lvi ni+=1 ia[ni]=lvi-i ni+=1 ia[ni]=lvi-i-1 ni+=1
	end
	--b) quads
	for iy=1,(steps//2)-2 do
		local b=1+(steps+1)*(iy-1)
		for ix=1,steps do
			ia[ni]=b+ix ni+=1 ia[ni]=b+ix+1 ni+=1 ia[ni]=b+ix+steps+1 ni+=1
			ia[ni]=b+ix+steps+1 ni+=1 ia[ni]=b+ix+1 ni+=1 ia[ni]=b+ix+steps+2 ni+=1
		end
	end
	self:setGenericArray(3,Shader.DFLOAT,3,lvi,va)
	self:setVertexArray(va)
	self:setIndexArray(ia)
	self._steps=steps
	self._va=va self._ia=ia
end
function Sphere:mapTexture(texture)
	self:setTexture(texture)
	if texture then
		local tw,th=texture:getWidth(),texture:getHeight()
		local va={}
		local i=3
		--TexCoords
		va[1]=tw/2 va[2]=0
		for iy=1,(self._steps//2)-1 do
			local y=th*(1-iy*2/self._steps)
			for ix=0,self._steps do
				local x=tw*(ix/self._steps)
				va[i]=x i+=1
				va[i]=y i+=1
			end
		end
		va[i]=tw/2	va[i+1]=th
		self:setTextureCoordinateArray(va)
		self:updateMode(Mesh3D.MODE_TEXTURE,0)
	else
		self:updateMode(0,Mesh3D.MODE_TEXTURE)
	end
end
function Sphere:getCollisionShape()
	if not self._r3dshape then
		self._r3dshape=r3d.SphereShape.new(1)
	end
	return self._r3dshape
end

--Unit Cylinder along Y axis
local Cylinder=Core.class(Mesh3D)
function Cylinder:init(steps)
	local va,ia,na={},{},{}
	local rs=(2*3.141592654)/steps
	local i,ni=7,1
	--Vertices/Normals
	va[1]=0 va[2]=1 va[3]=0
	va[4]=0	va[5]=-1 va[6]=0
	na[1]=0 na[2]=1 na[3]=0
	na[4]=0	na[5]=-1 na[6]=0
	for ix=0,steps do
		local x=math.cos(ix*rs)
		local z=-math.sin(ix*rs)
		va[i]=x na[i]=0 i+=1
		va[i]=1 na[i]=1 i+=1
		va[i]=z na[i]=0 i+=1
		va[i]=x na[i]=x i+=1
		va[i]=1 na[i]=0 i+=1
		va[i]=z na[i]=z i+=1
		va[i]=x na[i]=x i+=1
		va[i]=-1 na[i]=0 i+=1
		va[i]=z na[i]=z i+=1
		va[i]=x na[i]=0 i+=1
		va[i]=-1 na[i]=-1 i+=1
		va[i]=z na[i]=0 i+=1
	end
	--Indices
	for i=3,steps*4-1,4 do
		--For rendering, take care of normals
		ia[ni]=1 ni+=1 ia[ni]=i ni+=1 ia[ni]=i+4 ni+=1
		ia[ni]=2 ni+=1 ia[ni]=i+3 ni+=1 ia[ni]=i+7 ni+=1
		ia[ni]=i+1 ni+=1 ia[ni]=i+2 ni+=1 ia[ni]=i+5 ni+=1
		ia[ni]=i+2 ni+=1 ia[ni]=i+6 ni+=1 ia[ni]=i+5 ni+=1
	end
	
	self:setGenericArray(3,Shader.DFLOAT,3,#na//3,na)
	self:setVertexArray(va)
	self:setIndexArray(ia)
	self._steps=steps
	self._va=va self._ia=ia
end
function Cylinder:mapTexture(texture)
	self:setTexture(texture)
	if texture then
		local tw,th=texture:getWidth(),texture:getHeight()
		local va={}
		local i=5
		--TexCoords
		local twh,thh=tw/2,th/2
		va[1]=twh va[2]=0
		va[3]=twh va[4]=th
		for xi=0,self._steps do
			local x=tw*(xi/self._steps)
			va[i]=x i+=1
			va[i]=0 i+=1
			va[i]=x i+=1
			va[i]=0 i+=1
			va[i]=x i+=1
			va[i]=th i+=1
			va[i]=x i+=1
			va[i]=th i+=1
		end
		self:setTextureCoordinateArray(va)
		self:updateMode(Mesh3D.MODE_TEXTURE,0)
	else
		self:updateMode(0,Mesh3D.MODE_TEXTURE)
	end
end
function Cylinder:getCollisionShape()
	if not self._r3dshape then
		--For collision, ensure closed/CCW shape
		local steps=self._steps
		local ca,fa={},{}
		local nc,nf=1,1
		for i=3,steps*4-1,4 do ca[nc]=i+1 nc+=1 end
		fa[nf]=steps nf+=1
		for i=3,steps*4-1,4 do
			ca[nc]=i+1 nc+=1 ca[nc]=i+2 nc+=1 
			ca[nc]=i+6 nc+=1 ca[nc]=i+5 nc+=1 
			fa[nf]=4 nf+=1
		end
		ca[nc-2]=5 ca[nc-1]=4
		for i=steps*4-1,3,-4 do ca[nc]=i+2 nc+=1 end
		fa[nf]=steps nf+=1
		self._r3dshape=r3d.ConvexMeshShape.new(self._va,ca,fa)
	end
	return self._r3dshape
end

D3={
	Mesh=Mesh3D,
	Cube=Box,
	Sphere=Sphere,
	Cylinder=Cylinder
}

D3.checkCCW=function(v,i,f)
	local fi=1
	for fn=1,#f do
		local s=""
		for l=fi,fi+f[fn]-1 do
			local ii=i[l]*3-2
			local ax,ay,az=v[ii],v[ii+1],v[ii+2]		
			s=s..string.format("%d:[%f,%f,%f] ",i[l],ax,ay,az)
		end
		fi+=f[fn]
		print(fn,f[fn],s)
	end
end
