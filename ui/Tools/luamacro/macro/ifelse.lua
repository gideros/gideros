local M = require 'macro'

local function eval (expr,was_expr)
    expr = tostring(expr)
    if was_expr then expr = "return "..expr end
    local chunk = M.assert(loadstring(expr))
    local ok, res = pcall(chunk)
    if not ok then M.error("error evaluating "..res) end
    return res
end

local function eval_line (get,was_expr)
    local args = get:line()
    return eval(args,was_expr)
end

local function grab (get)
    local ilevel = 0
    while true do
        local t,v = get()
        while t ~= '@' do t = get() end
        t,v = get()
        if v == 'if' then
            ilevel = ilevel + 1
        else -- 'end','elseif','else'
            if ilevel > 0 and v == 'end' then
                ilevel = ilevel - 1
            elseif ilevel == 0 then return '@'..v end
        end
    end
end

M.define('@',function(get,put)
    local t,v = get()
--~     print('got',t,v)
    return put:iden(v..'_')
end)

local ifstack,push,pop = {},table.insert,table.remove

local function push_if (res)
--~     print 'push'
    push(ifstack, not (res==false or res==nil))
end

local function pop_if ()
--~     print 'pop'
    pop(ifstack)
end

M.define('if_',function(get)
    local res = eval_line(get,true)
    push_if(res)
    if not res then
        return grab(get)
    end
end)

M.define('elseif_',function(get)
    local res
    if ifstack[#ifstack] then
        res = false
    else
        res = eval_line(get,true)
        pop_if()
        push_if(res)
    end
    if not res then
        return grab(get)
    end
end)

M.define('else_',function(get)
    if #ifstack == 0 then M.error("mismatched else") end
    if ifstack[#ifstack] then
        return grab(get)
    end
end)

M.define('end_',function(get)
    pop_if()
end)

M.define('let_',function(get)
    eval_line(get)
end)

M.define('eval_(X)',function(X)
    return tostring(eval(X,true))
end)
