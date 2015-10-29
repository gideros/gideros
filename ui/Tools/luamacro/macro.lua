----------------------------------------------
-- LuaMacro 2, a macro-preprocessor for Lua.
-- Unlike LuaMacro 1.x, it does not depend on the token-filter patch and generates
-- Lua code which can be printed out or compiled directly. C-style macros are easy, but LM2
-- allows for macros that can read their own input and generate output using Lua code.
-- New in this release are lexically-scoped macros.
-- The Lua Lpeg Lexer is by Peter Odding.
--
-- Examples:
--
--     macro.define 'sqr(x) ((x)*(x))'
--     macro.define 'assert_(expr) assert(expr,_STR_(expr))'
--     macro.define('R(n)',function(n)
--       n = n:get_number()
--       return ('-'):rep(n)
--     end
--     macro.define('lazy',function(get)
--        get() -- skip space
--        local expr,endt = get:upto(function(t,v)
--            return t == ',' or t == ')' or t == ';'
--              or (t=='space' and v:match '\n')
--        end)
--        return 'function(_) return '..tostring(expr)..' end'..tostring(endt)
--     end)
--
--
-- @author Steve Donovan
-- @copyright 2011
-- @license MIT/X11
-- @module macro
-- @alias M

local macro = {}
local M = macro
local lexer = require 'macro.lexer'
local Getter = require 'macro.Getter'
local TokenList = require 'macro.TokenList'
local scan_code = lexer.scan_lua
local append = table.insert
local setmetatable = setmetatable

--local tdump = require 'pl.pretty'.dump

local scan_iter, tnext = Getter.scan_iter, Getter.next


M.upto_keywords = Getter.upto_keywords
M.Putter = TokenList.new

-- given a token list, a set of formal arguments and the actual arguments,
-- return a new token list where the formal arguments have been replaced
-- by the actual arguments
local function substitute_tokenlist (tl,parms,args)
    local append,put_tokens = table.insert,TokenList.tokens
    local parm_map = {}
    for i,name in ipairs(parms) do
        parm_map[name] = args[i]
    end
    local res = {}
    for _,tv in ipairs(tl) do
        local t,v = tv[1],tv[2]
        if t == 'iden' then
            local pval = parm_map[v]
            if pval then
                put_tokens(res,pval)
            else
                append(res,tv)
            end
        else
            append(res,tv)
        end
    end
    return res
end

----------------
-- Defining and Working with Macros.
-- @section macros

--- make a copy of a list of tokens.
-- @param tok the list
-- @param pred copy up to this condition; if defined, must be a function
-- of two arguments, the token type and the token value.
-- @return the copy
-- @return the token that matched the predicate
function M.copy_tokens(tok,pred)
    local res = {}
    local t,v = tok()
    while t and not (pred and pred(t,v)) do
        append(res,{t,v})
        t,v = tok()
    end
    return res,{t,v}
end

---- define new lexical tokens.
-- @param extra a list of strings defining the new tokens
-- @usage macro.define_tokens{'{|'}
function M.define_tokens(extra)
    lexer.add_extra_tokens(extra)
end

local imacros,smacros = {},{}

M.macro_table = imacros

--- define a macro using a specification string and optional function.
-- The specification looks very much like a C preprocessor macro: the name,
-- followed by an optional formal argument list (_no_ space after name!) and
-- the substitution. e.g `answer 42` or `sqr(x) ((x)*(x))`
--
-- If there is no substitution, then the second argument must be a function which
-- will be evaluated for the actual substitution. If there are explicit parameters, then they will be passed as token lists. Otherwise, the function is passed a `get` and a `put` argument, which are `Getter` and `TokenList` objects.
--
-- The substitution function may return a `TokenList` object, or a string.
-- @param macstr
-- @param subst_fn the optional substitution function
-- @see macro.Getter, macro.TokenList
function M.define(macstr,subst_fn)
    local tok,t,macname,parms,parm_map
    local mtbl
    tok = scan_code(macstr)
    t,macname = tok()
    if t == 'iden' then mtbl = imacros
    elseif t ~= 'string' and t ~= 'number' and t ~= 'keyword' then
        mtbl = smacros
    else
        error("a macro cannot be of type "..t)
    end
    t = tok()
    if t == '(' then
        parms = Getter.new(tok):idens()
    end
    mtbl[macname] = {
        name = macname,
        subst = subst_fn or M.copy_tokens(tok),
        parms = parms
    }
end

--- define a macro using a function and a parameter list.
-- @param name either an identifier or an operator.
-- @param subst a function
-- @param parms a list of parameter names
-- @return the existing value of this macro, if any
function M.set_macro(name,subst,parms)
    local macros
    if name:match '^[_%a][_%w]*$' then
        macros = imacros
    else
        macros = smacros
    end
    if subst == nil then
        macros[name] = nil
        return
    end
    local last = macros[name]
    if type(subst) ~= 'table' or not subst.name then
        subst = {
            name = name,
            subst = subst,
            parms = parms
        }
    end
    macros[name] = subst
    return last
end

--- defined a scoped macro. Like define except this macro will not
-- be visible outside the current scope.
-- @param name either an identifier or an operator.
-- @param subst a function
-- @param parms a list of parameter names
-- @see set_macro
function M.define_scoped (name,subst,parms)
    local old_value = M.set_macro(name,subst,parms)
    M.block_handler(-1,function()
        M.set_macro(name,old_value)
    end)
end

--- get the value of a macro. The macro substitution must either be a
-- a string or a single token.
-- @param name existing macro name
-- @return a string value, or nil if the macro does not exist.
function M.get_macro_value(name)
    local mac = imacros[name]
    if not mac then return nil end
    if type(mac.subst) == 'table' then
        return mac.subst[1][2]
    else
        return mac.subst
    end
end

local function get_macro (mac, no_error)
    local macro = imacros[mac]
    if not macro and not no_error then
        M.error("macro "..mac.." is not defined")
    end
    return macro
end

local push,pop = table.insert,table.remove

--- push a value on the stack associated with a macro.
-- @param name macro name
-- @param value any value
function M.push_macro_stack (name,value)
    local macro = get_macro(name)
    macro.stack = macro.stack or {}
    push(macro.stack,value)
end

--- pop a value from a macro stack.
-- @param name macro name
-- @return any value, if defined
function M.pop_macro_stack (name)
    local macro = get_macro(name)
    if macro.stack and #macro.stack > 0 then
        return pop(macro.stack)
    end
end

--- value of top of macro stack.
-- @param name macro name
-- @return any value, if defined
function M.value_of_macro_stack (name)
    local macro = get_macro(name,true)
    if not macro then return nil end
    if macro.stack and #macro.stack > 0 then
        return macro.stack[#macro.stack]
    end
end

local lua_keywords = {
    ['do'] = 'open', ['then'] = 'open', ['else'] = 'open', ['function'] = 'open',
    ['repeat'] = 'open';
    ['end'] = 'close', ['until'] = 'close',['elseif'] = 'close'
}

local c_keywords = {}
local keywords = lua_keywords

local block_handlers,keyword_handlers = {},{}
local level = 1

--- specify a block handler at a given level.
-- a block handler may indicate with an extra true return
-- that it wants to persist; it is passed the getter and the keyword
-- so we can get more specific end-of-block handlers.
-- @param lev relative block level
-- @param action will be called when the block reaches the level
function M.block_handler (lev,action)
    lev = lev + level
    if not block_handlers[lev] then
        block_handlers[lev] = {}
    end
    append(block_handlers[lev],action)
end

local function process_block_handlers(level,get,v)
    local persist,result
    for _,bh in pairs(block_handlers[level]) do
        local res,keep = bh(get,v)
        if not keep then
            if res then result = res end
        else
            persist = persist or {}
            append(persist,bh)
        end
    end
    block_handlers[level] = persist
    return result
end


--- set a keyword handler. Unlike macros, the keyword itself is always
-- passed through, but the handler may add some output afterwards.
-- If the action is nil, then the handler for that keyword is removed.
-- @param word keyword
-- @param action function to be called when keyword is encountered
-- @return previous handler associated with this keyword
function M.keyword_handler (word,action)
    if word == 'BEGIN' or word == 'END' then
        keyword_handlers[word] = action
        return
    end
    if action then
        local last = keyword_handlers[word]
        keyword_handlers[word] = action
        return last
    else
        keyword_handlers[word] = nil
    end
end

--- set a scoped keyword handler. Like keyword_handler, except
-- it restores the original keyword handler (if any) at the end
-- of the current block.
-- @param word keyword
-- @param action to be called when keyword is encountered
-- @see keyword_handler
function M.scoped_keyword_handler (keyword, action)
    local last = M.keyword_handler(keyword,action)
    M.block_handler(-1,function()
        M.keyword_handler(keyword,last)
    end)
end

-- a convenient way to use keyword handlers. This sets a handler and restores
-- the old handler at the end of the current block.
-- @param word keyword
-- @param action to be called when keyword is encountered
-- @return a function that creates a scoped keyword handler
function M.make_scoped_handler(keyword,handler)
    return function() M.scoped_keyword_handler(keyword, action) end
end

M.please_throw = false

--- macro error messages.
-- @param msg the message: will also have file:line.
function M.error(msg)
    msg = M.filename..':'..lexer.line..': '..msg
    if M.please_throw then
        error(msg,2)
    else
        io.stderr:write(msg,'\n')
        os.exit(1)
    end
end

M.define ('debug_',function()
    M.DEBUG = true
end)

--- macro error assert.
-- @param expr an expression.
-- @param msg a message
function M.assert(expr,msg)
    if not expr then M.error(msg or 'internal error') end
    return expr
end

Getter.error = M.error
Getter.assert = M.assert
TokenList.assert = M.assert

local line_updater, line_table, last_name, last_lang

local function lua_line_updater (iline,oline)
    if not line_table then line_table = {} end
    append(line_table,{il=iline,ol=oline})
end

local function c_line_updater (iline,oline,last_t,last_v)
    local endt = last_t == 'space' and last_v or '\n'
    return '#line '..iline..' "'..M.filename..'"'..endt
end

local make_putter = TokenList.new

--- do a macro substitution on Lua source.
-- @param src Lua source (either string or file-like reader)
-- @param out output (a file-like writer)
-- @param name input file name
-- @param use_c nil for Lua; if 'line', then output #line directives; if true, then don't
-- @return the result as table of strings
-- @return line number information
function M.substitute(src,name, use_c)
    local out, ii = {}, 1
    local subparse
    if name then
        last_name = name
        last_lang = use_c
    else
        name = last_name
        use_c = last_lang and true
        subparse = true
    end
    M.filename = name
    if use_c then
        lexer = require 'macro.clexer'
        scan_code = lexer.scan_c
        keywords = c_keywords
        if use_c == 'line' then
            line_updater = c_line_updater
        else
            line_updater = function() end
        end
    else
        lexer = require 'macro.lexer'
        scan_code = lexer.scan_lua
        keywords = lua_keywords
        line_updater = lua_line_updater
    end
    local tok = scan_code(src,name)
    local iline,iline_changed = 0
    local last_t,last_v = 'space','\n'
    local do_action


    local t,v = tok()

    -- this function get() is always used, so that we can handle end-of-stream properly.
    -- The substitution mechanism pushes a new stream on the tstack, which is popped
    -- when empty.
    local tstack = {}
    local push,pop = table.insert,table.remove

    local function get ()
        last_t,last_v = t,v
        local t,v = tok()
        while not t do
            tok = pop(tstack)
            if tok == nil then
                if not subparse and keyword_handlers.END then
                    do_action(keyword_handlers.END)
                    keyword_handlers.END = nil
                end
                if tok == nil then -- END action might have inserted some tokens
                    return nil
                end
            end -- finally finished
            t,v = tok()
        end
        if name == lexer.name and iline ~= lexer.line  then
            iline = lexer.line -- input line has changed
            iline_changed = last_v
        end
        return t,v
    end

    local getter = Getter.new(get)

    --- get a list of consecutive matching tokens.
    -- @param get token fetching function
    -- @param accept set of token types (default: `{space=true,comment=true}`)
    function getter.matching (get, accept)
        accept = accept or {space=true, comment=true}
        local tl = TokenList.new()
        local t,v = get:peek(1, true)
        while accept[t] do
            t,v = get ()
            append(tl, {t, v})
            t,v = get:peek(1, true)
        end
        return tl
    end

    function getter:peek (offset,dont_skip)
        local step = offset < 0 and -1 or 1 -- passing offset 0 is undefined
        local k = 0
        local token, t, v
        repeat
            while true do
                token = tok (k)
                if not token then return nil, 'EOS' end
                t,v = token[1], token[2]
                if dont_skip or (t ~= 'space' and t ~= 'comment') then break end
                k = k + 1
            end
            offset = offset - step
            k = k + step
        until offset == 0
        return t,v,k+1
    end

    function getter:peek2 ()
        local t1,v1,k1 = self:peek(1)
        local t2,v2 = self:peek(k1+1)
        return t1,v1,t2,v2
    end

    function getter:patch (idx,text)
        out[idx] = text
    end

    function getter:placeholder (put)
        put:iden '/MARK?/'
        return ii
    end

    function getter:copy_from (pos,clear)
        local res = {}
        for i = pos, ii do
            if out[i] and not out[i]:match '^#line' then
                append(res,out[i])
            end
        end
        if clear then
            for i = pos, ii do
                table.remove(out,pos)
                ii = ii - 1
            end
        end
        return table.concat(res)
    end

    -- this feeds the results of a substitution into the token stream.
    -- substitutions may be token lists, Lua strings or nil, in which case
    -- the substitution is ignored. The result is to push a new token stream
    -- onto the tstack, so it can be fetched using get() above
    local function push_substitution (subst)
        if subst == nil then return end
        local st = type(subst)
        push(tstack,tok)
        if st == 'table' then
            subst = scan_iter(subst)
        elseif st == 'string' then
            subst = scan_code(subst)
        end
        tok = subst
    end
    M.push_substitution = push_substitution

    -- a macro object consists of a subst object and (optional) parameters.
    -- If there are parms, then a macro argument list must follow.
    -- The subst object is either a token list or a function; if a token list we
    -- substitute the actual parameters for the formal parameters; if a function
    -- then we call it with the actual parameters.
    -- Without parameters, it may be a simple substitution (TL or Lua string) or
    -- may be a function. In the latter case we call it passing the token getter,
    -- assuming that it will grab anything it needs from the token stream.
    local function expand_macro(get,mac)
        local pass_through
        local subst = mac.subst
        local fun = type(subst)=='function'
        if mac.parms then
            t = tnext(get);
            if t ~= '(' then
                M.error('macro '..mac.name..' expects parameters')
            end
            local args,err = Getter.list(get)
            M.assert(args,'no end of argument list')
            if fun then
                subst = subst(unpack(args))
            else
                if #mac.parms ~= #args then
                    M.error(mac.name.." takes "..#mac.parms.." arguments")
                end
                subst = substitute_tokenlist(subst,mac.parms,args)
            end
        elseif fun then
            subst,pass_through = subst(getter,make_putter())
        end
        push_substitution(subst)
        return pass_through
    end

    local multiline_tokens,sync = lexer.multiline_tokens,lexer.sync
    local line,last_diff = 0,0

    function do_action (action)
        push_substitution(action(getter,make_putter()))
    end

    if not subparse and keyword_handlers.BEGIN then
        do_action(keyword_handlers.BEGIN)
    end

    while t do
        --print('tv',t,v)
        local dump = true
        if t == 'iden' then -- classic name macro
            local mac = imacros[v]
            if mac then
                dump = expand_macro(get,mac)
            end
        elseif t == 'keyword' then
            -- important to track block level for lexical scoping and block handlers
            local class = keywords[v]
            if class == 'open' then
                if v ~= 'else' then level = level + 1 end
            elseif class == 'close' then
                level = level - 1
                if block_handlers[level] then
                    local res = process_block_handlers(level,get,v)
                    if res then push_substitution(res) end
                end
            --* elseif class == 'hook' then
            end
            local action = keyword_handlers[v]
            if action then do_action(action) end
        else -- any unused 'operator' token (like @, \, #) can be used as a macro
            if use_c then
                if v == '{' then
                    level = level + 1
                elseif v == '}' then
                    level = level - 1
                    if block_handlers[level] then
                        local res = process_block_handlers(level,get,v)
                        if res then push_substitution(res) end
                    end
                end
            end
            local mac = smacros[v]
            if mac then
                dump = expand_macro(get,mac)
            end
        end
        if dump then
            if multiline_tokens[t] then -- track output line
                line = sync(line, v)
            end
            if iline_changed then
                local diff = line - iline
                if diff ~= last_diff then
                    local ldir = line_updater(iline,line,last_t,last_v)
                    if ldir then out[ii] = ldir; ii=ii+1 end
                    last_diff = diff
                end
                iline_changed = nil
            end
            out[ii] = v
            ii = ii + 1
        end
        t,v = get()
    end

    return out,line_table
end

--- take some Lua source and return the result of the substitution.
-- Does not raise any errors.
-- @param src either a string or a readable file object
-- @param name optional name for the chunk
-- @return the result or nil
-- @return the error, if error
function M.substitute_tostring(src,name,use_c,throw)
    M.please_throw = true
    local ok,out,li
    if throw then
        out,li = M.substitute(src,name,use_c)
    else
        ok,out,li = pcall(M.substitute,src,name,use_c)
    end
    if type(src) ~= 'string' and src.close then src:close() end
    if not ok then return nil, out
    else
        return table.concat(out), li
    end
end

local lua52 = _VERSION:match '5.2'
local load, searchpath = load, package.searchpath

if not lua52 then -- Lua 5.1
    function load (env,src,name)
        local chunk,err = loadstring(src,name)
        if chunk and env then
            setfenv(chunk,env)
        end
        return chunk,err
    end
end

if not searchpath then
    local sep = package.config:sub(1,1)
    searchpath = function (mod,path)
        mod = mod:gsub('%.',sep)
        for m in path:gmatch('[^;]+') do
            local nm = m:gsub('?',mod)
            local f = io.open(nm,'r')
            if f then f:close(); return nm end
        end
    end
end

--- load Lua code in a given envrionment after passing
-- through the macro preprocessor.
-- @param src either a string or a readable file object
-- @param name optional name for the chunk
-- @param env the environment (may be nil)
-- @return the cnunk, or nil
-- @return the error, if no chunk
function M.load(src,name,env)
    local res,err = M.substitute_tostring(src,'tmp')
    if not res then return nil,err end
    return loadin(env,res,name)
end

--- evaluate Lua macro code in a given environment.
-- @param src either a string or a readable file object
-- @param env the environment (can be nil)
-- @return true if succeeded
-- @return result(s)
function M.eval(src,env)
    local chunk,err = M.loadin(src,'(tmp)',env)
    if not chunk then return nil,err end
    return pcall(chunk)
end

package.mpath = './?.m.lua'

--- Make `require` use macro expansion.
-- This is controlled by package.mpath, which is initially './?.m.lua'
function M.set_package_loader()
    -- directly inspired by https://github.com/bartbes/Meta/blob/master/meta.lua#L32,
    -- after a suggestion by Alexander Gladysh
    table.insert(package.loaders, function(name)
        local fname = searchpath(name,package.mpath)
        if not fname then return nil,"cannot find "..name end
        local res,err = M.load(io.open(fname),lname)
        if not res then
            error (err)
        end
        return res
    end)
end

return macro
