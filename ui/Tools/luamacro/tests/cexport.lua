local M = require 'macro'

local cf,copy,null
if package.config:sub(1,1) == '\\' then
    cf = 'fc'; copy = 'copy'; null = ' > null'
else
    cf = 'diff'; copy = 'cp'; null = ' > /dev/null'
end

local f,hname,mname

M.keyword_handler('BEGIN',function()
    hname = M.filename:gsub('%.%a+$','')..'.h'
    mname = hname:gsub('%.','_'):upper()
    f = io.open(M.filename..'.h','w')
    f:write('#ifndef ',mname,'\n')
    f:write('#define ',mname,'\n')
end)

M.keyword_handler ('END',function()
    f:write('#endif\n')
    f:close()
    local tmpf = M.filename..'.h'
    if os.execute(cf..' '..hname..' '..tmpf..null) ~= 0 then
        os.execute(copy..' '..tmpf..' '..hname..null)
    end
end)

M.define('export',function(get)
    local t,v = get:next()
    local decl,out
    if v == '{' then -- block!
        decl = tostring(get:upto '}')
        decl = M.substitute_tostring(decl)
        f:write(decl,'\n')
    else
        decl = v .. ' ' .. tostring(get:upto '{')
        decl = M.substitute_tostring(decl)
        f:write(decl,';\n')
        out = decl .. '{'
    end
    return out
end)


--[[

Example of a with-statement:

    with(MyType *,bonzo) {
        .x = 2;
        .y = 3;
        with(SubType *,.data) {
            .name = "hello";
            .ids = my.ids;
            printf("count %d\n",.count);
        }
    }


M.define('with',function(get)
  get:expecting '('
  local args = get:list()
  local T, expr = args[1],args[2]
  get:expecting '{'
  M.define_scoped('.',function()
    local lt,lv = get:peek(-1,true) --  peek before the period...
    if lt ~= 'iden' then
      return '_var->'
    else
      return nil,true -- pass through
    end
  end)
  return '{ ' .. tostring(T) .. ' _var = '..tostring(expr)..'; '
end)

]]
