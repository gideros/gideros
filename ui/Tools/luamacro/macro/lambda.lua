--- Short anonymous functions (lambdas).
-- This syntax is suited
-- to any naive token-processor because the payload is always inside parens.
-- It is an example of a macro associated with a 'operator' character.
--
-- Syntax is `\<args>(<expr>)`
--
-- `\x(x+10)` is short for
-- `function(x) return x+10 end`. There may be a number of formal argumets,
-- e.g. `\x,y(x+y)` or there may be none, e.g. `\(somefun())`. Such functions
-- may return multiple values, e.g `\x(x+1,x-1)`.
--
-- @module macro.lambda

local M = require 'macro'

M.define ('\\',function(get,put)
	local args, body = get:idens('('), get:list()
	return put:keyword 'function' '(' : idens(args) ')' :
        keyword 'return' : list(body) : space() : keyword 'end'
end)

