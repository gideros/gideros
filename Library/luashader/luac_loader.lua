local lc,lcp

local function loadBlock(n)
	local r=lc:sub(lcp,lcp+n-1) 
	lcp+=n
	return r
end

local function loadInt()
	local a,b,c,d=lc:byte(lcp,lcp+3) lcp+=4
	if endian then return d|(c<<8)|(b<<16)|(a<<24) end
	return a|(b<<8)|(c<<16)|(d<<24)
end

local function loadIntX()
	local a,b,c,d=lc:byte(lcp,lcp+3) lcp+=4
	if endian then return #d|(#c<<8)|(#b<<16)|(#a<<24) end
	return #a|(#b<<8)|(#c<<16)|(#d<<24)
end

local function loadByte()
	local a=lc:byte(lcp,lcp) lcp+=1
	return a
end

local function loadString()
	local s=loadInt()
	if s==0 then return nil end
	s=loadBlock(s)
	s=s:sub(1,#s-1)
	return s
end

local function loadVector(n,l)
	local v={}
	for i=1,n do v[i]=l() end
	return v
end

local function loadCode(f)
	local n=loadInt()
	f.codeSize=n
	f.code=loadVector(n,loadIntX)
end

local function loadNumber()
	return { "double", loadBlock(8):decodeValue("d",false) }
end

local function loadConstants(f)
	local n=loadInt()
	f.sizek=n
	local k={}
	for i=1,n do
		local t=loadByte()
		local v
		if t==1 then v=(loadByte()~=0)
		elseif t==3 then v=loadNumber()
		elseif t==4 then v=loadString()
		else assert(false,"Unsupported constant type in lua bytecode:"..t)
		end
		k[i]=v
	end
	f.k=k
	
	n=loadInt()
	f.sizep=n
	local p={}
	for i=1,n do p[i]=loadFunction(f.source) end
	f.p=p
end

local function loadDebug(f)
	local n=loadInt()
	f.sizelineinfo=n
	f.lineinfo=loadVector(n,loadInt)
	n=loadInt()
	f.sizelocvars=n
	local t={}
	for i=1,n do t[i]={ varname=loadString(), startpc=loadInt(), endpc=loadInt()} end
	f.locvars=t
	n=loadInt()
	f.sizeupvalues=n
	f.locvars=loadVector(n,loadString)
end

function loadFunction(p)
	local f={}
	f.source=loadString() or p
    f.linedefined=loadInt()
	f.lastlinedefined=loadInt()
	f.nups=loadByte(S)
	f.numparams=loadByte(S)
	f.is_vararg=loadByte(S)
	f.maxstacksize=loadByte(S)
	loadCode(f)
	loadConstants(f)
	loadDebug(f)
	return f
end

function luacLoad(m)
	lcp=1
	lc=m
	local hdr=loadBlock(12)
	endian=(hdr:byte(7)==0)
	--print("Endianness:",endian)
	return loadFunction()
end
