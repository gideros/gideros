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
end

function UAPack:getFiles()
	return self.assets
end
