local macro = require 'macro'

macro.define('$',function(get)
   local t,v = get()
   if t == 'iden' then
      return 'os.getenv("'..v..'")'
   elseif t == '(' then
      local rest = get:upto ')'
      return 'eval("'..tostring(rest)..'")'
   end
end)

return function()
    return [[
local function eval(cmd)
    local f = io.popen(cmd,'r')
    local res = f:read '*a'
    f:close()
    return res
end
]]
end
