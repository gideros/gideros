MagicaVoxel=Core.class()

function MagicaVoxel:init(path)
	local f=io.open(path)
	local b=f:read("*a")
	f:close()
	
	local hdr=b:decodeValue("ii")
	assert(hdr[1]==0x20584F56,"Not a vox file"..path)
	assert(hdr[2]==150,"Version 150 expected")
	
	self.data=self:parseChunk(b:sub(9))
end

function MagicaVoxel:getModel(optimize)
	local root={
		type="group",
		parts={},
		name="magica",
	}
	local mdl,colorMap
	for _,cc in ipairs(self.data.sub) do
		if cc.id=="SIZE" then
			local sizes=cc.data:decodeValue("iii")
			mdl={ type="voxel", max=sizes }
		elseif mdl and cc.id=="XYZI" then
			local vc=cc.data:decodeValue("i")
			mdl.voxelCount=vc
			local vtab=cc.data:sub(5)
			local vx={}
			for v=1,#vtab do
				vx[v]=vtab:byte(v,v)
			end
			for v=1,#vx,4 do
				local s=vx[v+1]
				vx[v+1]=vx[v+2]
				vx[v+2]=s
			end
			mdl.voxels=vx
			table.insert(root.parts,mdl)
			mdl=nil
		elseif cc.id=="RGBA" then
			colorMap=cc.data			
		end
	end	
	if colorMap then
		for _,m in ipairs(root.parts) do
			m.colorMap=colorMap
		end
	end
	if optimize then self:optimize(root) end
	return root
end

function MagicaVoxel:optimize(root)
	local dirs={
		vector(0,0,-1,0), vector(0,0,1,0),
		vector(0,-1,0,0), vector(0,1,0,0),
		vector(-1,0,0,0), vector(1,0,0,0),
	}
	for _,m in ipairs(root.parts) do
		local vmap={}
		local vx=m.voxels
		for v=1,#vx,4 do
			local vpos=vector(vx[v],vx[v+1],vx[v+2],0)
			vmap[vpos]=true
		end
		local opts=0
		for v=1,#vx,4 do
			local vpos=vector(vx[v],vx[v+1],vx[v+2],0)
			local disc=0
			for d=1,6 do
				if vmap[vpos+dirs[d]] then disc=disc|(1<<(d-1)) opts+=1 end
			end
			vx[v+3]=vx[v+3]|(disc<<8)
		end
		print("Removed "..opts.." faces out of "..(6*#vx/4))
	end
end
	
function MagicaVoxel:parseChunk(b)
	local cid=b:sub(1,4)
	if #cid<4 then return end
	local hdr=b:sub(5,5+7):decodeValue("ii")
	local cc=b:sub(13,13+hdr[1]-1)
	local csub=b:sub(13+hdr[1],13+hdr[1]+hdr[2]-1)
	local chunk={ id=cid, data=cc }
	while true do
		local c,cl=self:parseChunk(csub)
		if not c then break end
		chunk.sub=chunk.sub or {}
		table.insert(chunk.sub,c)
		csub=csub:sub(cl+1)
	end
	return chunk,12+hdr[1]+hdr[2]
end
