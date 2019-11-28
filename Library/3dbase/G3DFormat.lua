G3DFormat={}

function G3DFormat.computeG3DSizes(g3d)
	if g3d.type=="group" then
		for _,v in pairs(g3d.parts) do
			G3DFormat.computeG3DSizes(v)
			if g3d.min then
				g3d.min={math.min(g3d.min[1],v.min[1]),math.min(g3d.min[2],v.min[2]),math.min(g3d.min[3],v.min[3])}
				g3d.max={math.max(g3d.max[1],v.max[1]),math.max(g3d.max[2],v.max[2]),math.max(g3d.max[3],v.max[3])}
			else
				g3d.min={v.min[1],v.min[2],v.min[3]}
				g3d.max={v.max[1],v.max[2],v.max[3]}
			end
		end
		if not g3d.min then
			g3d.min={0,0,0}
			g3d.max={0,0,0}
		end
	elseif g3d.type=="mesh" then
		local minx,miny,minz=100000,100000,100000
		local maxx,maxy,maxz=-100000,-100000,-100000
		for id=1,#g3d.indices do
		 local i=g3d.indices[id]*3-2
		 local x,y,z=g3d.vertices[i],g3d.vertices[i+1],g3d.vertices[i+2]
		 minx=math.min(minx,x)
		 miny=math.min(miny,y)
		 minz=math.min(minz,z)
		 maxx=math.max(maxx,x)
		 maxy=math.max(maxy,y)
		 maxz=math.max(maxz,z)
		end
		g3d.min={minx,miny,minz}
		g3d.max={maxx,maxy,maxz}
	else
		assert(g3d.type,"No type G3D structure")
		assert(false,"Unrecognized object type: "..g3d.type)
	end
	g3d.center={(g3d.max[1]+g3d.min[1])/2,(g3d.max[2]+g3d.min[2])/2,(g3d.max[3]+g3d.min[3])/2}
end

function G3DFormat.srtToMatrix(v,rev)
	local mt=Matrix.new()
	if rev and v.t then
		if v.t then
			mt:translate(v.t[1],v.t[2],v.t[3])
		end
	else
		if v.s then
			mt:scale(v.s[1],v.s[2],v.s[3])
		end
	end
	if v.r then 
		local X,Y,Z,W=v.r[1],v.r[2],v.r[3],v.r[4]
		local xx,xy,xz,xw,yy,yz,yw,zz,zw
		local m00,m01,m02,m10,m11,m12,m20,m21,m22
		xx      = X * X
		xy      = X * Y
		xz      = X * Z
		xw      = X * W

		yy      = Y * Y
		yz      = Y * Z
		yw      = Y * W

		zz      = Z * Z
		zw      = Z * W

		m00  = 1 - 2 * ( yy + zz )
		m01  =     2 * ( xy - zw )
		m02 =     2 * ( xz + yw )
		m10  =     2 * ( xy + zw )
		m11  = 1 - 2 * ( xx + zz )
		m12  =     2 * ( yz - xw )
		m20  =     2 * ( xz - yw )
		m21  =     2 * ( yz + xw )
		m22 = 1 - 2 * ( xx + yy )
		local tx,ty,tz=0
		if v.tr then
			tx=v.t[1]
			ty=v.t[2]
			tz=v.t[3]
		end
		local rm=Matrix.new()
		rm:setMatrix(m00,m10,m20,0,m01,m11,m21,0,m02,m12,m22,0,tx,ty,tz,1)
		--rm:setMatrix(m00,m01,m02,0,m10,m11,m12,0,m20,m21,m22,0,0,0,0,1)
		rm:multiply(mt) mt=rm
	end
	if rev and v.s then
		mt:scale(v.s[1],v.s[2],v.s[3])
	else
		if v.t then
			mt:translate(v.t[1],v.t[2],v.t[3])
		end
	end
	return mt
end

function G3DFormat.quaternionToEuler(w,x,y,z)
   -- roll (x-axis rotation)
   local sinr_cosp = 2 * (w * x + y * z)
   local cosr_cosp = 1 - 2 * (x * x + y * y)
   local rx = math.atan2(sinr_cosp, cosr_cosp)

   -- pitch (y-axis rotation)
    local sinp = 2 * (w * y - z * x)
	local ry
    if (math.abs(sinp) >= 1) then
        ry= 3.141592654 / 2
		if sinp<0 then ry=-ry end
    else
        ry = math.asin(sinp)
	end

    -- yaw (z-axis rotation)
    local siny_cosp = 2 * (w * z + x * y)
    local cosy_cosp = 1 - 2 * (y * y + z * z)
    local rz = math.atan2(siny_cosp, cosy_cosp)
	
	return ^>rx,^>ry,^>rz
end

function G3DFormat.buildG3DObject(obj,mtls,top)
	mtls=mtls or {}
	m=D3.Mesh.new()
	m:setVertexArray(obj.vertices)
	m:setIndexArray(obj.indices)
	mtl={}
	if obj.material then 
		mtl=mtls[obj.material]
		assert(mtl,"No such material: "..obj.material)
	end
	if obj.color then
		m:setColorTransform(obj.color[1],obj.color[2],obj.color[3],obj.color[4])
	end
	--m:setColorArray(c)
	local smode=0
	if mtl.textureFile and not mtl.texture then
		mtl.texture=Texture.new(mtl.textureFile,true)
		mtl.texturew=mtl.texture:getWidth()
		mtl.textureh=mtl.texture:getHeight()
	end
	if (mtl.texture~=nil) then
		m:setTexture(mtl.texture)
		local tc={}
		for i=1,#obj.texcoords,2 do
			tc[i]=obj.texcoords[i]*mtl.texturew
			tc[i+1]=obj.texcoords[i+1]*mtl.textureh
		end
		m:setTextureCoordinateArray(tc)
		m.hasTexture=true
		smode=smode|D3.Mesh.MODE_TEXTURE
	end
	if mtl.normalMapFile and not mtl.normalMap then
		mtl.normalMap=Texture.new(mtl.normalMapFile,true)
		mtl.normalMapW=mtl.normalMap:getWidth()
		mtl.normalMapH=mtl.normalMap:getHeight()
	end
	if (mtl.normalMap~=nil) then
		m:setTexture(mtl.normalMap,1)
		m.hasNormalMap=true
		smode=smode|D3.Mesh.MODE_BUMP
	end
	if mtl.kd then
		m:setColorTransform(mtl.kd[1],mtl.kd[2],mtl.kd[3],mtl.kd[4])
	end
	if obj.normals then
		m.hasNormals=true
		m:setGenericArray(3,Shader.DFLOAT,3,#obj.normals/3,obj.normals)
		smode=smode|D3.Mesh.MODE_LIGHTING
	end
	if obj.animdata then
		m:setGenericArray(4,Shader.DFLOAT,4,#obj.animdata.bi/4,obj.animdata.bi)
		m:setGenericArray(5,Shader.DFLOAT,4,#obj.animdata.bw/4,obj.animdata.bw)
		if top and top.bones and obj.bones then
			m.animBones={}
			for k,v in ipairs(obj.bones) do
				local bone=top.bones[v.node]
				local pose=G3DFormat.srtToMatrix(v.poseSrt)
				m.animBones[k]={ bone=bone, poseMat=pose}
				top.animMeshes=top.animMeshes or {}
				top.animMeshes[m]=true
			end
			m.bonesTop=top
		end
		smode=smode|D3.Mesh.MODE_ANIMATED
	end
	m:updateMode(smode|D3.Mesh.MODE_SHADOW,0)
	return m
end

function G3DFormat.buildG3D(g3d,mtl,top)
	local spr=nil
	if g3d.type=="group" then
		spr=Sprite.new()
		local ltop=top or spr
		spr.name=g3d.name
		spr.objs={}
		if g3d.bones then
			spr.bones={}
			for k,v in pairs(g3d.bones) do
				spr.bones[k]=v
			end
		end
		spr.animations=g3d.animations
		for k,v in pairs(g3d.parts) do
			local m=G3DFormat.buildG3D(v,mtl,ltop)
			spr:addChild(m)
			spr.objs[k]=m
			if top and top.bones then
				if top.bones[k] then 
					top.bones[k]=m 
				end
			end
		end
	elseif g3d.type=="mesh" then
		spr=G3DFormat.buildG3DObject(g3d,mtl,top)
	else
		assert(g3d.type,"No type G3D structure")
		assert(false,"Unrecognized object type: "..g3d.type)
	end
	if spr then
		spr.min=g3d.min
		spr.max=g3d.max
		spr.center=g3d.center
		if g3d.transform then
			local m=Matrix.new()
			m:setMatrix(unpack(g3d.transform))
			spr:setMatrix(m)
		elseif g3d.srt then
			spr:setMatrix(G3DFormat.srtToMatrix(g3d.srt))
		end
		spr.srt=g3d.srt
	end
	if spr.animMeshes then
		for m,_ in pairs(spr.animMeshes) do
			if m.animBones then
				for _,b in ipairs(m.animBones) do
					local pose=Matrix.new()
					pose:setMatrix(b.poseMat:getMatrix())
					local p=m
					local id=Matrix.new()
					while true do
						p:setMatrix(id)
						p=p:getParent()
						if p==spr then break end
					end
					pose:invert()
					b.poseIMat=pose
				end
			end
		end
	end

	return spr
end

function G3DFormat.mapCoords(v,t,n,faces)
	imap={}
	vmap={}
	imap.alloc=function(self,facenm,i)
		 local iv=i.v or 0
		 if (iv<0) then
			iv=(#v/3+1+iv)
		 end
		 iv=iv-1
		 local it=i.t or 0
		 if (it==nil) then																																														
		  it=-1
		 else
		  if (it<0) then
			it=(#t/2)+it+1
		  end
		  it=it-1
		 end
		 local inm=i.n or 0
		 if (inm<0) then
		  inm=(#n/3)+inm+1
		 end
		 inm=inm-1
		 if inm==-1 then inm=facenm.code end
		 local ms=iv..":"..it..":"..inm
		 if vmap[ms]==nil then
			local ni=self.ni+1
			self.ni=ni
			table.insert(facenm.lvi,#self.lv)
			assert(v[iv*3+1],"Missing Vertex:"..iv)
			table.insert(self.lv,v[iv*3+1])
			table.insert(self.lv,v[iv*3+2])
			table.insert(self.lv,v[iv*3+3])
			if it>=0 then
				table.insert(self.lvt,t[it*2+1])
				table.insert(self.lvt,(1-t[it*2+2]))
			else 
				table.insert(self.lvt,0)
				table.insert(self.lvt,0)
			end
			if inm>=0 then
				table.insert(self.lvn,n[inm*3+1])
				table.insert(self.lvn,n[inm*3+2])
				table.insert(self.lvn,n[inm*3+3])
			else 
				local vngmap=self.vngmap[iv] or { }
				self.vngmap[iv]=vngmap
				table.insert(vngmap,#self.lvn)
				table.insert(facenm.lvni,#self.lvn)
				table.insert(self.lvn,0)
				table.insert(self.lvn,0)
				table.insert(self.lvn,0)
			end
			self.vmap[ms]=ni
		 end
		 return self.vmap[ms]
	end		
	imap.i={}
	imap.ni=0
	imap.lv={}
	imap.lvt={}
	imap.lvn={}
	imap.vmap={}
	imap.vngmap={}
	imap.gnorm=-2
	
	for _,face in ipairs(faces) do
	local itab={}
	local normtab={ code=imap.gnorm, lvi={}, lvni={} }
	for _,i in ipairs(face) do
		table.insert(itab,imap:alloc(normtab,i))
	end
	imap.gnorm=imap.gnorm-1
	if (#itab>=3) then
		if #normtab.lvni>0 then -- Gen normals
			local ux=imap.lv[normtab.lvi[2]+1]-imap.lv[normtab.lvi[1]+1]
			local uy=imap.lv[normtab.lvi[2]+2]-imap.lv[normtab.lvi[1]+2]
			local uz=imap.lv[normtab.lvi[2]+3]-imap.lv[normtab.lvi[1]+3]
			local vx=imap.lv[normtab.lvi[3]+1]-imap.lv[normtab.lvi[1]+1]
			local vy=imap.lv[normtab.lvi[3]+2]-imap.lv[normtab.lvi[1]+2]
			local vz=imap.lv[normtab.lvi[3]+3]-imap.lv[normtab.lvi[1]+3]
			local nx=uy*vz-uz*vy
			local ny=uz*vx-ux*vz
			local nz=ux*vy-uy*vx
			local nl=math.sqrt(nx*nx+ny*ny+nz*nz)
			if nl==0 then nl=1 end
			nx=nx/nl
			ny=ny/nl
			nz=nz/nl
			for _,vni in ipairs(normtab.lvni) do
				imap.lvn[vni+1]=nx
				imap.lvn[vni+2]=ny
				imap.lvn[vni+3]=nz
			end
		end
		for ii=3,#itab,1 do
			table.insert(imap.i,itab[1])
			table.insert(imap.i,itab[ii-1])
			table.insert(imap.i,itab[ii])
		end
	end
	end
		for _,vm in pairs(imap.vngmap) do
			local nx,ny,nz=0,0,0
			for _,vn in ipairs(vm) do
				nx=nx+imap.lvn[vn+1]
				ny=ny+imap.lvn[vn+2]
				nz=nz+imap.lvn[vn+3]
			end
			local nl=math.sqrt(nx*nx+ny*ny+nz*nz)
			if nl==0 then nl=1 end
			nx=nx/nl
			ny=ny/nl
			nz=nz/nl
			for _,vn in ipairs(vm) do
				imap.lvn[vn+1]=nx
				imap.lvn[vn+2]=ny
				imap.lvn[vn+3]=nz
			end	
		end
	return { vertices=imap.lv, texcoords=imap.lvt, normals=imap.lvn, indices=imap.i }
end
