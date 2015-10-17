--[[--- A Lua lexical scanner using LPeg.
= CREDITS
Written by Peter Odding, 2007/04/04

= THANKS TO
- the Lua authors for a wonderful language;
- Roberto for LPeg;
- caffeine for keeping me awake :)

= LICENSE
Shamelessly ripped from the SQLite[3] project:

   The author disclaims copyright to this source code.  In place of a legal
   notice, here is a blessing:

      May you do good and not evil.
      May you find forgiveness for yourself and forgive others.
      May you share freely, never taking more than you give.

@module macro.lexer
--]]

local lexer = {}
local lpeg = require 'lpeg'
local P, R, S, C, Cb, Cc, Cg, Cmt, Ct =
      lpeg.P, lpeg.R, lpeg.S, lpeg.C, lpeg.Cb, lpeg.Cc, lpeg.Cg, lpeg.Cmt, lpeg.Ct

-- create a pattern which captures the lua value [id] and the input matching
-- [patt] in a table
local function token(id, patt) return Ct(Cc(id) * C(patt)) end

-- private interface
local table_of_tokens
local extra_tokens

function lexer.add_extra_tokens(extra)
    extra_tokens = extra_tokens or {}
    for _,t in ipairs(extra) do
        table.insert(extra_tokens,t)
    end
    table_of_tokens = nil -- re-initialize
end

function lexer.init ()
    local digit = R('09')

    -- range of valid characters after first character of identifier
    --local idsafe = R('AZ', 'az', '\127\255') + P '_'
    local idsafe = R('AZ', 'az') + P '_' + R '\206\223' * R '\128\255'
    -- operators
    local OT = P '=='
    if extra_tokens then
        for _,ex in ipairs(extra_tokens) do
            OT = OT + P(ex)
        end
    end
    local operator = token('operator', OT + P '.' + P '~=' + P '<=' + P '>=' + P '...'
                                              + P '..' + S '+-*/%^#=<>;:,.{}[]()')
    -- identifiers
    local ident = token('iden', idsafe * (idsafe + digit) ^ 0)

    -- keywords
    local keyword = token('keyword', (P 'and' + P 'break' + P 'do' + P 'elseif' +
       P 'else' + P 'end' + P 'false' + P 'for' + P 'function' + P 'if' +
       P 'in' + P 'local' + P 'nil' + P 'not' + P 'or' + P 'repeat' + P 'return' +
       P 'then' + P 'true' + P 'until' + P 'while') * -(idsafe + digit))

    -- numbers
    local number_sign = S'+-'^-1
    local number_decimal = digit ^ 1
    local number_hexadecimal = P '0' * S 'xX' * R('09', 'AF', 'af') ^ 1
    local number_float = (digit^1 * P'.' * digit^0 + P'.' * digit^1) *
                         (S'eE' * number_sign * digit^1)^-1
    local number = token('number', number_hexadecimal +
                                   number_float +
                                   number_decimal)

    -- callback for [=[ long strings ]=]
    -- ps. LPeg is for Lua what regex is for Perl, which makes me smile :)
    local equals  = P '=' ^ 0
    local open    = P '[' * Cg(equals, "init") * P '[' * P '\n' ^ -1
    local close   = P ']' * C(equals) * P ']'
    local closeeq = Cmt(close * Cb "init", function (s, i, a, b) return a == b end)
    local longstring = open * C((P(1) - closeeq)^0) * close --/ 1

    -- strings
    local singlequoted_string = P "'" * ((1 - S "'\r\n\f\\") + (P '\\' * 1)) ^ 0 * "'"
    local doublequoted_string = P '"' * ((1 - S '"\r\n\f\\') + (P '\\' * 1)) ^ 0 * '"'
    local string = token('string', singlequoted_string +
                                   doublequoted_string +
                                   longstring)

    -- comments
    local singleline_comment = P '--' * (1 - S '\r\n\f') ^ 0
    local multiline_comment = P '--' * longstring
    local comment = token('comment', multiline_comment + singleline_comment)

    -- whitespace
    local whitespace = token('space', S('\r\n\f\t ')^1)

    -- ordered choice of all tokens and last-resort error which consumes one character
    local any_token = whitespace + number + keyword + ident +
                      string + comment + operator + token('error', 1)


    table_of_tokens = Ct(any_token ^ 0)
end

-- increment [line] by the number of line-ends in [text]
local function sync(line, text)
   local index, limit = 1, #text
   while index <= limit do
      local start, stop = text:find('\r\n', index, true)
      if not start then
         start, stop = text:find('[\r\n\f]', index)
         if not start then break end
      end
      index = stop + 1
      line = line + 1
   end
   return line
end
lexer.sync = sync

lexer.line = 0

-- we only need to synchronize the line-counter for these token types
local multiline_tokens = { comment = true, string = true, space = true }
lexer.multiline_tokens = multiline_tokens

function lexer.scan_lua_tokenlist(input)
    if not table_of_tokens then
        lexer.init()
    end
    assert(type(input) == 'string', 'bad argument #1 (expected string)')
    local line = 1
    local tokens = lpeg.match(table_of_tokens, input)
    for i, token in pairs(tokens) do
        local t = token[1]
        if t == 'operator' or t == 'error' then
            token[1] = token[2]
        end
        token[3] = line
        if multiline_tokens[t] then
            line = sync(line, token[2])
        end
    end
    return tokens
end

--- get a token iterator from a source containing Lua code.
-- Note that this token iterator includes spaces and comments, and does not convert
-- string and number tokens - so e.g. a string token is quoted and a number token is
-- an unconverted string.
-- @param input the source - can be a string or a file-like object (i.e. read() returns line)
-- @param name for the source
function lexer.scan_lua(input,name)
    if type(input) ~= 'string' and input.read then
        input = input:read('*a')
    end
    local tokens = lexer.scan_lua_tokenlist(input)
    local i, n = 1, #tokens
    return function(k)
        if k ~= nil then
            k = i + k
            if k < 1 or k > n then return nil end
            return tokens[k]
        end
        local tok = tokens[i]
        i = i + 1
        if tok then
            lexer.line = tok[3]
            lexer.name = name
            return tok[1],tok[2]
        end
    end
end

return lexer
