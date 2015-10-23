local macro = require 'macro'

macro.define('forall',function(get)
  local var = get:iden()
  local t,v = get:next() -- will be 'in'
  local rest = tostring(get:upto 'do')
  return ('for _,%s in ipairs(%s) do'):format(var,rest)
end)
