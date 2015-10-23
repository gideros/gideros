--[[--- A C lexical scanner using LPeg.
= CREDITS
= based on the C lexer in Peter Odding's lua-lxsh
@module macro.clexer
--]]

local clexer = {}
local lpeg = require 'lpeg'
local P, R, S, C, Cc, Ct = lpeg.P, lpeg.R, lpeg.S, lpeg.C, lpeg.Cc, lpeg.Ct

-- create a pattern which captures the lua value [id] and the input matching
-- [patt] in a table
local function token(id, patt) return Ct(Cc(id) * C(patt)) end

-- private interface
local table_of_tokens
local extra_tokens

function clexer.add_extra_tokens(extra)
    extra_tokens = extra_tokens or {}
    for _,t in ipairs(extra) do
        table.insert(extra_tokens,t)
    end
    table_of_tokens = nil -- re-initialize
end

function clexer.init ()
    local digit = R('09')

    local upp, low = R'AZ', R'az'
    local oct, dec = R'07', R'09'
    local hex      = dec + R'AF' + R'af'
    local letter   = upp + low
    local alnum    = letter + dec + '_'
    local endline  = S'\r\n\f'
    local newline  = '\r\n' + endline
    local escape   = '\\' * ( newline
                        + S'\\"\'?abfnrtv'
                        + (#oct * oct^-3)
                        + ('x' * #hex * hex^-2))


    -- range of valid characters after first character of identifier
    local idsafe = R('AZ', 'az', '\127\255') + P '_'

    -- operators
    local OT = P '=='
    if extra_tokens then
        for _,ex in ipairs(extra_tokens) do
            OT = OT + P(ex)
        end
    end
    local operator = token('operator', OT + P '.' + P'>>=' + '<<=' + '--' + '>>' + '>=' + '/=' + '==' + '<='
    + '+=' + '<<' + '*=' + '++' + '&&' + '|=' + '||' + '!=' + '&=' + '-='
    + '^=' + '%=' + '->' + S',)*%+&(-~/^]{}|.[>!?:=<;')
    -- identifiers
    local ident = token('iden', idsafe * (idsafe + digit) ^ 0)

    -- keywords
    local keyword = token('keyword', (P 'auto' + P 'break' + P 'case' + P'char' +
        P 'const' + P 'continue' + P 'default' +
        P 'do' + P 'double' + P 'else' + P 'enum' + P 'extern' + P 'float' +
        P 'for' + P 'goto' + P 'if' + P 'int' + P 'long' + P 'register' +
        P 'return' + P 'short' + P 'signed' + P 'sizeof' + P 'static' +
        P 'struct' + P 'switch' + P 'typedef' + P 'union' + P 'void' +
        P 'volatile' + P 'while') * -(idsafe + digit))

    -- numbers
    local number_sign = S'+-'^-1
    local number_decimal = digit ^ 1
    local number_hexadecimal = P '0' * S 'xX' * R('09', 'AF', 'af') ^ 1
    local number_float = (digit^1 * P'.' * digit^0 + P'.' * digit^1) *
                         (S'eE' * number_sign * digit^1)^-1
    local number = token('number', number_hexadecimal +
                                   number_float +
                                   number_decimal)


    local string = token('string', '"' * ((1 - S'\\\r\n\f"') + escape)^0 * '"')
    local char = token('char',"'" * ((1 - S"\\\r\n\f'") + escape) * "'")

    -- comments
    local singleline_comment = P '//' * (1 - S '\r\n\f') ^ 0
    local multiline_comment = '/*' * (1 - P'*/')^0 * '*/'
    local comment = token('comment', multiline_comment + singleline_comment)
    local prepro = token('prepro',P '#' * (1 - S '\r\n\f') ^ 0)

    -- whitespace
    local whitespace = token('space', S('\r\n\f\t ')^1)

    -- ordered choice of all tokens and last-resort error which consumes one character
    local any_token = whitespace + number + keyword + ident +
                      string + char + comment + prepro + operator + token('error', 1)


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
clexer.sync = sync

clexer.line = 0

-- we only need to synchronize the line-counter for these token types
local multiline_tokens = { comment = true, space = true }
clexer.multiline_tokens = multiline_tokens

function clexer.scan_c_tokenlist(input)
    if not table_of_tokens then
        clexer.init()
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
-- S  is the source - can be a string or a file-like object (i.e. read() returns line)
-- Note that this token iterator includes spaces and comments, and does not convert
-- string and number tokens - so e.g. a string token is quoted and a number token is
-- an unconverted string.
function clexer.scan_c(input,name)
    if type(input) ~= 'string' and input.read then
        input = input:read('*a')
    end
    local tokens = clexer.scan_c_tokenlist(input)
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
            clexer.line = tok[3]
            clexer.name = name
            return tok[1],tok[2]
        end
    end

end

return clexer
