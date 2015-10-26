local M = require 'macro'

local concat = table.concat

M.define ('Tab',function(get)
    get() -- skip space
    -- 'Tab' acts as a 'type' followed by a variable list
    local vars,endt = get:idens (function(t,v)
        return t == '=' or (t == 'space' and v:find '\n')
    end)
    local values = {}
    for i,name in ipairs(vars) do
        M.define_scoped(name,function(get,put)
            local t,v = get:peek(1)
            if t ~= '[' then return nil, true end -- pass-through; plain var reference
            get:expecting '['
            local args = get:list(']','')
            local index = args[1]
            for i = 1,#index do
                if index[i][1] == '#' and (i == #index or index[i+1][1] ~= 'iden') then
                    table.insert(index,i+1,{'iden',name})
                end
            end
            return '['..tostring(index)..']',true
        end)
        values[i] = '{}'
    end
    local lcal = M._interactive and '' or 'local '
    local res = lcal..concat(vars,',')..' = '..concat(values,',')..tostring(endt)
    return res
end)



