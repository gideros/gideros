
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
        result[nb + 1] = string.sub(str, lastPos):match("(.-)%s*$")
    end
    return result
end

local function parsemtl(mtls,path,file)
 mtl=nil
 for line in io.lines(path.."/"..file) do
  fld=Split(line," ",6)
  if fld[1]=="newmtl" then
    mtl={}
	mtls[fld[2]]=mtl
  elseif fld[1]=="map_Kd" then
   mtl.texture=Texture.new(path.."/"..fld[2],true)
   mtl.texturew=mtl.texture:getWidth()
   mtl.textureh=mtl.texture:getHeight()
  end
 end
end

function loadObj(path,file)
 local spr=Sprite.new()
 v = {}
 imap = nil
 vt = {}
 mtls={}
 mtl=nil
 for line in io.lines(path.."/"..file) do
  fld=Split(line," ",6)
  if fld[1]=="v" then
	table.insert(v,tonumber(fld[2]))
	table.insert(v,tonumber(fld[3]))
	table.insert(v,tonumber(fld[4]))
  elseif fld[1]=="vt" then
	table.insert(vt,tonumber(fld[2]))
	table.insert(vt,tonumber(fld[3]))
  elseif fld[1]=="f" then
	if imap==nil then
		imap={}
		imap.alloc=function(self,ifld)
         local ifl=Split(ifld,"/",3) 
		 local iv=tonumber(ifl[1])-1
		 local it=tonumber(ifl[2])
		 if (it==nil) then																																														
		  it=0
		 else
		  it=it-1
		 end
		 local ms=iv..":"..it
		 if self.vmap[ms]==nil then
			ni=self.ni+1
			self.ni=ni
			table.insert(self.lv,v[iv*3+1])
			table.insert(self.lv,v[iv*3+2])
			table.insert(self.lv,v[iv*3+3])
			table.insert(self.lvt,vt[it*2+1]*mtl.texturew)
			table.insert(self.lvt,vt[it*2+2]*mtl.textureh)
			self.vmap[ms]=ni
		 end
		 return self.vmap[ms]
		end		
		imap.i={}
		imap.ni=0
		imap.lv={}
		imap.lvt={}
		imap.vmap={}
	end
	i1=imap:alloc(fld[2])
	i2=imap:alloc(fld[3])
	i3=imap:alloc(fld[4])
	if (fld[5]~=nil) then
		i4=imap:alloc(fld[5])
	else
		i4=nil
	end
	table.insert(imap.i,i1)
	table.insert(imap.i,i2)
	table.insert(imap.i,i3)
	if (i4~=nil) then
	 table.insert(imap.i,i1)
	 table.insert(imap.i,i3)
	 table.insert(imap.i,i4)
	end
  elseif fld[1]=="o" then
  print(line)
  if (imap~=nil) then
	local m=Mesh.new(true)
	m:setVertexArray(imap.lv)
	m:setTextureCoordinateArray(imap.lvt)
	m:setIndexArray(imap.i)
	--m:setColorArray(c)
	if (mtl~=nil) then
	 m:setTexture(mtl.texture)
	end
	spr:addChild(m)
    imap=nil
  end
  elseif fld[1]=="mtllib" then
   parsemtl(mtls,path,fld[2])
  elseif fld[1]=="usemtl" then
   mtl=mtls[fld[2]]
  end
 end
 --spr:setColorTransform(1.0,0,0,1.0)
 return spr
end
