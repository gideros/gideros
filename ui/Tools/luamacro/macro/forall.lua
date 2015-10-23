--------------------
-- `forall` statement.
-- The syntax is `forall VAR SELECT [if CONDN] do` where
-- `SELECT` is either `in TBL` or `= START,FINISH`
--
-- For example,
--
--    forall name in {'one','two'} do print(name) end
--
--    forall obj in get_objects() if obj:alive() then
--        obj:action()
--    end
--
-- Using `forall`, we also define _list comprehensions_ like
-- `L{s:upper() | s in names if s:match '%S+'}`
--
-- @module macro.forall

local M = require 'macro'

--- extended for statement.
-- @macro forall
M.define('forall',function(get,put)
    local var = get:iden()
    local t,v = get:next()
    local rest,endt = get:list(M.upto_keywords('do','if'))
    put:keyword 'for'
    if v == 'in' then
        put:iden '_' ',' :iden(var):keyword 'in'
        put:iden 'ipairs' '(' :list(rest) ')'
    elseif v == '=' then
        put:iden(var) '=' :list(rest)
    else
        M.error("expecting in or =")
    end
    put:keyword 'do'
    if endt[2] == 'if' then
        rest,endt = get:list(M.upto_keywords('do'))
        put:keyword 'if':list(rest):keyword 'then':iden '_END_END_'
    end
    return put
end)

--- list comprehension.
-- Syntax is `L{expr | select}` where `select` is as in `forall`,
-- or `L{expr for select}` where `select` is as in the regular `for` statement.
-- @macro L
-- @return a list of values
-- @usage L{2*x | x in {1,2,3}} == {1,4,9}
-- @usage L{2*x|x = 1,3} == {1,4,9}
-- @usage L{{k,v} for k,v in pairs(t)}
-- @see forall
M.define('L',function(get,put)
    local t,v = get:next() -- must be '{'
    local expr,endt = get:list(function(t,v)
        return t == '|' or t == 'keyword' and v == 'for'
    end,'')
    local select = get:list('}','')
    put '(' : keyword 'function' '(' ')' :keyword 'local':iden 'res' '=' '{' '}'
    if endt[2] == '|' then
        put:iden'forall'
    else
        put:keyword 'for'
    end
    put:list(select):space():keyword'do'
    put:iden'res' '[' '#' :iden'res' '+' :number(1) ']' '=' :list(expr):space()
    put:keyword 'end' :keyword 'return' : iden 'res' :keyword 'end' ')' '(' ')'
    put:iden '_POP_':string'L'
    return put
end)
