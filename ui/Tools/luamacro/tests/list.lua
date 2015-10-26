local M = require 'macro'
List = require 'pl.List'

local list_check

-- list <var-list> [ = <init-list> ]
-- acts as a 'macro factory', making locally-scoped macros for the variables,
-- and emitting code to initialize plain variables.
M.define ('list',function(get)
    get() -- skip space
    -- 'list' acts as a 'type' followed by a variable list, which may be
    -- followed by initial values
    local values
    local vars,endt = get:idens (function(t,v)
        return t == '=' or (t == 'space' and v:find '\n')
    end)
    -- there is an initialization list
    if endt[1] == '=' then
        values,endt = get:list '\n'
    else
        values = {}
    end
    -- build up the initialization list
    for i,name in ipairs(vars) do
       M.define_scoped(name,list_check)
       values[i] = 'List('..tostring(values[i] or '')..')'
    end
    local lcal = M._interactive and '' or 'local '
    local res = lcal..table.concat(vars,',')..' = '..table.concat(values,',')..tostring(endt)
    return res
end)

function list_check (get,put)
    local t,v = get:peek(1)
    if t ~= '[' then return nil, true end -- pass-through; plain var reference
    get:expecting '['
    local args = get:list(']',':')
    -- it's just plain table access
    if #args == 1 then return '['..tostring(args[1])..']',true end

    -- two items separated by a colon; use sensible defaults
    M.assert(#args == 2, "slice has two arguments!")
    local start,finish = tostring(args[1]),tostring(args[2])
    if start == '' then start = '1' end
    if finish == '' then finish = '-1' end

    -- look ahead to see if we're on the left hand side of an assignment
    if get:peek(1) == '=' then
       get:next() -- skip '='
       local rest,eoln = get:upto '\n'
       rest,eoln = tostring(rest),tostring(eoln)
       return (':slice_assign(%s,%s,%s)%s'):format(start,finish,rest,eoln),true
    else
        return (':slice(%s,%s)'):format(start,finish),true
    end
end



