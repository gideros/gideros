


local OPS={}
local function numvalue(n)
	n=tostring(n)
	if (n:find("%.")) then return n end
	return n..".0"
end

local function isInt(expr)
	return tonumber(expr.value//1)==tonumber(expr.value)
end

local function promote(expr,type)
	assert(expr.vtype=="hF1" and type=="hI1","Only turning hF1 to hI1 is supported")
	expr.vtype=type
	expr.value=tostring(expr.value//1)
end

function OPS:genOp()
	local co=self.ctable[self.pc]
	assert(co,"Expected op not found")
	self.pc+=1
	assert(OPS[co.op],"Op '"..co.op.."("..(co.a1 or "")..","..(co.a2 or "")..")' not handled")
	--print("GENOP:",self.skipping,co.op.."("..(co.a1 or "")..","..(co.a2 or "")..")")
	return OPS[co.op](self,co.a1,co.a2)
end

function OPS:skipOp(n)
	self.skipping=(self.skipping or 0)+1
	for i=1,n or 1 do
		self:genOp()
	end
	self.skipping-=1
	if self.skipping==0 then self.skipping=nil end
end

function OPS:nextOp()
	return self.ctable[self.pc] and self.ctable[self.pc].op
end

function OPS:indent()
	return ("  "):rep(self.indentc)
end

function OPS:CSTN(val)
	if self.skipping then return end
	return { value=numvalue(val), vtype="hF1" }
end

function OPS:CSTB(val)
	if self.skipping then return end
	return { value=val, vtype="BOOL" }
end

function OPS:VGLB(sym)
	if self.skipping then return end
	assert(self._g[sym],"Unknown global "..sym)
	--print("VGLB:",self._g[sym].type,self._g[sym].value,self._g[sym].vtype)
	return self._g[sym]
end

function OPS:VLOC(name)
	if self.skipping then return end
	return { value=name, vtype=self.locals[name] or "hF1" }
end

function OPS:EFCT(acount)
	for i=1,acount do
		-- Fetch args
	end
	while self:nextOp()~="SBLK" do self.pc+=1 end
	if self.skipping then return self:skipOp() end
	return self:genOp()
end

local function checkSwizzling(var,sw)
	assert(#sw>0 and #sw<=4,"Swizzling pattern size issue:"..sw)
	local vecType=var.vtype:sub(1,2)
	local vecSize=tonumber(var.vtype:sub(3))
	assert(vecSize,"Only vectors can be swizzled: "..var.vtype.."["..sw.."]")
	assert((vecType=="lF" or vecType=="hF" or vecType=="mF") and (vecSize>1) and (vecSize<=4),"Only vectors of float are supported when swizzling: "..var.vtype.."["..sw.."]")
	local comps="xyrgst"
	if vecSize>2 then comps=comps.."zbp" end
	if vecSize>3 then comps=comps.."waq" end
	assert(sw==sw:match("["..comps.."]+"),"Invalid swizzling pattern: "..sw)
	return vecType..(#sw)
end

function OPS:ETIN(name)
	if self.skipping then return self:skipOp() end
	local expr=self:genOp()
	
	return { value=expr.value.."."..name, vtype=checkSwizzling(expr,name) }
end

function OPS:ETIE(name)
	if self.skipping then return self:skipOp(2) end
	local expr=self:genOp()
	local index=self:genOp()
	return { value=expr.value.."["..index.value.."]", vtype="hF1" }
end

function OPS:EIFE(name)
	if self.skipping then return self:skipOp(3) end
	local cond=self:genOp()
	local etrue=self:genOp()
	local efalse=self:genOp()
	return { value="(("..cond.value..")?("..etrue.value.."):("..efalse.value.."))", vtype=etrue.type }
end

function OPS:EUNA(op)
	if self.skipping then return self:skipOp() end
	local expr=self:genOp()
	if op=="not" then op="!" end
	local rtype=self._ot[op..expr.vtype] or expr.vtype
	return { vtype=rtype, value=op..expr.value }
end

function OPS:EGRP(op)
	if self.skipping then return self:skipOp() end
	local expr=self:genOp()
	return { vtype=expr.vtype, value="("..expr.value..")" }
end

function OPS:EBIN(op)
	if self.skipping then return self:skipOp(2) end
	local left=self:genOp()
	local right=self:genOp()
	local terms=self._handlers.GENOP and self._handlers.GENOP(op,left,right)
	if not terms then 
		if op=="^" then
			terms="pow("..left.value..","..right.value..")"
		else
			terms=left.value..op..right.value 
		end
	end
	local rtype=self._ot[left.vtype..op..right.vtype] or left.vtype
	return { vtype=rtype, value=terms }
end

function OPS:ECAL(acount)
	local var=self:genOp()
	local args={}
	local sargs=""
	if self.skipping then return self:skipOp(acount) end

	for i=1,acount do 
		local a=self:genOp()
		sargs=sargs..a.value.."," 
		table.insert(args,a)
	end

	local rtype=var.rtype or "hF1"
	if tonumber(rtype) then 
		rtype=args[tonumber(rtype)].vtype
	end
	if var.evaluate then
		return { vtype=var.rtype, value=var.evaluate(self,unpack(args)) }
	end
	
	if var.callargs and #var.callargs>0 then
		sargs=sargs..var.callargs
	end

	if #sargs>0 then sargs=sargs:sub(1,#sargs-1) end
	return { value=var.value.."("..sargs..")", vtype=rtype }
end

function OPS:SLFN(name)
	if self.skipping then return self:skipOp() end
	self.localFunctions[name]=self:genOp()
	return ""
end

function OPS:SFCT()
	if self.skipping then return self:skipOp(2) end
	local name=self:genOp().value
	local glf=self._g[name]
	assert(glf.type=="func",glf.value.." must be a function")
	
	local spc=self.pc
	self:skipOp()
	local stable={}
	for i=spc,self.pc do table.insert(stable,self.ctable[i]) end
	
	glf._fcode=stable
	
	if self._handlers.SUBFUNC then
		self._handlers.SUBFUNC(self,glf)
	end
	return ""
end

function OPS:SCAG(op)
	if self.skipping then return self:skipOp(2) end
	local var=self:genOp()
	local exp=self:genOp()
	local cop=op.."="
	local terms=self._handlers.GENOPEQ and self._handlers.GENOPEQ(cop,var,exp)
	if not terms then 
		if cop=="^=" then
			terms=var.value.."=pow("..var.value..","..exp.value..")"
		else
			terms=var.value..cop..exp.value 
		end
	end
	return self:indent()..terms..";\n"
end

function OPS:SBLK(scount)
	if self.skipping then return self:skipOp(scount) end
	local ss=""
	self.indentc+=1
	for i=1,scount do ss=ss..self:genOp() end
	self.indentc-=1
	return self:indent().."{\n"..ss..self:indent().."}\n"
end

function OPS:SRTN(rcount)
	if self.skipping then return self:skipOp(rcount) end
	rcount=tonumber(rcount)
	assert(rcount<2,"Can't return more than one value")
	local rval={}
	if rcount==1 then
		rval=self:genOp()
	end
	local ret=""
	if self._handlers.RETURN then
		--print("RetVal:",rval)
		ret=self._handlers.RETURN(rval.value)
	else
		ret="return "..rval.value..";\n"
	end
	return self:indent()..ret
end

function OPS:SIF_(hasElse)
	local hasElse=tonumber(hasElse)>0	
	if self.skipping then 
		local ac=2
		if hasElse then ac=3 end
		return self:skipOp(ac) 
	end
	local cond=self:genOp()
	local thenB
	if cond.value=="false" then 
		thenB="{}\n" 
		self:skipOp() 
	else
		thenB=self:genOp()
	end
	local ss=self:indent().."if ("..cond.value..")\n"..thenB
	if hasElse then
		local elseB
		if cond.value=="true" then 
			elseB="{}\n" 
			self:skipOp() 
		else
			elseB=self:genOp()
		end
		ss=ss..self:indent().."else\n"..elseB
	end
	return ss
end

function OPS:SWHL()
	if self.skipping then return self:skipOp(2) end
	local cond=self:genOp()
	local body=self:genOp()
	return self:indent().."while ("..cond.value..")\n"..body
end

function OPS:SRPT()
	if self.skipping then return self:skipOp(2) end
	local body=self:genOp()
	local cond=self:genOp()
	return self:indent().."do\n"..body..self:indent().."while (!("..cond.value.."));\n"
end

function OPS:SFOR(var,hasInc)
	hasInc=tonumber(hasInc)>0
	if self.skipping then 
		local ac=2
		if hasInc then ac=3 end
		return self:skipOp(ac) 
	end
	local ss=""
	local from=self:genOp()
	local to=self:genOp()
	local step={ value="1.0" }
	if hasInc then step=self:genOp() end
	local type=from.vtype
	step.vtype=type
	--Reduce type to int if possible
	if isInt(from) and isInt(to) and isInt(step) then 
		type="hI1" 
		promote(from,type)
		promote(to,type)
		promote(step,type)
	end
	type=self._t[type] or type
	return self:indent()..(("for (%s %s=%s;%s<=%s;%s+=%s)\n"):format(type,var,from.value,var,to.value,var,step.value))..self:genOp()
end

function OPS:SLCL(vcount)
	local vn={}
	while self:nextOp()=="SLNM" do 
		table.insert(vn,{ name=self:genOp(), type="hF1" })
	end
	if self.skipping then return self:skipOp(vcount) end
	for i=1,vcount do
		local exp=self:genOp()
		vn[i].val=exp.value
		vn[i].type=exp.vtype
		assert(self._t[exp.vtype],"Unknown type '"..exp.vtype.."'")
	end
	local ss=""
	for _,vd in ipairs(vn) do
		local veq=""
		if vd.val then veq="=" end
		local ltype=self._t[vd.type] or vd.type
		ss=ss..self:indent()..ltype.." "..vd.name..veq..(vd.val or "")..";\n"
		self.locals[vd.name]=vd.type
	end
	return ss
end

function OPS:SAGN(vcount)
	if self.skipping then return self:skipOp(vcount*2) end
	local vn={}
	for i=1,vcount do
		local exp=self:genOp()
		table.insert(vn,{ name=exp.value, type=exp.type })
	end
	for i=1,vcount do
		local exp=self:genOp()
		vn[i].val=exp.value
		vn[i].type=exp.vtype
	end
	local ss=""
	for _,vd in ipairs(vn) do
		local veq=""
		if vd.val then veq="=" end
		ss=ss..self:indent()..vd.name..veq..(vd.val or "")..";\n"
	end
	return ss
end

function OPS:SEXP()
	if self.skipping then return self:skipOp() end
	return self:indent()..self:genOp().value..";\n"
end

function OPS:SLNM(name)
	return name
end

function codegen_u(f,argsmap,globalmap,typemap,optypemap,ophandlers)
	--print(pcode)
	local ctable={}
	if type(f)=="function" then
		local pcode=string.dumpPseudocode(f)
		for code,a1,a2 in pcode:gmatch("(....):?([^:\n]*):?([^\n]*)\n") do
			table.insert(ctable,{ op=code, a1=a1, a2=a2 })
		end
	else
		ctable=f
	end
	assert(#ctable>0,"No code found")
	local ctx={ ctable=ctable, pc=1, indentc=0, localFunctions={}, locals={} }
	ctx._g=globalmap or {}
	ctx._l={}
	ctx._t=typemap
	ctx._ot=optypemap
	ctx._handlers=ophandlers or {}
	for k,v in ipairs(argsmap or {}) do  if v.value then ctx.locals[v.value]=v.vtype  end end

	setmetatable(ctx, { __index=OPS })
	assert(ctable[1].op=="EFCT","Not a function definition")
	local rcode=ctx:genOp()
	--print(rcode)
	return rcode
end