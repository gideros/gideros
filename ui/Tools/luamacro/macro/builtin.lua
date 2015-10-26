-------
-- LuaMacro built-in macros.
-- @module macro.builtin

local M = require 'macro'

local function macro_def (scoped)
    return function (get)
        local t,name,parms,openp
        local t,name = get:next()
        local upto,ret
        if t == '(' then
            t,name = get:next()
            upto = function(t,v) return t == ')' end
        else
            upto = function(t,v)
                return t == 'space' and v:find '\n'
            end
            -- return space following (returned by copy_tokens)
            ret = true
        end
        -- might be immediately followed by a parm list
        t,openp = get()
        if openp == '(' then
            parms = get:names()
        end
        -- the actual substitution is up to the end of the line
        local args, space = M.copy_tokens(get,upto)
        if scoped then
            M.define_scoped(name,args,parms)
        else
            M.set_macro(name,args,parms)
        end
        return ret and space[2]
    end
end

--- a macro for defining lexically scoped simple macros.
-- def_ may be followed by an arglist, and the substitution is the
-- rest of the line.
-- @usage def_ block (function() _END_CLOSE_
-- @usage def_ sqr(x) ((x)*(x))
-- @macro def_
M.define ('def_',macro_def(true))

--- a global version of `def_`.
-- @see def_
-- @macro define_
M.define ('define_',macro_def(false))

--- set the value of an existing macro.
-- the name and value follows immediately, and the value must be
-- a single token
-- @usage set_ T 'string'
-- @usage set_ F function
-- @macro set_
M.define('set_',function(get)
    local name = get:name()
    local t,v = get:next()
    M.set_macro(name,{{t,v}})
end)

--- undefining identifier macros.
-- @macro undef_
M.define('undef_',function(get)
    M.set_macro(get:name())
end)

--- Insert text after current block end. `_END_` is followed by a quoted string
-- and is used to insert that string after the current block closes.
-- @macro _END_
M.define ('_END_',function(get)
    local str = get:string()
    M.block_handler(-1,function(get,word)
        if word ~= 'end' then return nil,true end
        return str
    end)
end)

--- insert an end after the next closing block.
-- @macro _END_END_
-- @see _END_
M.define '_END_END_ _END_ " end"'

--- insert a closing parens after next closing block.
-- @usage def_ begin (function() _END_CLOSE_
-- fun begin ... end --> fun (function() ... end)
-- @macro _END_CLOSE_
-- @see _END_
M.define '_END_CLOSE_ _END_ ")"'

--- 'stringizing' macro.
-- Will convert its argument into a string.
-- @usage def_ _assert(x) assert(x,_STR_(x))
-- @macro _STR_
M.define('_STR_(x)',function(x)
    x = tostring(x)
    local put = M.Putter()
    return put '"':name(x) '"'
end)

-- macro stack manipulation


--- push a value onto a given macro' stack.
-- @macro _PUSH_
-- @param mac existing macro name
-- @param V a string
M.define('_PUSH_(mac,V)',function(mac,V)
    M.push_macro_stack(mac:get_string(),V:get_string())
end)

--- pop a value from a macro's stack.
-- @macro _POP_
-- @param mac existing macro name
-- @return a string
-- @see _PUSH_
M.define('_POP_',function(get,put)
    local val = M.pop_macro_stack(get:string())
    if val then
        return put(val)
    end
end)

--- drop the top of a macro's stack.
-- Like `_POP_`, except that it does not return the value
-- @macro _DROP_
-- @return existing macro name
-- @see _POP_
M.define('_DROP_',function(get)
    M.pop_macro_stack(get:string())
end)

--- Load a Lua module immediately. This allows macro definitions to
-- to be loaded before the rest of the file is parsed.
-- If the module returns a function, then this is assumed to be a
-- substitution function, allowing macro modules to insert code
-- at this point.
-- @macro require_
M.define('require_',function(get,put)
    local name = get:string()
    local ok,fn = pcall(require,name)
    if not ok then
        fn = require('macro.'..name)
    end
    if type(fn) == 'function' then
        return fn(get,put)
    end
end)

--- Include the contents of a file. This inserts the file directly
-- into the token stream, and is equivalent to cpp's `#include` directive.
-- @macro include_
M.define('include_',function(get)
    local str = get:string()
    local f = M.assert(io.open(str))
    local txt = f:read '*a'
    f:close()
    M.push_substitution(txt)
end)

