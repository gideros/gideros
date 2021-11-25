Gltf=Core.class()

function Gltf:init(path,name)
	self.path=path
	if name then
		local f=io.open(path.."/"..name)
		self.desc=json.decode(f:read("*a"))
		f:close()
	end
end

function Gltf:getScene(i)
	i=i or self.desc.scene+1
	local ns=self.desc.scenes[i]
	local root={}
	root.type="group"
	root.parts={}
	for ni,n in ipairs(ns.nodes) do
		root.parts["n"..ni]=self:getNode(n+1)
	end
	return root
end

function Gltf:getNode(i)
	local nd=self.desc.nodes[i]
	local root={}
	root.type="group"
	root.parts={}
	root.name=nd.name
	if nd.mesh then
		local md=self.desc.meshes[nd.mesh+1]
		for pi,prim in ipairs(md.primitives) do
			local function bufferIndex(str)
				for n,id in pairs(prim.attributes) do
					if n:sub(1,#str)==str then return id+1 end
				end
				return 0
			end
			local m={ 
					vertices=self:getBuffer(bufferIndex("POSITION")), 
					texcoords=self:getBuffer(bufferIndex("TEXCOORD")), 
					normals=self:getBuffer(bufferIndex("NORMAL")), 
					indices=self:getBuffer(prim.indices+1,true), 
					type="mesh",
					material=self:getMaterial((prim.material or -1)+1),
					}
			root.parts["p"..pi]=m
		end
	end
	if nd.children then
		for ni,n in ipairs(nd.children) do
			root.parts["n"..ni]=self:getNode(n+1)
		end
	end
	root.srt={ s=nd.scale, r=nd.rotation, t=nd.translation }
	return root
end

function Gltf:getBuffer(i,indices)
	local bd=self.desc.accessors[i]
	if bd==nil then return nil end
	if bd._array then return bd._array end
	local t={}
	local buf,stride=self:getBufferView(bd.bufferView+1)
	local bc=bd.count
	local bm=1
	if bd.type=="SCALAR" then
	elseif bd.type=="VEC2" then bm=2
	elseif bd.type=="VEC3" then bm=3
	elseif bd.type=="VEC4" then bm=4
	else
		assert(false,"Unhandled type:"..bd.type)
	end
	local tm=os:clock()
	
	--print("ACC:",i,buf:size(),bd.count,bc,bd.componentType)
	--[[
	 GL_BYTE (5120)
GL_DOUBLE (5130)
GL_FALSE (0)
GL_FLOAT (5126)
GL_HALF_NV (5121)
GL_INT (5124)
GL_SHORT (5122)
GL_TRUE (1)
GL_UNSIGNED_BYTE (5121)
GL_UNSIGNED_INT (5125)
GL_UNSIGNED_INT64_AMD (35778)
GL_UNSIGNED_SHORT (5123)
]]
	local cl=0
	if bd.componentType==5126 then
		cl=4
	elseif bd.componentType==5123 then
		cl=2
	elseif bd.componentType==5125 then
		cl=4
	else
		assert(false,"Unhandled componentType:"..bd.componentType)
	end
	if stride>0 then stride=stride-cl*bm end
	local br=bd.byteOffset or 0
	--print(br,bm,bc,stride)
	local ii=1
	for ci=1,bc do
		for mi=1,bm do
			if bd.componentType==5126 then
				t[ii]=buf:get(br,4):decodeValue("f")
				br+=4
			elseif bd.componentType==5123 then
				t[ii]=buf:get(br,2):decodeValue("s")
				if indices then t[ii]+=1 end
				br+=2
			elseif bd.componentType==5125 then
				t[ii]=buf:get(br,4):decodeValue("i")
				if indices then t[ii]+=1 end
				br+=4
			else
				assert(false,"Unhandled componentType:"..bd.componentType)
			end
			ii+=1
		end
		br+=stride
	end
	
	--[[if (os:clock()-tm)>.1 then
		print(i,json.encode(bd)," in ",os:clock()-tm)
	end]]
	bd._array=t
	return t
end

local gltfNum=0
function Gltf:getBufferView(n,ext)
	local bd=self.desc.bufferViews[n]
	local buf=self.desc.buffers[bd.buffer+1]
	if not buf.data then
		if buf.uri then
			local d=buf.uri:sub(1,37)
			if d=="data:application/octet-stream;base64," then
				buf.data=Cryptography.unb64(buf.uri:sub(38))
			end
		end
		if not buf.data then
			buf.data=self:loadBuffer(bd.buffer+1,buf)
		end
	end
	gltfNum+=1
	local bname="_gltf_"..gltfNum..(ext or "")
	local bb=Buffer.new(bname)
	bb:set(buf.data:sub((bd.byteOffset or 0)+1,(bd.byteOffset or 0)+bd.byteLength))
	return bb,bd.byteStride or 0,bname
end

function Gltf:getImage(n)
	local bd=self.desc.images[n]
	if not bd then return nil end
	if bd.uri then
		return self.path.."/"..bd.uri
	end
	if bd.bufferView then
		local iext=nil
		if bd.mimeType=="image/jpeg" then iext=".jpg"
		elseif bd.mimeType=="image/png" then iext=".png"
		end
		assert(iext,"Unsupported image type:"..bd.mimeType)
		local _,_,bname=self:getBufferView(bd.bufferView+1,iext)
		return "|B|"..bname
	end
end

function Gltf:loadBuffer(i,buf)
	local f=io.open(self.path.."/"..buf.uri)
	local data=f:read("*a")
	f:close()
	return data
end

function Gltf:getMaterial(i)
	local bd=self.desc.materials[i]
	if bd==nil then return nil end
	if bd._mat then return bd._mat end
	local mat={}
	if bd.pbrMetallicRoughness then
		mat.kd=bd.pbrMetallicRoughness.baseColorFactor
		if mat.kd then
			for i=1,4 do mat.kd[i]=mat.kd[i]^.3 end		
		end
		local td=bd.pbrMetallicRoughness.baseColorTexture
		if td and td.index then
			mat.textureFile=self:getImage(td.index+1)
		end
	end
	bd._mat=mat
	return mat
end

Glb=Core.class(Gltf,function (path,name) return path,nil end)

function Glb:init(path,name)
	local fn=name
	if path then
		fn=path.."/"..name
	end
	local f=io.open(fn)
	assert(f,"File not found:"..fn)
	self.binData=f:read("*a")
	f:close()
	
	local hdr=self.binData:decodeValue("iii")
	assert(hdr[1]==0x46546c67,"Not a glb file"..name)
	local length=hdr[3]-12
	local l=13
	
	local chunks={}
	while length>=8 do
		local chdr=self.binData:sub(l,l+7):decodeValue("ii")
		local cl,ct=chdr[1],chdr[2]
		--print("CHUNK",("%08x:%08x"):format(cl,ct))
		table.insert(chunks,{type=chdr[2],length=chdr[1],start=l+8})
		l+=8+cl
		length-=(8+cl)
	end
	assert(chunks[1].type==0x4E4F534A,"GLB: first buffer should be JSON")
	self.binChunks=chunks
	self.desc=json.decode(self.binData:sub(chunks[1].start,chunks[1].start+chunks[1].length-1))
	--print(json.encode(self.desc))
end

function Glb:loadBuffer(i,buf)
	return self.binData:sub(self.binChunks[i+1].start,self.binChunks[i+1].start+self.binChunks[i+1].length-1)
end
