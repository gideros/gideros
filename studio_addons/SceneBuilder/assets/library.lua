local lfs=require "lfs"
AssetShelf=Core.class()

function AssetShelf.setLibraryPath(path)
	AssetShelf.libraryPath=path or application:getNativePath("|D|Library")
	local ll=#AssetShelf.libraryPath
	if AssetShelf.libraryPath:sub(ll,ll)=="/" then AssetShelf.libraryPath=AssetShelf.libraryPath:sub(1,ll-1) end
end

function AssetShelf.loadAll()
	local t={}
	local libdir=AssetShelf.libraryPath
	lfs.mkdir(libdir)
	for file in lfs.dir(libdir) do
		if file:sub(1,1)~="." then
			local filePath=application:getNativePath(AssetShelf.libraryPath.."/"..file)
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
	self.libPath=self.libraryPath.."/"..name
	lfs.mkdir(self.libraryPath)
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
	return G3DFormat.buildG3D(data,nil,nil,{
		resolvePath=function(path,type)
			return self.libPath.."/"..path
		end
	})
end
	
function AssetShelf:readFile(file)
	return Files.loadJson(self.libPath.."/"..file)
end

function AssetShelf:exportAsset(name,folder,exportContext)
	lfs.mkdir(application:getNativePath(folder.."/"..self.name))
	local data=self:readFile(name..".g3d")
	local tlist={}
	self:checkG3dTextures(data,{},tlist)
	Files.saveJson(folder.."/"..self.name.."/"..name..".g3d",data,true)
	for _,tm in ipairs(tlist) do
		local tfile=folder.."/"..self.name.."/"..tm.table[tm.key]
		local sfile=self.libPath.."/"..tm.table[tm.key]
		exportContext.tfiles=exportContext.tfiles or {}
		if not exportContext.tfiles[sfile] then
			local td=Files.load(sfile)
			lfs.mkdir(application:getNativePath(folder.."/"..self.name.."/textures"))
			Files.save(tfile,td)
			exportContext.tfiles[sfile]=true
		end
	end
end

function AssetShelf:import(path,cb)
	local texmap={}
	local imp_n,imp_m=0,0
	
	local workmap={}
	for file in lfs.dir(application:getNativePath(path)) do
		local filePath=application:getNativePath(path.."/"..file)
		local attrs=lfs.attributes(filePath)
		if attrs and attrs.mode=="file" then
			local ftype
			if filePath:sub(-4):lower()==".glb" then ftype="glb" end
			if filePath:sub(-5):lower()==".gltf" then ftype="gltf" end
			if filePath:sub(-4):lower()==".obj" then ftype="obj" end
			if filePath:sub(-4):lower()==".vox" then ftype="vox" end
			if filePath:sub(-4):lower()==".fbx" then ftype="fbx" end
			if filePath:sub(-13):lower()==".unitypackage" then ftype="unity" end
			if ftype then
				local oname=file
				local odot=oname:find(".",nil,true)
				if odot then oname=oname:sub(1,odot-1) end
				if #oname==0 then
					print("Couldn't extract a name from file ",file)
				else
					local ua
					if ftype=="unity" then
						ua=UAPack.new(filePath)
						imp_m+=#ua:getPrefabs()
					else
						imp_m+=1
					end
					table.insert(workmap,{path=path,file=file,filePath=filePath,oname=oname,ftype=ftype, unityasset=ua})
				end
			end
		end
	end

	for fn,work in ipairs(workmap) do
		local path=work.path
		local file=work.file
		local filePath=work.filePath
		local oname=work.oname
		local ftype=work.ftype
		if self.files[oname] then
			print("Object "..oname.." already present in library, replacing")
		end
		
		imp_n+=1
		cb(imp_n,imp_m,oname)
		--print(oname,filePath)
		local data,mtls
		if ftype=="glb" then
			local glb=Glb.new(path,file)
			data=glb:getScene()
		elseif ftype=="gltf" then
			local glb=Gltf.new(path,file)
			data=glb:getScene()
		elseif ftype=="obj" then
			data,mtls=importObj(path,file)
		elseif ftype=="vox" then
			local vox=MagicaVoxel.new(path.."/"..file)
			data=vox:getModel(true)
		elseif ftype=="fbx" then
			local tout=application:getNativePath("|T|temp.gdx")
			os.execute(lfs.currentdir().."\\Tools\\fbx-conv-win32.exe -f -o g3dj \""..filePath.."\" \""..tout.."\"")
			data,mtls=loadGdx("|T|temp.gdx")
			os.remove("|T|temp.gdx")
		elseif ftype=="unity" then
			imp_n-=1
			local ua=work.unityasset
			local uafiles=ua:getPrefabs()
			for pn,ff in ipairs(uafiles) do
				imp_n+=1
				
				local aname=ff.filename
				cb(imp_n,imp_m,aname)
				
				Files.save("|T|temp.fbx",ff.mesh)
				local tout=application:getNativePath("|T|temp.gdx")
				local tin=application:getNativePath("|T|temp.fbx")
				os.execute(lfs.currentdir().."\\Tools\\fbx-conv-win32.exe -f -o g3dj \""..tin.."\" \""..tout.."\"")
				data,mtls=loadGdx("|T|temp.gdx")
				os.remove("|T|temp.gdx")
				os.remove("|T|temp.fbx")

							
				local textures={}
				self:checkG3dTextures(data,mtls,textures)
				
				local function mapUnityTexture(utex,map)
					local file=utex.pathname
					if map[file] then return map[file] end
					local tfile=tostring(pn).."_"..(file:match("([^./\\]+%.[^/\\]+)$"))
					lfs.mkdir(application:getNativePath(self.libPath.."/textures"))
					Files.save(self.libPath.."/textures/"..tfile,utex.asset)				
					local mfile="textures/"..tfile
					map[file]=mfile
					return mfile
				end
						
				local mats=ff.materials
				for _,tm in ipairs(textures) do
					if mats[1] and mats[1][tm.key] then												
						tm.table[tm.key]=mapUnityTexture(mats[1][tm.key],texmap)
					else
						print("No mapping for "..tm.key.." : "..tm.table[tm.key])
						tm.table[tm.key]=nil
					end
				end
						
				G3DFormat.makeSerializable(data,mtls)
				Files.saveJson(self.libPath.."/"..aname..".g3d",data,true)
				self:generateThumbnail(aname,data)
				self.files[aname]=aname..".g3d"
						
				data=nil
				Core.yield(true)
			end
		end
		if data then
			local textures={}
			self:checkG3dTextures(data,mtls,textures)
			self:mapTextures(textures,texmap)
			G3DFormat.makeSerializable(data,mtls)
			Files.saveJson(self.libPath.."/"..oname..".g3d",data,true)
			self:generateThumbnail(oname,data)
			self.files[oname]=oname..".g3d"
		end
		Core.yield(true)
	end					
	Files.saveJson(self.libPath.."/_manifest_.json",self.files)
end

function AssetShelf:mapTextures(texs,map)
	local donet={}
	local n=0
	for _,tm in ipairs(texs) do
		local skip
		if donet[tm.key] then
			if donet[tm.key][tm.table] then skip=true end
		else
			donet[tm.key]={}
		end
		donet[tm.key][tm.table]=true
		if not skip then
			local file=tm.table[tm.key]
			local mfile=map[file]
			if not mfile then
				local lfile=application:get("openFileDialog","Choose a texture file for "..file,nil,"Texture (*.png;*.jpg)")
				if lfile and lfile~="" then 
					mfile=lfile
				else
					mfile=file
				end
				n=n+1
				local tfile=tostring(n).."_"..(mfile:match("([^./\\]+%.[^/\\]+)$"))
				lfs.mkdir(application:getNativePath(self.libPath.."/textures"))
				local itex=Files.load(mfile)
				Files.save(self.libPath.."/textures/"..tfile,itex)				
				mfile="textures/"..tfile
				print(file,mfile)
				map[file]=mfile
			end
			tm.table[tm.key]=mfile
		end
	end
end

function AssetShelf:checkG3dTextures(g3d,mtls,textures)
	if g3d.type=="group" then
		for _,v in pairs(g3d.parts) do
			self:checkG3dTextures(v,mtls,textures)
		end
	elseif g3d.type=="mesh" then
		local mat=g3d.material
		if mtls and type(mat)=="string" then 
			mat=mtls[mat]
		end
		if mat and mat.textureFile then 
			table.insert(textures,{ table=mat, key="textureFile" })
		end
		if mat and mat.normalMapFile then 
			table.insert(textures,{ table=mat, key="normalMapFile" })
		end
	elseif g3d.type=="voxel" then
		
	else
		assert(g3d.type,"No type G3D structure")
		assert(false,"Unrecognized object type: "..g3d.type)
	end
end

function AssetShelf:generateThumbnail(name,data)
	G3DFormat.computeG3DSizes(data)
	local m=G3DFormat.buildG3D(data,nil,nil,{
		resolvePath=function(path,type)
			return self.libPath.."/"..path
		end
	})

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
