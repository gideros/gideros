--- a simple testing framework.
-- Defines a single statment macro assert_ which has the following syntax:
--
--  - assert_ val1 == val2
--  - assert_ val1 > val2
--  - assert_ val1 < val2
--  - assert_ val1 matches val2 (using string matching)
--  - assert_ val1 throws val2  (ditto, on exception string)
--
-- The `==` case has some special forms. If `val2` is `(v1,v2,..)` then
-- it's assumed that the expression `val1` returns multiple values. `==` will
-- also do value equality for plain tables.  If `val2` is a number given in
-- %f format (such as 3.14) then it will match `vall` up to that specified
-- number of digits.
--
--     assert_ {one=1,two=2} == {two=2,one=1}
--     assert_ 'hello' matches '^hell'
--     assert_ 2 > 1
--     assert_ ('hello'):find 'll' == (3,4)
--     assert_ a.x throws 'attempt to index global'
-- @module macro.assert

local M = require 'macro'
local relop = {
    ['=='] = 'eq',
    ['<'] = 'lt',
    ['>'] = 'gt'
}

local function numfmt (x)
    local int,frac = x:match('(%d+)%.(%d+)')
    if not frac then return nil end
    return '%'..#x..'.'..#frac..'f', x
end

--- assert that two values match the desired relation.
-- @macro assert_
M.define('assert_',function(get,put)
    local testx,tok = get:upto(function(t,v)
        return relop[t] or (t == 'iden' and (v == 'matches' or v == 'throws'))
    end)
    local testy,eos = get:line()
    local otesty = testy
    testx = tostring(testx)
    testy = tostring(testy)
    local t,v,op = tok[1],tok[2]
    if relop[t] then
        op = relop[t]
        if t == '==' then
            if testy:match '^%(.+%)$' then
                testx = 'T_.tuple('..testx..')'
                testy = 'T_.tuple'..testy
            elseif #otesty == 1 and otesty[1][1] == 'number' then
                local num = otesty[1][2]
                local fmt,num = numfmt(num)
                if fmt then -- explicit floating-point literal
                    testy = '"'..num..'"'
                    testx = '("'..fmt..'"):format('..testx..')'
                    op = 'match'
                end
            end
        end
    elseif v == 'matches' then
        op = 'match'
    elseif v == 'throws' then
        op = 'match'
        testx = 'T_.pcall_no(function() return '..testx..' end)'
    end
    return ('T_.assert_%s(%s,%s)%s'):format(op,testx,testy,tostring(eos))
end)

return function()
    return "T_ = require 'macro.lib.test'"
end
