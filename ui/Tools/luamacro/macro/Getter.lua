--- Getter class. Used to get values from the token stream. The first
-- argument `get` of a macro substitution function is of this type.
--
--    M.define ('\\',function(get,put)
--        local args, body = get:idens('('), get:list()
--        return put:keyword 'function' '(' : idens(args) ')' :
--             keyword 'return' : list(body) : space() : keyword 'end'
--    end)
--
-- The second argument `put` is a `TokenList` object.
-- @see macro.TokenList
-- @module macro.Getter

local TokenList = require 'macro.TokenList'
local append = table.insert
local setmetatable = setmetatable

local Getter = {
    __call = function(self)
        return self.fun()
    end
}
local M = Getter

Getter.__index = Getter;

local scan_iter

function Getter.new (get)
    return setmetatable({fun=get},Getter)
end

function Getter.from_tl(tl)
    return Getter.new(scan_iter(tl))
end

local Tok = {
    __tostring = function(self)
        return self[2]
    end
}

local function tok_new (t)
    return setmetatable(t,Tok)
end

-- create a token iterator out of a token list
function Getter.scan_iter (tlist)
    local i,n = 1,#tlist
    return function(k)
        if k ~= nil then
            k = i + k
            if k < 1 or k > n then return nil end
            return tlist[k]
        end
        local tv = tlist[i]
        if tv == nil then return nil end
        i = i + 1
        return tv[1],tv[2]
    end
end

scan_iter = Getter.scan_iter

--- get the next non-whitespace token.
-- @return token type
-- @return token value
-- @function Getter.next
function Getter.next(get)
    local t,v = get()
    while t == 'space' or t == 'comment' do
        t,v = get()
    end
    return t,v
end

local TL,LTL = TokenList.new, TokenList.new_list


local function tappend (tl,t,val)
    val = val or t
    append(tl,{t,val})
end

--- get a balanced block.
-- Typically for grabbing up to an `end` keyword including any nested
-- `if`, `do` or `function` blocks with their own `end` keywords.
-- @param tok the token stream
-- @param begintokens set of tokens requiring their own nested *endtokens*
--   (default: `{['do']=true,['function']=true,['if']=true}`)
-- @param endtokens set of tokens ending a block (default:`{['end']=true}`)
-- @return list of tokens
-- @return block end token in form `{type,value}`
-- @usage
--   -- copy a balanced table constructor
--   get:expecting '{'
--   put '{':tokens (get:block ({['{']=true}, {['}']=true}) '}')
function Getter.block(tok,begintokens,endtokens)
    begintokens = begintokens or {['do']=true,['function']=true,['if']=true}
    endtokens = endtokens or {['end']=true}
    local level = 1 -- used to count expected matching `endtokens`
    local tl = TL()
    local token,value
    repeat
        token,value = tok()
        if not token then return nil,'unexpected end of block' end
        if begintokens[value] then
	    level = level + 1
        elseif endtokens[value] then
	    level = level - 1
        end
	if level > 0 then  -- final end token is returned separately
            tappend(tl,token,value)
	end
    until level == 0
    return tl,tok_new{token,value}
end

--- get a delimited list of token lists.
-- Typically used for grabbing argument lists like ('hello',a+1,fred(c,d)); will count parens
-- so that the delimiter (usually a comma) is ignored inside sub-expressions. You must have
-- already read the start token of the list, e.g. open parentheses. It will eat the end token
-- and return the list of TLs, plus the end token. Based on similar code in Penlight's
-- `pl.lexer` module.
-- @param tok the token stream
-- @param endt the end token (default ')')
-- @param delim the delimiter (default ',')
-- @return list of token lists
-- @return end token in form {type,value}
function Getter.list(tok,endtoken,delim)
    endtoken = endtoken or ')'
    delim = delim or ','
    local parm_values = LTL()
    local level = 1 -- used to count ( and )
    local tl = TL()
    local is_end
    if type(endtoken) == 'function' then
        is_end = endtoken
    elseif endtoken == '\n' then
        is_end = function(t,val)
            return t == 'space' and val:find '\n'
        end
    else
        is_end = function (t)
            return t == endtoken
        end
    end
    local token,value = tok()
    if is_end(token,value) then return parm_values end
    if token == 'space' then
        token,value = tok()
    end
    while true do
        if not token then return nil,'unexpected end of list' end -- end of stream is an error!
        if is_end(token,value) and level == 1 then
            append(parm_values,tl)
            break
        elseif token == '(' then
            level = level + 1
            tappend(tl,'(')
        elseif token == ')' then
            level = level - 1
            if level == 0 then -- finished with parm list
                append(parm_values,tl)
                break
            else
                tappend(tl,')')
            end
        elseif token == '{' then
            level = level + 1
            tappend(tl,'{')
        elseif token == '}' then
            level = level - 1
            tappend(tl,'}')
        elseif token == delim and level == 1 then
            append(parm_values,tl) -- a new parm
            tl = TL()
        else
            tappend(tl,token,value)
        end
        token,value=tok()
    end
    return parm_values,tok_new{token,value}
end

function Getter.upto_keywords (k1,k2)
    return function(t,v)
        return t == 'keyword' and (v == k1 or v == k2)
    end,''
end

local tnext = Getter.next


function Getter.upto(tok,k1,k2)
    local endt = k1
    if type(k1) == 'string' and k1:match '^%a+$' then
        endt = Getter.upto_keywords(k1,k2)
    end
    local ltl,tok = tok:list(endt,'')
    M.assert(ltl ~= nil and #ltl > 0,'failed to grab tokens')
    return ltl[1],tok
end

function Getter.line(tok)
    return tok:upto(function(t,v)
        return (t=='space' and v:match '\n') or t == 'comment'
    end)
end


local function prettyprint (t, v)
    v = v:gsub ("\n", "\\n")
    if t == "string" then
        if #v > 16 then v = v:sub(1,12).."..."..v:sub(1,1) end
        return t.." "..v
    end
    if #v > 16 then v = v:sub(1,12).."..." end
    if t == "space" or t == "comment" or t == "keyword" then
        return t.." '"..v.."'"
    elseif t == v then
        return "'"..v.."'"
    else
        return t.." "..v
    end
end

--- get the next identifier token.
-- (will be an error if the token has wrong type)
-- @return identifier name
function Getter.iden(tok)
    local t,v = tnext(tok)
    M.assert(t == 'iden','expecting identifier, got '..prettyprint(t,v))
    return v
end

Getter.name = Getter.iden -- backwards compatibility!

--- get the next number token.
-- (will be an error if the token has wrong type)
-- @return converted number
function Getter.number(tok)
    local t,v = tnext(tok)
    M.assert(t == 'number','expecting number, got '..prettyprint(t,v))
    return tonumber(v)
end

--- get a delimited list of identifiers.
-- works like list.
-- @param tok the token stream
-- @param endt the end token (default ')')
-- @param delim the delimiter (default ',')
-- @see list
function Getter.idens(tok,endt,delim)
    local ltl,err = tok:list(endt,delim)
    if not ltl then error('idens: '..err) end
    local names = {}
    -- list() will return {{}} for an empty list of tlists
    for i = 1,#ltl do
	local tl = ltl[i]
        local tv = tl[1]
        if tv then
            if tv[1] == 'space' then tv = tl[2] end
            names[i] = tv[2]
        end
    end
    return names, err
end

Getter.names = Getter.idens -- backwards compatibility!

--- get the next string token.
-- (will be an error if the token has wrong type)
-- @return string value (without quotes)
function Getter.string(tok)
    local t,v = tok:expecting("string")
    return v:sub(2,-2)
end

--- assert that the next token has the given type. This will throw an
-- error if the next non-whitespace token does not match.
-- @param type a token type ('iden','string',etc)
-- @param value a token value (optional)
-- @usage  get:expecting '('
-- @usage  get:expecting ('iden','bonzo')
function Getter.expecting (tok,type,value)
    local t,v = tnext(tok)
    if t ~= type then M.error ("expected "..type.." got "..prettyprint(t,v)) end
    if value then
        if v ~= value then M.error("expected "..value.." got "..prettyprint(t,v)) end
    end
    return t,v
end

--- peek ahead or before in the token stream.
-- @param k positive delta for looking ahead, negative for looking behind.
-- @param dont_skip true if you want to check for whitespace
-- @return the token type
-- @return the token value
-- @return the token offset
-- @function Getter.peek

--- peek ahead two tokens.
-- @return first token type
-- @return first token value
-- @return second token type
-- @return second token value
-- @function Getter.peek2

--- patch the token stream at the end.
-- @param idx index in output table
-- @param text to replace value at that index
-- @function Getter.patch

--- put out a placeholder for later patching.
-- @param put a putter object
-- @return an index into the output table
-- @function Getter.placeholder

return Getter
