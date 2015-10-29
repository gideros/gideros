--[[---
Easy no-fuss modules.

Any function inside the module will be exported, unless it is explicitly
local. The functions are declared up front using patching, leading to efficient
calls between module functions.

    require_ 'module'

    function one ()
      return two()
    end

    function two ()
      return 42
    end

Classes can also be declared inside modules:

    require_ 'module'

    class A
        function set(self,val) @val = val end
        function get(self) return @val end
    end

Within class definitions, the macro `@` expands to either `self.` or `self:` depending
on context, and provides a Ruby-like shortcut.

If you give these modules names with `m.lua` extension like `mod.m.lua`, then you can
simply use `require()` to use them with LuaMacro.

@module macro.module
]]
local M = require 'macro'

local locals, locals_with_value = {}, {}
local ref

local function module_add_new_local (name)
    locals[#locals+1] = name
end

local function module_add_new_local_with_value (name,value)
    locals_with_value[name] = value
end


local function was_local_function (get)
  local tl,keyw = get:peek(-1)
   return tl=='keyword' and keyw=='local'
end

-- exclude explicitly local functions and anonymous functions.
M.keyword_handler('function',function(get)
  local tn,name = get:peek(1)
  local was_local = was_local_function(get)
  if not was_local and tn == 'iden' then
    module_add_new_local(name)
  end
end)

-- when the module is closed, this will patch the locals and
-- output the module table.
M.keyword_handler('END',function(get)
    local concat = table.concat
    local patch = ''
    if next(locals_with_value) then
        local lnames,lvalues = {},{}
        local i = 1
        for k,v in pairs(locals_with_value) do
            lnames[i] = k
            lvalues[i] = v
            i = i + 1
        end
        patch = patch..'local '..concat(lnames,',')..'='..concat(lvalues,',')..'; '
    end
    if #locals > 0 then
        patch = patch .. 'local '..concat(locals,',')
    end
    get:patch(ref,patch)
    local dcl = {}
    for i,name in ipairs(locals) do
        dcl[i] = name..'='..name
    end
    dcl = table.concat(dcl,', ')
    return 'return {' .. dcl .. '}'
end)

local no_class_require

-- the meaning of @f is either 'self.f' for fields, or 'self:f' for methods.
local function at_handler (get,put)
    local tn,name,tp = get:peek2(1)
    M.assert(tn == 'iden','identifier must follow @')
    return put:iden ('self',true) (tp=='(' and ':' or '.')
end

local function method_handler (get,put)
  local tn,name,tp = get:peek2()
  if not was_local_function(get) and tn == 'iden' and tp == '(' then
    return put ' ' :iden ('_C',true) '.'
  end
end

M.define ('_C_',function()
    M.define_scoped('@',at_handler)
    if not no_class_require then
        module_add_new_local_with_value('_class','require "macro.lib.class"')
        no_class_require = true
    end
    M.scoped_keyword_handler('function',method_handler)
end)

M.define('class',function(get)
    local base = ''
    local name = get:iden()
    if get:peek(1) == ':' then
        get:next()
        base = get:iden()
    end
    module_add_new_local(name)
    return ('do local _C = _class(%s); %s = _C; _C_\n'):format(base,name)
end)

-- the result of the macro is just a placeholder for the locals
return function(get,put)
    ref = get:placeholder(put)
    return put
end


