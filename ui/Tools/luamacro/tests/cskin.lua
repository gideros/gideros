-- cskin.lua
local M = require 'macro'

M.define_tokens{'&&','||','!=','{|'}

M.define '&& and'
M.define '|| or'
M.define '!= ~='
M.define '! not'

local push,pop = table.insert,table.remove
local bstack = {}
local btop = {}

local function push_brace_stack (newv)
    newv = newv or {}
    newv.lev = 0
    push(bstack,btop)
    btop = newv
end

local disabled = false

-- skinning can be temporarily disabled; this macro re-enables skinning
M.define('_SKIN_',function() disabled = false end)

local function block_statement (word,end_handler)
    return function(get,put)
        if disabled then return end
        get:expecting '('
        local stuff = get:upto ')'
        get:expecting '{'
        push_brace_stack {handler = end_handler}
        return put:space():tokens(stuff):space():keyword(word)
    end
end

local function endif_handler (get,put)
    local t,v = get:next()
    if t == 'keyword' and v == 'else' then
        local nt,nv = get:next()
        if nt == 'keyword' and nv == 'if' then
            return put:keyword 'elseif'
        else
            M.assert(nt == '{')
        end
        push_brace_stack() -- opening else block
        return put:keyword 'else'
    else
        return put:keyword 'end' (t,v)
    end
end

M.keyword_handler ('for',block_statement 'do')
M.keyword_handler ('while',block_statement 'do')
M.keyword_handler ('if',block_statement ('then',endif_handler))
M.keyword_handler ('elseif',block_statement ('then',endif_handler))

M.define('{',function()
    if btop.lev then
        btop.lev = btop.lev + 1
    end
    return nil,true --> pass-through macro
end)

M.define('}',function(get,put)
    if btop.lev == 0 then
        local res
        if btop.handler then res = btop.handler(get,put) end
        if not res then res = put:space():keyword'end' end
        btop = pop(bstack)
        return res
    else
        btop.lev = btop.lev - 1
        return nil,true --> pass-through macro
    end
end)

-- def for function is a little easier; it's a plain macro,
-- and we deliberately don't want to get rid of the parens.
M.define('def',function(get,put)
    local stuff = get:list('{','')
    put:keyword('function')
    if btop.classname then put:iden(btop.classname) '.' end
    push_brace_stack()
    return put:list(stuff)
end)

----- OOP support ------
M.define('class',function(get,put)
    local name,t,v = get:iden(),get:next()
    local base = ''
    if t == ':' then base = get:iden(); t = get:next() end
    M.assert(t == '{','expecting {')
    push_brace_stack {classname = name}
    return 'do '..name..' = class_('..base..')'
end)

class_ = require 'macro.lib.class'

---- making an existing macro play nicely with the C skin ---
require 'macro.forall'

local fun = M.get_macro_value 'forall'
M.define('forall',function (get,put)
    get:expecting '('
    local stuff = get:upto ')'
    get:expecting'{'
    stuff:space():keyword 'do'
    stuff = fun(get.from_tl(stuff),put)
    -- let the unskinned Lua pass through with _SKIN_ to re-enable skinning
    disabled = true
    stuff:iden '_SKIN_':space()
    push_brace_stack()
    return stuff
end)

-- often easier to express the macro in a skinned form
M.define('{|',function(get,put)
    local expr = tostring(get:upto '|')
    local select = tostring(get:upto '}')
    return ('(def(){local r={}; forall(%s) {r[#r+1]=%s} return r})()'):format(select,expr)
end)




