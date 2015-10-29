--- A try/except block.
-- This generates syntactical sugar around `pcal`l, and correctly
-- distinguishes between the try block finishing naturally and
-- explicitly using 'return' with no value. This is handled by
-- converting any no value `return` to `return nil`.
--
-- Apart from the usual creation of a closure, this uses a table
-- to capture all the results. Not likely to win speed contests,
-- but intended to be correct.
-- @module macro.try

local M = require 'macro'

local function pack (...)
    local args = {...}
    args.n = select('#',...)
    return args
end

function pcall_(fn,...)
    return pack(pcall(fn,...))
end

local function check_return_value(get,put)
    local t,v = get:next()
    put:space()
    if t=='keyword' and (v=='end' or v=='else' or v=='until') then
        put:keyword 'nil'
    end
    return put(t,v)
end


M.define('RR_',M.make_scoped_handler('return',check_return_value))


--- A try macro, paired with except.
--
--     try
--      maybe_something_bad()
--     except (e)
--      print(e)
--     end
-- @macro try
M.define 'try do local r_ =  pcall_(function() RR_ '
M.define 'except(e) end); if r_[1] then if r_.n > 1 then return unpack(r_,2,r_.n) end else local e = r_[2] _END_END_  '

