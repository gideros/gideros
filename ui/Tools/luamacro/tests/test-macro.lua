require 'pl'
local macro = require 'macro'

tmp = [[
for i = 1,10 do
    fred("hello",i)
    f(10,'hello')
    print(str(233))
    bonzo a,b,c;
    fun2(\x(x+1),\(a:get()),\x(\y(x+y)))
    frederick block
      for i = 1,10 do
        alpha block
          print(i)
        end
      end
    end

	try
      banzai(dog)
    except(ex)
      print(ex)
    end
end
]]

local M = macro
local define = M.define

define 'fred alice'
define 'alice print'
define 'f(x,y) fred(x..y))'
define ('str(x)',function(x)
    return '"'..x[1][2]..'"'
end)
define ('bonzo',function(get)
    local t = get();  M.assert(t == 'space','space required')
    local args = M.get_names(get,';')
    local res = {}
    M.put_keyword(res,'local')
    M.put_names(res,args)
    return res
end)

define ('@',function(get)
    return '"'..os.date('%c')..'"'
end)

define ('\\',function(get)
    --local args = M.get_names(get,'(')
	local args = get:names('(')
    local body = M.get_list(get)
	--[[
    local res = {}
    M.put_keyword(res,'function')
    M.put(res,'(')
    M.put_names(res,args)
    M.put (res,')')
    M.put_keyword(res,'return')
    M.put_list(res,body)
    M.put_space(res)
    M.put_keyword(res,'end')
    return res
	--]]
	local put = M.Putter()
	--print('*****',put,getmetatable(put).keyword)
	--return put;
	put:keyword 'function' '(' : names(args) ')'
	return put:keyword 'return' : list(body) : space() : keyword 'end'
end)

define ('block',function(get)
    M.block_handler(0,function(get)
        return ')'
    end)
    local res = {}
    M.put(res,'(')
    M.put_keyword(res,'function')
    M.put(res,'(')
    M.put(res,')')
    return res
end)


define ('_END_',function(get)
    local str = M.get_string(get)
    M.block_handler(0,function()
        return str
    end)
end)

define 'E_ _END_ " end"'

define 'try do local stat_,r_ = pcall(function()'
define 'except(e) end);  E_ if stat_ then if r_~=nil then return r_ end else local e = r_  '


local out = stringio.create()
macro.substitute(tmp,out)
print(out:value())

if arg[1] == '-i' then

io.write '? '
local line = io.read()
while line do
    local sub,err = macro.substitute_tostring(line..'\n')
	if sub then io.write(sub) else io.write(err,'\n') end
	io.write '? '
	line = io.read()
end

end
