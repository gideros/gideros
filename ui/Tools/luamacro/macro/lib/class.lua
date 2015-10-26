----
-- a basic class mechanism.
-- Used for some of the demonstrations; the `class` macro in the `module`
-- package uses it.  It provides a single function which returns a new 'class'.
-- The resulting object can be called to generate an instance of the class.
-- You may provide a base class for single inheritance; in this case, the functions
-- of the base class will be copied into the new class' metatable (so-called 'fat metatable')
--
-- Example:
--
--     local class = require 'macro.lib.class'
--     A = class()
--     function A._init(name) self.name = name end
--     a = A("hello")
--     assert(a.name == "hello")
--
-- @module macro.lib.class

return function (base)
  -- OOP with single inheritance
  local klass,cmt = {},{}
  if base then -- 'fat metatable' inheritance
    for k,v in pairs(base) do klass[k] = v end
  end
  klass.__index = klass
  -- provide a callable constructor that invokes user-supplied ctor
  function cmt:__call(...)
    local obj = setmetatable({},klass)
    if klass._init then klass._init(obj,...)
    elseif base and base._init then base._init(base,...) end
    return obj
  end
  setmetatable(klass,cmt)
  return klass
end
