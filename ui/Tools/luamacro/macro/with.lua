--[[--
A `with` statement. This works more like the Visual Basic statement than the
Pascal one; fields have an explicit period to indicate that they are special.
This makes variable scoping explcit.

    aLongTableName = {}
    with aLongTableName do
      .a = 1
      .b = {{x=1},{x=2}}
      .c = {f = 2}
      print(.a,.c.f,.b[1].x)
    end

Fields that follow an identifier or a `}` are passed as-is.

@module macro.with
]]
local M = require 'macro'

M.define('with',function(get,put)
  M.define_scoped('.',function()
    local lt,lv = get:peek(-1,true) --  peek before the period...
    if lt ~= 'iden' and lt ~= ']' then
      return '_var.'
    else
      return nil,true -- pass through
    end
  end)
  local expr = get:upto 'do'
  return 'do local _var = '..tostring(expr)..'; '
end)
