--[[
Load meshes from a wavefromt .obj file
Usage:
sprite=loadObj(path,file) : load the file located at path/file

Returned sprite has a few specific attributes:
- objs: table referencing all objects within the loaded file

]]
local function Split(str, delim, maxNb)
    -- Eliminate bad cases...
    if string.find(str, delim) == nil then
        return { str }
    end
    if maxNb == nil or maxNb < 1 then
        maxNb = 0    -- No limit
    end
    local result = {}
    local pat = "(.-)" .. delim .. "()"
    local nb = 0
    local lastPos
    for part, pos in string.gfind(str, pat) do
        nb = nb + 1
        result[nb] = part
        lastPos = pos
        if nb == maxNb then break end
    end
    -- Handle the last field
    if nb ~= maxNb then
        result[nb + 1] = string.sub(str, lastPos)
    end
    return result
end

local function parsemtl(mtls,path,file)
 local mtl={ texturew=0, textureh=0 }
 for line in io.lines(path.."/"..file) do
  fld=Split(line," ",6)
  for i=1,#fld,1 do
  fld[i]=string.gsub(fld[i], "\r", "") 
  end
  if (fld[2]~=nil) then
	fld[2]=string.gsub(fld[2], "\r", "")
  end
  if fld[1]=="newmtl" then
    --print("DM",fld[2])
    mtl={texturew=0, textureh=0}
	mtls[fld[2]]=mtl
  elseif fld[1]=="Kd" then
   mtl.kd={fld[2],fld[3],fld[4],1.0}
  elseif fld[1]=="map_Kd" then
   table.remove(fld,1)
   local f=table.concat(fld," ")
   --print("Texture:.. ["..path.."/"..f.."]")
   mtl.texture=Texture.new(path.."/"..f,true,{ wrap=TextureBase.REPEAT })
   mtl.texturew=mtl.texture:getWidth()
   mtl.textureh=mtl.texture:getHeight()
  elseif fld[1]=="map_Bump" then
   table.remove(fld,1)
   local f=table.concat(fld," ")
   --print("Texture:.. ["..path.."/"..f.."]")
   mtl.normalMap=Texture.new(path.."/"..f,true,{ wrap=TextureBase.REPEAT })
   mtl.normalMapW=mtl.normalMap:getWidth()
   mtl.normalMapH=mtl.normalMap:getHeight()
  end
 end
end

function string.starts(String,Start)
   return string.sub(String,1,string.len(Start))==Start
end

function string.ends(String,End)
   return End=='' or string.sub(String,-string.len(End))==End
end

function loadObj(path,file)
 local spr=Sprite.new()
 v = {}
 imap = nil
 vt = {}
 vn = {}
 mtls={}
 mtl=nil
 spr.min={1000000,1000000,1000000} 
 spr.max={-1000000,-1000000,-1000000} 
 spr.objs={}
 oname=nil
 local function buildObject()  
	local m=nil
	if (imap~=nil) then
		for _,vm in pairs(imap.vngmap) do
			local nx,ny,nz=0,0,0
			for _,vn in ipairs(vm) do
				nx=nx+imap.lvn[vn+1]
				ny=ny+imap.lvn[vn+2]
				nz=nz+imap.lvn[vn+3]
			end
			local nl=math.sqrt(nx*nx+ny*ny+nz*nz)
			nx=nx/nl
			ny=ny/nl
			nz=nz/nl
			for _,vn in ipairs(vm) do
				imap.lvn[vn+1]=nx
				imap.lvn[vn+2]=ny
				imap.lvn[vn+3]=nz
			end	
		end
		m=Mesh.new(true)
		m:setVertexArray(imap.lv)
		m:setIndexArray(imap.i)
		--m:setColorArray(c)
		if (mtl.texture~=nil) then
			m:setTextureCoordinateArray(imap.lvt)
			m:setTexture(mtl.texture)
			m.hasTexture=true
		end
		if (mtl.normalMap~=nil) then
			m:setTexture(mtl.normalMap,1)
			m.hasNormalMap=true
		end
		if mtl.kd then
			m:setColorTransform(mtl.kd[1],mtl.kd[2],mtl.kd[3],mtl.kd[4])
		end
		if #imap.lvn>0 then
			m.hasNormals=true
			m:setGenericArray(3,Shader.DFLOAT,3,#imap.lvn/3,imap.lvn)
		end
		local sobj=spr.objs[oname]
		if sobj==nil then
			sobj=Sprite.new()
			spr:addChild(sobj)
			spr.objs[oname]=sobj
			sobj.min={1000000,1000000,1000000} 
			sobj.max={-1000000,-1000000,-1000000} 
		end
		sobj:addChild(m)
		local minx,miny,minz=100000,100000,100000
		local maxx,maxy,maxz=-100000,-100000,-100000
		for i=1,#imap.lv-2,3 do
		 local x,y,z=imap.lv[i],imap.lv[i+1],imap.lv[i+2]
		 minx=math.min(minx,x)
		 miny=math.min(miny,y)
		 minz=math.min(minz,z)
		 maxx=math.max(maxx,x)
		 maxy=math.max(maxy,y)
		 maxz=math.max(maxz,z)
		end
		m.min={minx,miny,minz}
		m.max={maxx,maxy,maxz}
		m.center={(m.max[1]+m.min[1])/2,(m.max[2]+m.min[2])/2,(m.max[3]+m.min[3])/2}
		sobj.min={math.min(sobj.min[1],m.min[1]),math.min(sobj.min[2],m.min[2]),math.min(sobj.min[3],m.min[3])}
		sobj.max={math.max(sobj.max[1],m.max[1]),math.max(sobj.max[2],m.max[2]),math.max(sobj.max[3],m.max[3])}
		spr.min={math.min(spr.min[1],m.min[1]),math.min(spr.min[2],m.min[2]),math.min(spr.min[3],m.min[3])}
		spr.max={math.max(spr.max[1],m.max[1]),math.max(spr.max[2],m.max[2]),math.max(spr.max[3],m.max[3])}
		m.name=oname
		--[[print(oname,m.min[1],m.min[2],m.min[3],m.max[1],m.max[2],m.max[3])
		if string.starts(oname,"light") then
			lightPosX,lightPosY,lightPosZ=m.center[1],m.center[2],m.center[3]
			lightRef=m
		end]]
		imap=nil
	end
	return m
 end
 for line in io.lines(path.."/"..file) do
  fld=Split(line," ")
  for i=1,#fld,1 do
  fld[i]=string.gsub(fld[i], "\r", "") 
  end
  if fld[1]=="v" then
	table.insert(v,tonumber(fld[2]))
	table.insert(v,tonumber(fld[3]))
	table.insert(v,tonumber(fld[4]))
  elseif fld[1]=="vn" then
	table.insert(vn,tonumber(fld[2]))
	table.insert(vn,tonumber(fld[3]))
	table.insert(vn,tonumber(fld[4]))
  elseif fld[1]=="vt" then
	table.insert(vt,tonumber(fld[2]))
	table.insert(vt,tonumber(fld[3]))
  elseif fld[1]=="f" then
	if imap==nil then
		imap={}
		imap.alloc=function(self,ifld,facenm)
         local ifl=Split(ifld,"/",3) 
		 local iv=tonumber(ifl[1])
		 if (iv<0) then
			iv=(#v/3+1+iv)
		 end
		 iv=iv-1
		 local it=tonumber(ifl[2])
		 if (it==nil) then																																														
		  it=-1
		 else
		  if (it<0) then
			it=(#vt/2)+it+1
		  end
		  it=it-1
		 end
		 local inm=tonumber(ifl[3])
		 if (inm==nil) then																																														
		  inm=-1
		 else
		  if (inm<0) then
			inm=(#vn/3)+inm+1
		  end
		  inm=inm-1
		 end
		 if inm==-1 then inm=facenm.code end
		 local ms=iv..":"..it..":"..inm
		 if self.vmap[ms]==nil then
			ni=self.ni+1
			self.ni=ni
			table.insert(facenm.lvi,#self.lv)
			table.insert(self.lv,v[iv*3+1])
			table.insert(self.lv,v[iv*3+2])
			table.insert(self.lv,v[iv*3+3])
			if it>=0 then
				table.insert(self.lvt,vt[it*2+1]*mtl.texturew)
				table.insert(self.lvt,vt[it*2+2]*mtl.textureh)
			end
			if inm>=0 then
				table.insert(self.lvn,vn[inm*3+1])
				table.insert(self.lvn,vn[inm*3+2])
				table.insert(self.lvn,vn[inm*3+3])
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
	end
	local itab={}
	local normtab={ code=imap.gnorm, lvi={}, lvni={} }
	for ii=2,#fld,1 do
		if (fld[ii]~=nil) and (fld[ii]~="") then
			table.insert(itab,imap:alloc(fld[ii],normtab))
		end
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
  elseif fld[1]=="o" or fld[1]=="g" then
  buildObject()
  --print(line)
  oname=fld[2]
  elseif fld[1]=="mtllib" then
   table.remove(fld,1)
   parsemtl(mtls,path,table.concat(fld," "))
  elseif fld[1]=="usemtl" then
   buildObject()
   mtl=mtls[fld[2]]
  end
 end
 --spr:setColorTransform(1.0,0,0,1.0)
 buildObject() --If any in progress
 spr.center={(spr.max[1]+spr.min[1])/2,(spr.max[2]+spr.min[2])/2,(spr.max[3]+spr.min[3])/2}
 return spr
end
