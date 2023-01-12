--inspect=require "inspect"

local debug = nil
if debug then print("fbxloader.lua debug !!!!!!!!!!!!!!!!!!!!!!!!!") end

local function getULong(sec,off)
	local v1,v2,v3,v4=sec:byte(off,off+3)
	return v1+v2*256+(v3+v4*256)*65536
end

local function getLong(sec,off)
	local us=getULong(sec,off)
	if us>=32768*65536 then us=us-65536*65536 end
	return us
end

local function getUShort(sec,off)
	local v1,v2=sec:byte(off,off+1)
	return v1+v2*256
end

local function getShort(sec,off)
	local us=getUShort(sec,off)
	if us>=32768 then us=us-65536 end
	return us
end

local function getULongLong(sec,off)
	local v1,v2=getULong(sec,off),getULong(sec,off+4)
	return v1+v2*65536*65536
end

local function getLongLong(sec,off)
	local us=getULongLong(sec,off)
	if us>=32768*65536 then us=us-65536*65536 end
	return us
end

local function getFloat(sec,off)
  local sign = 1
  local mantissa = sec:byte(off+2) % 128

  for i = 1, 0, -1 do mantissa = mantissa * 256 + string.byte(sec, off+i) end

  if string.byte(sec, off+3) > 127 then sign = -1 end
  local exponent = (string.byte(sec, off+3) % 128) * 2 +
                   math.floor(string.byte(sec, off+2) / 128)
  if exponent == 0 then return 0 end
  mantissa = (math.ldexp(mantissa, -23) + 1) * sign
  return math.ldexp(mantissa, exponent - 127)
end

local function getDouble(sec,off)
   local sign = 1
   local mantissa = sec:byte(off+6) % 2^4
   for i = 5, 0, -1 do
     mantissa = mantissa * 256 + sec:byte(off+i)
   end
   if sec:byte(off+7) > 127 then sign = -1 end
   local exponent = (sec:byte(off+7) % 128) * 2^4 + math.floor(sec:byte(off+6) / 2^4)

   if exponent == 0 then
     return 0
   end
   mantissa = (math.ldexp(mantissa, -52) + 1) * sign
   return math.ldexp(mantissa, exponent - 1023)
end

local function loadFbxProperty(fd)
	local t=fd:read(1)
	if t=="Y" then
		local ct=fd:read(2)
		return getShort(ct,1)
	elseif t=="C" then
		local ct=fd:read(1)
		return (ct:byte(1)%2)==1
	elseif t=="I" then
		local ct=fd:read(4)
		return getLong(ct,1)
	elseif t=="F" then
		local ct=fd:read(4)
		return getFloat(ct,1)
	elseif t=="D" then
		local ct=fd:read(8)
		return getDouble(ct,1)
	elseif t=="L" then
		local ct=fd:read(8)
		return getLongLong(ct,1)
	elseif t=="S" or t=="R" then
		local ct=fd:read(4)
		local s=fd:read(getULong(ct,1))
		if t=="S" then 
			for i=1,#s do
				if s:byte(i)==0 and s:byte(i+1)==1 then
					s=s:sub(1,i-1).."::"..s:sub(i+2)
				end
			end
		end
		return s
	elseif t=="f" or t=="d" or t=="l" or t=="i" or t=="b" then
		local ct=fd:read(12)
		local al,en,el=getULong(ct,1),getULong(ct,5),getULong(ct,9)
		local esz=8
		if t=="f" or t=="i" then esz=4
		elseif t=="b" then esz=1
		end
		if en==1 then
			ct=fd:read(el)
			ct=zlib.decompress(ct)
		else
			ct=fd:read(esz*al)
		end
		local arr={}
		if t=="f" then
			for i=1,al*esz,esz do table.insert(arr,getFloat(ct,i)) end
		elseif t=="d" then
			for i=1,al*esz,esz do table.insert(arr,getDouble(ct,i)) end
		elseif t=="l" then
			for i=1,al*esz,esz do table.insert(arr,getLongLong(ct,i)) end
		elseif t=="i" then
			for i=1,al*esz,esz do table.insert(arr,getLong(ct,i)) end
		elseif t=="b" then
			for i=1,al*esz,esz do table.insert(arr,(ct:byte(i)%2)==1) end
		end
		return arr
	else
		print("Unhandled property type:"..t)
		assert(false)
		return nil
	end
end

local function loadFbxRecord(fd)
	local hdr=fd:read(13)
	local eof=getULong(hdr,1)
	local numProp=getULong(hdr,5)
	local propListLen=getULong(hdr,9)
	local nameLen=hdr:byte(13)
	if nameLen==0  then --NULL record
		return nil
	end
	local node={}
	local name=fd:read(nameLen)
	--print (eof,numProp,propListLen,nameLen,name)
	node._name=name
	node._props={}
	-- Read Properties
	for i=1,numProp do
		local val=loadFbxProperty(fd)
		table.insert(node._props,val)
	end
	while fd:seek()<eof do
		local subNode=loadFbxRecord(fd)
		if subNode and subNode._name~="" then
			if node[subNode._name]==nil then
				node[subNode._name]={}
			end
			table.insert(node[subNode._name],subNode)
		end
	end
	return node
end

local function applyLayerElement(m,l,obj,matTable)
	if debug then print(l.name) end
	local lfield,lsz=nil,0
	if l._name=="LayerElementUV" then
		lfield="UV"
		lsz=2
	elseif l._name=="LayerElementNormal" then
		lfield="Normals"
		lsz=3
	elseif l._name=="LayerElementMaterial" then
		--print("MAT",inspect.inspect(l))
		local matid=l.Materials[1]._props[1][1]
		local mat=matTable[matid+1]
		print ("MATDEF",inspect.inspect(mat))
		m.material=mat._props[2]
		
--[[		if mat._texturelayers and mat._texturelayers._texture then
			m.texture=Texture.new("Assets/Collections/Peluches/toysheep_Default_color.jpg",true) --TODO
			m.texturew=m.texture:getWidth()
			m.textureh=m.texture:getHeight()
			m:setTexture(m.texture)
		end ]]
		return
	else
		if debug then print(inspect.inspect(l)) end
		return
	end
	local itype=l.MappingInformationType[1]._props[1]
	local rtype=l.ReferenceInformationType[1]._props[1]
	
	local arr,iarr=nil,{}
	if debug then print(rtype,itype) end
	arr=l[lfield][1]._props[1]
	if rtype=="Direct" then
		for ia=0,#arr/lsz-1 do table.insert(iarr,ia) end
	elseif rtype=="IndexToDirect" then
		iarr=l[lfield.."Index"][1]._props[1]
		if debug then print(#arr,#iarr) end
	end
	if itype=="ByPolygonVertex" then
		local ia=1
		for _,f in ipairs(m.imap) do
			for _,pi in ipairs(f) do
				if lfield=="Normals" then
					pi.n=iarr[ia]+1
				elseif lfield=="UV" then
					pi.t=iarr[ia]+1
				end
				ia=ia+1
			end
		end
	else
		print("Unhandled mapping type:",itype)
		assert(false)
	end
	
	if lfield=="UV" then
			m.vt=arr
	elseif lfield=="Normals" then
			m.vn=arr
	end		
end

local function buildModel(fbx,objTable,objSubs,objnum)
	local matTable={}
	local m={ v={}, vt={}, vn={}, imap={}}
	
	local root={}
	root.type="group"
	root.parts={}
	
	if objSubs[objnum]==nil then
		return nil
	end
	
	for _,o in ipairs(objSubs[objnum]) do
		local obj=objTable[o]
		print ("SUB-OBJ",o,obj._name)
		if obj._name=="Model" then
			local model=obj
			local mname,mclass=obj._props[2],obj._props[3]
			if debug then print("MODEL",mclass,mname,o) end
			root.parts[o]=buildModel(fbx,objTable,objSubs,o)
			if model.Properties70 then
				if model.Properties70[1].P then
					local m=Matrix.new()
					local pm=Matrix.new()
					local gm=Matrix.new()
					for _,p in ipairs(model.Properties70[1].P) do
						local pname=p._props[1]
						--print(inspect.inspect(p._props))
						if pname=="Lcl Translation" then
							local tx,ty,tz=p._props[5],p._props[6],p._props[7]
							m:translate(tx,ty,tz)
						end
						if pname=="GeometricTranslation" then
							local tx,ty,tz=p._props[5],p._props[6],p._props[7]
							gm:translate(tx,ty,tz)
						end
						if pname=="PreRotation" then
							local rx,ry,rz=p._props[5],p._props[6],p._props[7]
							pm:rotate(rx,1,0,0)
							pm:rotate(ry,0,1,0)
							pm:rotate(rz,0,0,1)
						end
					end
					m:multiply(pm)
					m:multiply(gm)
					root.parts[o].transform={ m:getMatrix() }
				end
			end
		elseif obj._name=="Material" then
			table.insert(matTable,obj)
			if objSubs[obj._props[1]] then
			for _,o1 in ipairs(objSubs[obj._props[1]]) do
				local obj1=objTable[o1]
				if debug then print(obj1._name) end
				if obj1._name=="LayeredTexture" then
					obj._texturelayers=obj1
					for _,o2 in ipairs(objSubs[obj1._props[1]]) do
						local obj2=objTable[o2]
						if obj2._name=="Texture" then
							obj1._texture=obj2
						end
					end
				end
			end end
		end
	end
	if debug then print(inspect.inspect(matTable)) end
	for _,o in ipairs(objSubs[objnum]) do
		local obj=objTable[o]
		if obj._name=="Geometry" then
			--print(inspect.inspect(obj))
			local mname,mclass=obj._props[2],obj._props[3]
			if debug then print("GEOM",mclass,mname) end
			m.v=obj.Vertices[1]._props[1]
			if debug then print("VERTS",(#obj.Vertices[1]._props[1])/3) end
			local polys=obj.PolygonVertexIndex[1]._props[1]
			local simap={}
			for _,idx in ipairs(polys) do
				if idx>=0 then
					table.insert(simap,{v=idx+1})
				else
					table.insert(simap,{v=-idx})
					table.insert(m.imap,simap)
					simap={}
				end
			end
			for _,layer in ipairs(obj.Layer) do
				for _,layerElm in ipairs(layer.LayerElement) do
					local leType=layerElm.Type[1]._props[1]
					local leIndex=layerElm.TypedIndex[1]._props[1]
					local layerSet=obj[leType]
					local layer=nil
					for _,l in ipairs(layerSet) do if l._props[1]==leIndex then layer=l end end
					if debug then print(leType,leIndex,layer) end
					applyLayerElement(m,layer,obj,matTable)
				end
			end
--[[			if obj.Properties70 then
				if obj.Properties70[1].P then
					for _,p in ipairs(obj.Properties70[1].P) do
						local pname=p._props[1]
						if debug then print(inspect.inspect(p._props)) end
						if pname=="Color" then
							local r,g,b=p._props[5],p._props[6],p._props[7]
							m.color={r,g,b,1}
						end
					end
				end
			end ]]
		end
	end
--	print("M",inspect.inspect(m))
		local treeDesc=G3DFormat.mapCoords(m.v,m.vt,m.vn,m.imap)
	--print("M",inspect.inspect(treeDesc))
		treeDesc.type="mesh"
		treeDesc.material=mtl
		treeDesc.color=m.color
		treeDesc.material=m.material
		root.parts["Mesh"]=treeDesc
		if debug then print(oname,objnum) end
		return root
end

function importFbx(path,file,imtls)
	mtls=imtls or {}
	local fd=io.open(path.."/"..file,"rb")
	local header=fd:read(27)
	local fbx={}
	repeat
		local node=loadFbxRecord(fd)
		if node then
			if debug then print("ROOT",node._name) end
			fbx[node._name]=node
		end
	until node==nil

--[[
	local settingsFile = io.open("|D|FBXDump.txt", "w" )
	settingsFile:write(inspect.inspect(fbx))
	io.close(settingsFile)	
]]	
	--local model=fbx.Objects.Model
	--print("MODEL NAME:",model._props[2]," ID:",model._props[1])
	local objTable={}
	for k,v in pairs(fbx.Objects) do
		if k:sub(1,1)~="_" then
			for _,vi in ipairs(v) do
				objTable[vi._props[1]]=vi
			end
		end
	end
	
	local objSubs={}
	local cn=fbx.Connections.C or fbx.Connections.Connect
	if cn then 
	for _,v in ipairs(cn) do
		local oc,op=v._props[2],v._props[3]
		if not objSubs[op] then objSubs[op]={} end
		table.insert(objSubs[op],oc)
	end
	end
	
	if debug then print("TABLE:",inspect.inspect(objTable)) end
	if debug then print("CONN:",inspect.inspect(objSubs)) end
	
	local root=buildModel(fbx,objTable,objSubs,0)
	
	G3DFormat.computeG3DSizes(root)
	return root,mtls
end

function loadFbx(path,file,imtls)
 local root,mtls=importFbx(path,file,imtls)
 return G3DFormat.buildG3D(root,mtls)
end
