-- Simplifying writing C extensions for Lua
-- Adds new module and class constructs;
-- see class1.lc and str.lc for examples.
local M = require 'macro'

function dollar_subst(s,tbl)
  return (s:gsub('%$%((%a+)%)',tbl))
end

-- reuse some machinery from the C-skin experiments
local push,pop = table.insert,table.remove
local bstack,btop = {},{}

local function push_brace_stack (newv)
    newv = newv or {}
    newv.lev = 0
    push(bstack,btop)
    btop = newv
end

M.define('{',function()
    if btop.lev then
        btop.lev = btop.lev + 1
    end
    return nil,true --> pass-through macro
end)

M.define('}',function(get,put)
    if not btop.lev then
        return nil,true
    elseif btop.lev == 0 then
        local res
        if btop.handler then res = btop.handler(get,put) end
        if not res then res = put:space() '}' end
        btop = pop(bstack)
        return res
    else
        btop.lev = btop.lev - 1
        return nil,true --> pass-through macro
    end
end)

--------- actual implementation begins -------

local append = table.insert
local module

local function register_functions (names,cnames)
    local out = {}
    for i = 1,#names do
        append(out,('   {"%s",l_%s},'):format(names[i],cnames[i]))
    end
    return table.concat(out,'\n')
end

local function finalizers (names)
    local out = {}
    for i = 1,#names do
        append(out,names[i].."(L);")
    end
    return table.concat(out,'\n')
end

local typedefs

local preamble = [[
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#ifdef WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
#if LUA_VERSION_NUM > 501
#define lua_objlen lua_rawlen
#endif
]]

local finis = [[
static const luaL_Reg $(cname)_funs[] = {
    $(funs)
    {NULL,NULL}
};

EXPORT int luaopen_$(cname) (lua_State *L) {
#if LUA_VERSION_NUM > 501
    lua_newtable(L);
    luaL_setfuncs (L,$(cname)_funs,0);
    lua_pushvalue(L,-1);
    lua_setglobal(L,"$(cname)");
#else
    luaL_register(L,"$(cname)",$(cname)_funs);
#endif
    $(finalizers)
    return 1;
}
]]

M.define('module',function(get)
    local name = get:string()
    local cname = name:gsub('%.','_')
    get:expecting '{'
    local out = preamble .. typedefs
    push_brace_stack{
      name = name, cname = cname,
      names = {}, cnames = {}, finalizers = {},
      handler = function()
        local out = {}
        local funs = register_functions(btop.names,btop.cnames)
        local final = finalizers(btop.finalizers)
        append(out,dollar_subst(finis, {
            cname = cname,
            name = name,
            funs = funs,
            finalizers = final
        }))
        return table.concat(out,'\n')
    end }
    module = btop
    return out
end)


M.define('def',function(get)
    local fname = get:name()
    local cname = (btop.ns and btop.ns..'_' or '')..fname
    append(btop.names,fname)
    append(btop.cnames,cname)
    get:expecting '('
    local args = get:list():strip_spaces()
    get:expecting '{'
    local t,space = get()
    indent = space:gsub('^%s*[\n\r]',''):gsub('%s$','')
    local out = {"static int l_"..cname.."(lua_State *L) {"}
    if btop.massage_arg then
        btop.massage_arg(args)
    end
    for i,arg in ipairs(args) do
        local mac = arg[1][2]..'_init'
        if arg[3] and arg[3][1] == '=' then
            mac = mac .. 'o'
            i = i .. ',' .. arg[4][2]
        end
        if not arg[2] then M.error("parameter must be TYPE NAME [= VALUE]") end
        append(out,indent..mac..'('..arg[2][2]..','..i..');')
    end
    --append(out,space)
    return table.concat(out,'\n')..space
end)

M.define('constants',function(get,put)
    get:expecting '{'
    local consts = get:list '}' :strip_spaces()
    --for k,v in pairs(btop) do io.stderr:write(k,'=',tostring(v),'\n') end
    -- os.exit()
    local fname = 'set_'..btop.cname..'_constants'
    local out = { 'static void '..fname..'(lua_State *L) {'}
    if not btop.finalizers then M.error("not inside a module") end
    append(btop.finalizers,fname)
    for _,c in ipairs(consts) do
        local type,value,name
        if #c == 1 then -- a simple int constant: CONST
            name = c:pick(1)
            type = 'Int'
            value = name
        else -- Type CONST [ = VALUE ]
            type = c:pick(1)
            name = c:pick(2)
            if #c == 2 then
                value = name
            else
                value = c:pick(4)
            end
        end
        append(out,('%s_set("%s",%s);'):format(type,name,value ))
    end
    append(out,'}')
    return table.concat(out,'\n')
end)

M.define('assign',function(get)
    get:expecting '{'
    local asses = get:list '}' :strip_spaces()
    local out = {}
    for _,c in ipairs(asses) do
        append(out,('%s_set("%s",%s);\n'):format(c:pick(1),c:pick(2),c:pick(4)) )
    end
    return table.concat(out,'\n')
end)

local load_lua = [[
static void load_lua_code (lua_State *L) {
  luaL_dostring(L,lua_code_block);
}
]]

M.define('lua',function(get)
    get:expecting '{'
    local block = tostring(get:upto '}')
    local code_name = 'lua_code_block'
    local out = {'static const char *'.. code_name .. ' = ""\\'}
    for line in block:gmatch('([^\r\n]+)') do
        line = line:gsub('\\','\\\\'):gsub('"','\\"')
        append(out,'  "'..line..'\\n"\\')
    end
    append(out,';')
    append(out,load_lua)
    out = table.concat(out,'\n')
    append(module.finalizers,'load_lua_code')
    return out
end)

typedefs = [[
typedef const char *Str;
typedef const char *StrNil;
typedef int Int;
typedef double Number;
typedef int Boolean;
]]


M.define 'Str_init(var,idx) const char *var = luaL_checklstring(L,idx,NULL)'
M.define 'Str_inito(var,idx,val) const char *var = luaL_optlstring(L,idx,val,NULL)'
M.define 'Str_set(name,value) lua_pushstring(L,value); lua_setfield(L,-2,name)'
M.define 'Str_get(var,name) lua_getfield(L,-1,name); var=lua_tostring(L,-1); lua_pop(L,1)'
M.define 'Str_geti(var,idx) lua_rawgeti(L,-1,idx); var=lua_tostring(L,-1); lua_pop(L,1)'

M.define 'StrNil_init(var,idx) const char *var = lua_tostring(L,idx)'

M.define 'Int_init(var,idx) int var = luaL_checkinteger(L,idx)'
M.define 'Int_inito(var,idx,val) int var = luaL_optinteger(L,idx,val)'
M.define 'Int_set(name,value) lua_pushinteger(L,value); lua_setfield(L,-2,name)'
M.define 'Int_get(var,name) lua_getfield(L,-1,name); var=lua_tointeger(L,-1); lua_pop(L,1)'
M.define 'Int_geti(var,idx) lua_rawgeti(L,-1,idx); var=lua_tointeger(L,-1); lua_pop(L,1)'

M.define 'Number_init(var,idx) double var = luaL_checknumber(L,idx)'
M.define 'Number_inito(var,idx,val) double var = luaL_optnumber(L,idx,val)'
M.define 'NUmber_set(name,value) lua_pushnumber(L,value); lua_setfield(L,-2,name)'
M.define 'Number_get(var,name) lua_getfield(L,-1,name); var=lua_tonumber(L,-1); lua_pop(L,1)'
M.define 'Number_geti(var,idx) lua_rawgeti(L,-1,idx); var=lua_tonumber(L,-1); lua_pop(L,1)'

M.define 'Boolean_init(var,idx) int var = lua_toboolean(L,idx)'
M.define 'Boolean_set(name,value) lua_pushboolean(L,value); lua_setfield(L,-2,name)'
M.define 'Boolean_get(var,name) lua_getfield(L,-1,name); var=lua_toboolean(L,-1); lua_pop(L,1)'
M.define 'Boolean_geti(var,idx) lua_rawgeti(L,-1,idx); var=lua_toboolean(L,-1); lua_pop(L,1)'

M.define 'Value_init(var,idx) int var = idx'

M.define('lua_tests',function(get)
    get:expecting '{'
    local body = get:upto '}'
    local f = io.open(M.filename..'.lua','w')
    f:write(tostring(body))
    f:close()
end)

------ class support ----------------------

local klass_ctor = "static void $(klass)_ctor(lua_State *L, $(klass) *this, $(fargs))"

local begin_klass = [[

typedef struct {
  $(fields)
} $(klass);

define_ $(klass)_init(var,idx) $(klass) *var = $(klass)_arg(L,idx)

#define $(klass)_MT "$(klass)"

$(klass) * $(klass)_arg(lua_State *L,int idx) {
  $(klass) *this = ($(klass) *)luaL_checkudata(L,idx,$(klass)_MT);
  luaL_argcheck(L, this != NULL, idx, "$(klass) expected");
  return this;
}

$(ctor);

static int push_new_$(klass)(lua_State *L,$(fargs)) {
  $(klass) *this = ($(klass) *)lua_newuserdata(L,sizeof($(klass)));
  luaL_getmetatable(L,$(klass)_MT);
  lua_setmetatable(L,-2);
  $(klass)_ctor(L,this,$(aargs));
  return 1;
}

]]

local end_klass = [[

static const struct luaL_Reg $(klass)_methods [] = {
  $(methods)
  {NULL, NULL}  /* sentinel */
};

static void $(klass)_register (lua_State *L) {
  luaL_newmetatable(L,$(klass)_MT);
#if LUA_VERSION_NUM > 501
  luaL_setfuncs(L,$(klass)_methods,0);
#else
  luaL_register(L,NULL,$(klass)_methods);
#endif
  lua_pushvalue(L,-1);
  lua_setfield(L,-2,"__index");
  lua_pop(L,1);
}
]]

M.define('class',function(get)
    local name = get:iden()
    get:expecting '{'
    local fields = get:upto (function(t,v)
        return t == 'iden' and v == 'constructor'
    end)
    fields = tostring(fields):gsub('%s+$','\n')
    get:expecting '('
    local out = {}
    local args = get:list()
    local f_args = args:strip_spaces()
    local a_args = f_args:pick(2)
    f_args = table.concat(args:__tostring(),',')
    a_args = table.concat(a_args,',')
    local subst = {klass=name,fields=fields,fargs=f_args,aargs=a_args }
    local proto = dollar_subst(klass_ctor,subst)
    subst.ctor = proto
    append(out,dollar_subst(begin_klass,subst))
    append(out,proto)
    local pp = {{'iden',name},{'iden','this'}}
    push_brace_stack{
      names = {}, cnames = {}, ns = name, cname = name,
      massage_arg = function(args)
        table.insert(args,1,pp)
      end,
      handler = function(get,put)
        append(module.finalizers,name.."_register")
        local methods = register_functions(btop.names,btop.cnames)
        return dollar_subst(end_klass,{methods=methods,klass=name,fargs=f_args,aargs=a_args})
      end
    }
    return table.concat(out,'\n')
end)

