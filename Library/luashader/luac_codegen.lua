--!NEEDS:luac_loader.lua
local debug=false
local codeName={
  "MOVE",
  "LOADK",
  "LOADBOOL",
  "LOADNIL",
  "GETUPVAL",
  "GETGLOBAL",
  "GETTABLE",
  "SETGLOBAL",
  "SETUPVAL",
  "SETTABLE",
  "NEWTABLE",
  "SELF",
  "ADD",
  "SUB",
  "MUL",
  "DIV",
  "MOD",
  "POW",
  "BOR",
  "BAND",
  "OP_BXOR",
  "BLSHFT",
  "BRSHFT",
  "BNOT",
  "INTDIV",
  "UNM",
  "NOT",
  "LEN",
  "CONCAT",
  "JMP",
  "EQ",
  "LT",
  "LE",
  "TEST",
  "TESTSET",
  "CALL",
  "TAILCALL",
  "RETURN",
  "FORLOOP",
  "FORPREP",
  "TFORLOOP",
  "SETLIST",
  "CLOSE",
  "CLOSURE",
  "VARARG",
  "MAX",
  "MIN",
  "DEG",
  "RAD",
  "ADD_EQ",
  "SUB_EQ",
  "MUL_EQ",
  "DIV_EQ",
  "MOD_EQ",
  "POW_EQ",
}

--UNSUPPORTED: GETUPVAL,SETUPVAL,NEWTABLE,SETTABLE,SELF
local function mprint(...)
	if debug then print(...) end
end

local function KStr(k)
	if type(k)=="table" then
		if k[1]=="double" then
			return k[2]
		end
	end
	return tostring(k)
end

local function RKStr(f,i)
	if (i&0x100)>0 then
		return KStr(f.k[i&0xFF+1])
	else	
		return "$"..i
	end
end

local function numvalue(n)
	n=tostring(n)
	if (n:find("%.")) then return n end
	return n..".0"
end

local function KVal(f,i)
	local k=f.k[i+1]
	assert((type(k)=="table")and(k[1]=="double"),"Only numbers are allowed")
	return { type="val",value=numvalue(k[2]),vtype="hF1" }
end

local function KRVal(f,i)
	if (i&0x100)>0 then
		local k=f.k[i&0xFF+1]
		assert((type(k)=="table")and(k[1]=="double"),"Only numbers are allowed")
		return { type="val",value=numvalue(k[2]),vtype="hF1" }
	else
		return f._l[i]
	end		
end


local function outcode(f,s)
	--print("C:",s)
	f._c=f._c..s
end
local function mapGlobal(f,sym)
	assert(f._g[sym],"Unknown global "..sym)
	return f._g[sym]
end
local ivar=1
local function ensureLocalVar(f,v,r)
	--print("LOCAL:"..v) 
	if (f._l[v] and f._l[v].type=="var" and f._l[v].vtype==r) then return end
	local vname="_GID_Local_"..ivar
	ivar+=1
	assert(f._t[r],"Unknown type '"..r.."'")
	f._hc=f._hc..f._t[r].." "..vname..";\n"
	if (f._l[v] and f._l[v].type=="val" and f._l[v].vtype==r) then
		outcode(f,vname.."="..f._l[v].value..";\n")
	end
	f._l[v]={type="var",value=vname,vtype=r}
end

function gen_GETUPVAL(is,f,a,b,c,bc)
	assert(false,"Upvalues aren't supported")
end
function gen_SETUPVAL(is,f,a,b,c,bc)
	assert(false,"Upvalues aren't supported")
end

function gen_GETGLOBAL(is,f,a,b,c,bc)
	mprint("$"..a.."=_G["..KStr(f.k[bc+1]).."]")
	f._l[a]=mapGlobal(f,KStr(f.k[bc+1]))
end

function gen_SETGLOBAL(is,f,a,b,c,bc)
mprint("_G["..KStr(f.k[bc+1]).."]=$"..a)
outcode(f,mapGlobal(f,KStr(f.k[bc+1])).value.."="..f._l[a].value..";\n")
end

local function checkSwizzling(var,sw)
	assert(#sw>0 and #sw<=4,"Swizzling pattern size issue:"..sw)
	local vecType=var.vtype:sub(1,2)
	local vecSize=tonumber(var.vtype:sub(3))
	assert(vecSize,"Only vectors can be swizzled: "..var.vtype)
	assert((vecType=="lF" or vecType=="hF" or vecType=="mF") and (vecSize>1) and (vecSize<=4),"Only vectors of float are supported when swizzling: "..var.vtype)
	local comps="xyrgst"
	if vecSize>2 then comps=comps.."zbp" end
	if vecSize>3 then comps=comps.."waq" end
	assert(sw==sw:match("["..comps.."]+"),"Invalid swizzling pattern: "..sw)
	return vecType..(#sw)
end

function gen_GETTABLE(is,f,a,b,c,bc)
	local idx=RKStr(f,c)
	mprint("$"..a.."=$"..b.."["..idx.."]")
	local newType=checkSwizzling(f._l[b],idx)
	--UGLY, this only handle swizzling, shader specific, and don't do any checks
	local bv=f._l[b].value.."."..idx
	ensureLocalVar(f,a,newType)
	outcode(f,f._l[a].value.."="..bv..";\n")
	--f._l[a]={type=f._l[b].type,value=f._l[b].value.."."..idx,vtype=newType}
--outcode(f,mapGlobal(f,KStr(f.k[bc+1])).value.."="..f._l[a].value..";\n")
end

function gen_SETTABLE(is,f,a,b,c,bc)
	local idx=RKStr(f,b)
	mprint("$"..a.."["..idx.."]".."=$"..c)
	local newType=checkSwizzling(f._l[a],idx)
	--UGLY, this only handle swizzling, shader specific, and don't do any checks
	assert(f._l[a].type=="var","Cannot set value of ("..f._l[a].value.."), a '"..f._l[a].type.."'")
	--assert(f._l[a].vtype==newType,"Cannot set value of ("..f._l[a].value.."), type mismatch '"..f._l[a].vtype.."'!='"..newType.."'")
	outcode(f,f._l[a].value.."."..idx.."="..KRVal(f,c).value..";\n")
end

function gen_FORPREP(is,f,a,b,c,bc,sbc)
	mprint("for ($"..a..",PC:"..sbc..")")
	ensureLocalVar(f,a,f._l[a].vtype)
	ensureLocalVar(f,a+3,f._l[a].vtype)
	outcode(f,("for (%s=%s;%s<=%s;%s+=%s,%s=%s) {"):format(f._l[a+3].value,f._l[a].value,f._l[a].value,f._l[a+1].value,f._l[a].value,f._l[a+2].value,f._l[a+3].value,f._l[a].value))
end

function gen_FORLOOP(is,f,a,b,c,bc,sbc)
	mprint("forloop ($"..a..",PC:"..sbc..")")
	outcode(f,"}\n")
end

function gen_MOVE(is,f,a,b)
	assert(f._l[b],"Local "..b.." not yet initialized")
	f._l[a]=f._l[b]
mprint("$"..a.."=$"..b)
end

function gen_opun(is,f,a,b,op,rtype)
	mprint("$"..a.."="..op..RKStr(f,b))
	local vb=KRVal(f,b)
	local terms=f._handlers.GENOPUN and f._handlers.GENOPUN(op,vb.value)
	if not terms then 
		terms=op.."("..vb.value..")" 
	end
	rtype=rtype or f._ot[op..vb.vtype] or vb.vtype
	ensureLocalVar(f,a,rtype)
	outcode(f,f._l[a].value.."="..terms..";\n")
end

function gen_UNM(is,f,a,b)
	gen_opun(is,f,a,b,"-")
end

function gen_op(is,f,a,b,c,op,rtype)
	mprint("$"..a.."="..RKStr(f,b)..op..RKStr(f,c))
	local vb=KRVal(f,b)
	local vc=KRVal(f,c)
	local terms=f._handlers.GENOP and f._handlers.GENOP(op,vb,vc)
	if not terms then 
		if op=="^^" then
			terms="pow("..vb.value..","..vc.value..")"
		else
			terms="("..vb.value..op..vc.value..")" 
		end
	end
	rtype=rtype or f._ot[vb.vtype..op..vc.vtype] or vb.vtype
	ensureLocalVar(f,a,rtype)
	outcode(f,f._l[a].value.."="..terms..";\n")
end

function gen_MUL(is,f,a,b,c)
	gen_op(is,f,a,b,c,"*")
end

function gen_DIV(is,f,a,b,c)
	gen_op(is,f,a,b,c,"/")
end

function gen_ADD(is,f,a,b,c)
	gen_op(is,f,a,b,c,"+")
end

function gen_SUB(is,f,a,b,c)
	gen_op(is,f,a,b,c,"-")
end

function gen_MOD(is,f,a,b,c)
	gen_op(is,f,a,b,c,"%")
end

function gen_POW(is,f,a,b,c)
	gen_op(is,f,a,b,c,"^^")
end

function gen_opeq(is,f,a,b,op,rtype)
	mprint("$"..a..op..RKStr(f,b))
	local vb=KRVal(f,b)
	rtype=rtype or f._ot[f._l[a].vtype..op..vb.vtype] or vb.vtype
	ensureLocalVar(f,a,rtype)
	local terms=f._handlers.GENOPEQ and f._handlers.GENOPEQ(op,f._l[a],vb)
	if not terms then 
		if op=="^^=" then
			terms=f._l[a].value.."=pow("..f._l[a].value..","..vb.value..")"
		else
			terms=f._l[a].value..op..vb.value 
		end
	end

	outcode(f,terms..";\n")
end

function gen_ADD_EQ(is,f,a,b,c)
	gen_opeq(is,f,a,b,"+=")
end

function gen_SUB_EQ(is,f,a,b,c)
	gen_opeq(is,f,a,b,"-=")
end

function gen_MUL_EQ(is,f,a,b,c)
	gen_opeq(is,f,a,b,"*=")
end

function gen_DIV_EQ(is,f,a,b,c)
	gen_opeq(is,f,a,b,"/=")
end

function gen_MOD_EQ(is,f,a,b,c)
	gen_opeq(is,f,a,b,"%=")
end

function gen_POW_EQ(is,f,a,b,c)
	gen_opeq(is,f,a,b,"^^=")
end

local function endblock(c,f)
	outcode(f,"}\n")
end

function gen_compop(is,f,a,b,c,op,lc)
	mprint("$"..a.."="..RKStr(f,b)..op..RKStr(f,c))
	
	local jis=f.code[lc+1]
    assert((jis&0x3F)()==29,"JMP expected after a conditional")
	local joff=((jis>>14)&0x3FFFF)()-131071
	f._lc=f._lc+1
	local vb=KRVal(f,b)
	local vc=KRVal(f,c)
			
	local terms="("..vb.value..op..vc.value..")"
	local abool="false" if (a>0) then abool="true" end
	outcode(f,"if (("..terms..")!="..abool..") {\n")
	--f._breaks[lc+2]={ callback=endblock	}
	f._breaks[lc+2+joff]={ callback=endblock	}
end

function gen_EQ(is,f,a,b,c,bc,sbc,lc)
	gen_compop(is,f,a,b,c,"==",lc)
end

function gen_LT(is,f,a,b,c,bc,sbc,lc)
	gen_compop(is,f,a,b,c,"<",lc)
end

function gen_LE(is,f,a,b,c,bc,sbc,lc)
	gen_compop(is,f,a,b,c,"<=",lc)
end

function gen_LOADK(is,f,a,b,c,bc)
mprint("$"..a.."="..KStr(f.k[bc+1]))
	local v=KVal(f,bc)
	f._l[a]={type="val",value=v.value, vtype="hF1"}
end

function gen_JMP(is,f,a,b,c,bc,sbc,lc)
assert(false,"Standalone JMP aren't supported")
 --Generate if context
--	outcode(f,"if ("..f._l[a].value..") {\n")
	f._breaks[lc+1+sbc]={ callback=endblock	}
--TODO: handle subpart
end

function gen_CALL(is,f,a,b,c)
	assert((c-1)<=1,"Multiple returns aren't allowed ("..(c-1)..")")
	assert(c>0,"Indeterminate return count aren't allowed. Avoid directly passing the result of a function as the last arg of another.")
	assert(f._l[a].type=="func",f._l[a].value.." isn't a function")
	mprint("$"..a.."=$"..a.."(#"..b..")")
	local fn=f._l[a].value
	local fne=f._l[a].evaluate
	local fnr=f._l[a].rtype
	if c==2 then -- Return
		assert(fnr,"Function '"..fn.."' return type not specified")
		if tonumber(fnr) then 
			fnr=f._l[a+fnr].vtype
		end
		ensureLocalVar(f,a,fnr)
		outcode(f,f._l[a].value.."=")
	end		
	if fne then
		--INLINE
		local args={}
		for p=1,b-1 do
			args[p]=f._l[a+p].value
		end
		outcode(f,fne(unpack(args))..";\n")
	else
		outcode(f,fn.."(")
		for p=1,b-1 do
			outcode(f,f._l[a+p].value)
			if p<(b-1) then outcode(f,",") end
		end
		outcode(f,");\n")
	end
end

function gen_RETURN(is,f,a,b)
assert(b<=2,"Multiple returns aren't allowed")
	mprint("return $"..a.." (#"..(b-1)..")")
	if (f._handlers.RETURN) then
		local gcode=nil
		if (b<2) then 
			gcode=f._handlers.RETURN()
		else
			gcode=f._handlers.RETURN(f._l[a].value)			
		end
		if gcode then
			outcode(f,gcode)
			return
		end
	end
	if (b<2) then 
		outcode(f,"return;\n")
	else
		outcode(f,"return "..f._l[a].value..";\n");
	end
end

local pcount=0
local tcount=0
local function processFunction(f)	
	assert(f.nups==0,"Upvalues not supported")
	if f.nups==0 then --candidate for C
		pcount+=1
		--print("FCT",f.source)
		local lc=1
		f._breaks={}
		while lc<=f.codeSize do
			if f._breaks[lc] then f._breaks[lc]:callback(f,lc) end
			local is=f.code[lc] lc+=1
			local a=((is>>6)&0xFF)()
			local c=((is>>14)&0x1FF)()
			local b=((is>>23)&0x1FF)()
			local bc=((is>>14)&0x3FFFF)()
			local op=(is&0x3F)()
			local cn=codeName[op+1]
			mprint(lc,cn,op,a,b,c,bc,bc-131071)
			f._lc=lc
			assert(_G["gen_"..cn],cn.." instruction isn't supported")
			_G["gen_"..cn](is,f,a,b,c,bc,bc-131071,lc-1) --Add sbc with bias
			lc=f._lc
		end
	end
	tcount+=1
	--for _,p in ipairs(f.p) do processFunction(p) end
end

function codegen(f,argsmap,globalmap,typemap,optypemap,ophandlers)
	pcount=0
	tcount=0
	if type(f)=="function" then
		f=luacLoad(string.dump(f))
	end
	f._c=""
	f._hc=""
	f._g=globalmap or {}
	f._l={}
	f._t=typemap
	f._ot=optypemap
	f._handlers=ophandlers or {}
	for k,v in ipairs(argsmap or {}) do f._l[k-1]=v  end
	processFunction(f)
	--print("ProcessedCount",pcount,tcount)
	--print("CODE:\n",f._c)
	return f._hc..f._c
end