---------------
-- A TokenList class for generating token lists.
--
-- There are also useful `get_` methods for extracting values from
-- the first token.
--
-- @module macro.TokenList

local TokenList = {}
local M = TokenList
TokenList.__index = TokenList

local append = table.insert

function TokenList.new (tl)
    return setmetatable(tl or {},TokenList)
end

local TokenListList = {}

function TokenList.new_list (ltl)
    return setmetatable(ltl or {},TokenListList)
end

TokenListList.__index = function(self,key)
    local m = TokenList[key]
    return function(self,...)
        local res = {}
        for i = 1,#self do res[i] = m(self[i],...) end
        return TokenList.new_list(res)
    end
end

-- token-getting helpers


local function extract (tl)
    local tk = tl[1]
    if tk[1] == 'space' then
        tk = tl[2]
    end
    return tk
end

--- get an identifier from front of a token list.
-- @return identifier name
function TokenList.get_iden (tl)
    local tk = extract(tl)
    M.assert(tk[1]=='iden','expecting identifier')
    return tk[2]
end

--- get an number from front of a token list.
-- @return number
function TokenList.get_number(tl)
    local tk = extract(tl)
    M.assert(tk[1]=='number','expecting number')
    return tonumber(tk[2])
end

--- get a string from front of a token list.
-- @return string value (without quotes)
function TokenList.get_string(tl)
    local tk = extract(tl)
    M.assert(tk[1]=='string')
    return tk[2]:sub(2,-2) -- watch out! what about long string literals??
end

--- takes a token list and strips spaces and comments.
-- @return new tokenlist
function TokenList.strip_spaces (tl)
    local out = TokenList.new()
    for _,t in ipairs(tl) do
        if t[1] ~= 'comment' and t[1] ~= 'space' then
            append(out,t)
        end
    end
    return out
end

--- pick the n-th token from this tokenlist.
-- Note that it returns the value and type, not the type and value.
-- @param n (1 to #self)
-- @return token value
-- @return token type
function TokenList.pick (tl,n)
    local t = tl[n]
    return t[2],t[1]
end

-- token-putting helpers
local comma,space = {',',','},{'space',' '}

--- append an identifier.
-- @param name the identifier
-- @param no_space true if you don't want a space after the iden
-- @return self
function TokenList.iden(res,name,no_space)
    append(res,{'iden',name})
    if not no_space then
        append(res,space)
    end
    return res
end

TokenList.name = TokenList.iden -- backwards compatibility!

--- append a string.
-- @param s the string
-- @return self
function TokenList.string(res,s)
    append(res,{'string','"'..s..'"'})
    return res
end

--- append a number.
-- @param val the number
-- @return self
function TokenList.number(res,val)
    append(res,{'number',val})
    return res
end

--- put out a list of identifiers, separated by commas.
-- @param res output token list
-- @param names a list of identifiers
-- @return self
function TokenList.idens(res,names)
    for i = 1,#names do
        res:iden(names[i],true)
        if i ~= #names then append(res,comma) end
    end
    return res
end

TokenList.names = TokenList.idens -- backwards compatibility!

--- put out a token list.
-- @param res output token list
-- @param tl a token list
-- @return self
function TokenList.tokens(res,tl)
    for j = 1,#tl do
        append(res,tl[j])
    end
    return res
end

--- put out a list of token lists, separated by commas.
-- @param res output token list
-- @param ltl a list of token lists
-- @return self
function TokenList.list(res,ltl)
    for i = 1,#ltl do
        res:tokens(ltl[i])
        if i ~= #ltl then append(res,comma) end
    end
    return res
end

--- put out a space token.
-- @param res output token list
-- @param space a string containing only whitespace (default ' ')
-- @return self
function TokenList.space(res,space)
    append(res,{'space',space or ' '})
    return res
end

--- put out a keyword token.
-- @param res output token list
-- @param keyw a Lua keyword
-- @param no_space true if you don't want a space after the iden
-- @return self
function TokenList.keyword(res,keyw,no_space)
    append(res,{'keyword',keyw})
    if not no_space then
        append(res,space)
    end
    return res
end

--- convert this tokenlist into a string.
function TokenList.__tostring(tl)
    local res = {}
    for j = 1,#tl do
        append(res,tl[j][2])
    end
    return table.concat(res)
end

--- put out a operator token. This is the overloaded call operator
-- for token lists.
-- @param res output token list
-- @param keyw an operator string
function TokenList.__call(res,t,v)
    append(res,{t,v or t})
    return res
end

return TokenList
