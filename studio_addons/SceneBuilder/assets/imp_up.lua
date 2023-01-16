UAPack=Core.class()

function UAPack:init(file)
	local res=Files.load(file)
	assert(res and res:byte(1) and res:byte(2) and (res:byte(1)==31) and (res:byte(2)==139),"Not a unity package")
	local ok,error = pcall(function () res = zlib.decompress(res,15+16) end)
	assert(ok,"Not a unity package")
	
	local folders={}
	local o,size=0,#res
	while (o+511)<size do
		local fname=res:sub(o+1,res:find("\0",o+1,true)-1)
		local fsize=tonumber(res:sub(o+125,res:find("\0",o+125,true)-1),8)
		local ftype=res:byte(o+257) 
		if (not fname) or (fname=="") or (not fsize) then break end
		local fp,ff=fname:match("([^/]+)/(.*)")
		local fdata=res:sub(o+513,o+512+fsize)
		--print(fname,ftype,fsize,fp,ff)
		folders[fp]=folders[fp] or {}
		if #ff>0 then
			if ff=="asset" then
				--Figure out format
				if fdata:sub(1,20)=="Kaydara FBX Binary  " then
					folders[fp].type="fbx"
				end				
			elseif ff=="asset.meta" then
				folders[fp].meta=self:parseMeta(fdata)
			elseif ff=="pathname" then
				if not folders[fp].type then
					folders[fp].type=fdata:match("[^./\\]+%.([^/\\]+)$")
					if folders[fp].type then folders[fp].type=folders[fp].type:lower() end
				end			
			end
			--print(ff,#fdata)
			folders[fp][ff]=fdata
		end
		
		o+=(1+((fsize+511)//512))*512		
	end
	
	self.assets=folders
	
	-- Collect prefabs
	self.prefabs={}
	
	local function fileref(r)
		local guid=r:match("guid: (%w+)")
		return guid and folders[guid]
	end
	for _,ff in pairs(folders) do
		if ff.type=="prefab" then
			local yaml=self:parseYaml(ff.asset)
			local rmesh=fileref(yaml.MeshFilter.m_Mesh)
			local mats={}
			for n,r in ipairs(yaml.MeshRenderer.m_Materials) do
				local rmat=fileref(r)
				if rmat then
					local myaml=self:parseYaml(rmat.asset)
					--print(json.encode(myaml))
					local mtex,btex
					pcall(function() mtex=fileref(myaml.Material.m_SavedProperties.m_TexEnvs._MainTex.m_Texture) end)
					pcall(function() btex=fileref(myaml.Material.m_SavedProperties.m_TexEnvs._BumpMap.m_Texture) end)
					--print("PTEX:",mtex and mtex.pathname, btex and btex.pathname)
					mats[n]={textureFile=mtex, normalMapFile=btex}
				end
			end
			if rmesh and rmesh.type=="fbx" then
				table.insert(self.prefabs,
					{
						mesh=rmesh.asset,
						filename=ff.pathname:match("([^./\\]+)%.[^/\\]+$"),
						materials=mats
					})
			end	
		end
	end
end

function UAPack:getFiles()
	return self.assets
end

function UAPack:getPrefabs()
	return self.prefabs
end

function UAPack:parseMeta(data)
	local f={}
	local t,tn=f,{}
	while data do
		local ld=data:find("\n")
		local line
		if ld then
			line=data:sub(1,ld-1)
			data=data:sub(ld+1)
		else
			line=data
			data=nil
		end
		--print(line)
		local indent,key,val=line:match("(%s*)(%w+):%s*([^%s]*.*)")
		if not key or #key==0 then break end
		local ilev=#(indent or "")
		for ii=ilev+1,ilev+40 do tn[ii]=nil end
		--print(ilev,key,"//",val)
		t=tn[ilev] or t
		tn[ilev]=t
		if val and #val>0 then
			t[key]=val
		else
			t[key]={} t=t[key]			
		end
	end
	return f
end

function UAPack:parseYaml(data)
	local f={}
	local t,tn=f,{}
	while data do
		local ld=data:find("\n")
		local line
		if ld then
			line=data:sub(1,ld-1)
			data=data:sub(ld+1)
		else
			line=data
			data=nil
		end
		--print(line)
		if line:sub(1,1)=="%" then --Comment
		elseif line:sub(1,3)=="---" then --Section
		else
			local indent,key,sep,val=line:match("([%s-]*)([^%s:]*)(:)%s*([^%s]*.*)")
			if not key or #key==0 then break end
			local ilev=#(indent or "")
			for ii=ilev+1,ilev+40 do tn[ii]=nil end
			--print(ilev,key,"//",val)
			t=tn[ilev] or t
			tn[ilev]=t
			if indent:find("-") then
				if #(val or "")>0 then
					local kk=key..(sep or "")..(val or "")
					table.insert(t,kk)
				else
					t[key]={} t=t[key]			
				end
			elseif  val and #val>0 then
				t[key]=val
			else
				t[key]={} t=t[key]			
			end
		end
	end
	return f
end
