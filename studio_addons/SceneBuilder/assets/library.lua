local lfs=require "lfs"
AssetShelf=Core.class()

function AssetShelf.loadAll()
	local t={}
	local libdir=application:getNativePath("|D|Library")
	lfs.mkdir(libdir)
	for file in lfs.dir(libdir) do
		if file:sub(1,1)~="." then
			local filePath=application:getNativePath("|D|Library".."/"..file)
			local attrs=lfs.attributes(filePath)
			if attrs and attrs.mode=="directory" then
				t[#t+1]=AssetShelf.new(file)
			end
		end
	end
	table.sort(t,function(a,b) return a.name<b.name end)
	return t
end

function AssetShelf:init(name)
	self.name=name
	self.libPath="|D|Library/"..name
	lfs.mkdir(application:getNativePath("|D|Library"))
	lfs.mkdir(application:getNativePath(self.libPath))
	self.files=self:readFile("_manifest_.json") or {}	
end

function AssetShelf:delete()
	local function deletedir(dir)
		for file in lfs.dir(dir) do
			local file_path = dir..'/'..file
			if file ~= "." and file ~= ".." then
				if lfs.attributes(file_path, 'mode') == 'file' then
					os.remove(file_path)
				elseif lfs.attributes(file_path, 'mode') == 'directory' then
					deletedir(file_path)
				end
			end
		end
		lfs.rmdir(dir)
	end
	deletedir(application:getNativePath(self.libPath))
end

function AssetShelf:getNames()
	local n={}
	for k,v in pairs(self.files) do n[#n+1]=k end
	table.sort(n)
	return n
end

function AssetShelf:getThumbnail(name)
	return Texture.new(self.libPath.."/"..name..".png",true,{ mipmap=true })
end

function AssetShelf:getModel(name)
	local data=self:readFile(name..".g3d")
	G3DFormat.computeG3DSizes(data)
	return G3DFormat.buildG3D(data)
end
	
function AssetShelf:readFile(file)
	return Files.loadJson(self.libPath.."/"..file)
end

function AssetShelf:exportAsset(name,folder)
	lfs.mkdir(application:getNativePath(folder.."/"..self.name))
	local data=self:readFile(name..".g3d")
	Files.saveJson(folder.."/"..self.name.."/"..name..".g3d",data)
end

function AssetShelf:import(path)
	for file in lfs.dir(application:getNativePath(path)) do
		local filePath=application:getNativePath(path.."/"..file)
		local attrs=lfs.attributes(filePath)
		if attrs and attrs.mode=="file" then
			local ftype
			if filePath:sub(-4):lower()==".glb" then ftype="glb" end
			if filePath:sub(-4):lower()==".obj" then ftype="obj" end
			if filePath:sub(-4):lower()==".vox" then ftype="vox" end
			if ftype then
				local oname=file
				local odot=oname:find(".",nil,true)
				if odot then oname=oname:sub(1,odot-1) end
				if #oname==0 then
					print("Couldn't extract a name from file ",file)
				else
					if self.files[oname] then
						print("Object "..oname.." already present in library, replacing")
					end
					--print(oname,filePath)
					local data
					if ftype=="glb" then
						local glb=Glb.new(path,file)
						data=glb:getScene()
					elseif ftype=="obj" then
						data=importObj(path,file)
					elseif ftype=="vox" then
						local vox=MagicaVoxel.new(path.."/"..file)
						data=vox:getModel(true)
					end
					G3DFormat.makeSerializable(data)
					--print(require("inspect").inspect(data))
					Files.saveJson(self.libPath.."/"..oname..".g3d",data)
					self:generateThumbnail(oname,data)
					self.files[oname]=oname..".g3d"
				end					
			end
		end
	end
	Files.saveJson(self.libPath.."/_manifest_.json",self.files)
end

function AssetShelf:generateThumbnail(name,data)
	G3DFormat.computeG3DSizes(data)
	local m=G3DFormat.buildG3D(data)

	-- Adjust scale
	m:setAnchorPosition(m.center[1],m.center[2],m.center[3])
	local span=(m.max[1]-m.min[1])<>(m.max[2]-m.min[2])<>(m.max[3]-m.min[3])
	m:setScale(1/span,1/span,1/span)
	--
	local v=D3View.new(128,128)
	v:getScene():addChild(m)
	v:lookAt(1,1,1,0,0,0,0,1,0)
	Lighting.setLight(1,1,1,.3)

	D3Anim.updateBones()
	local rt=self.thumbRT or RenderTarget.new(128,128)
	self.thumbRT=rt
	rt:clear(0,0)
	rt:draw(v)
	rt:save(self.libPath.."/"..name..".png")
end
