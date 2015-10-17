local M = require 'macro'

function _assert_arg(val,idx,t)
    if type(val) ~= t then
        error(("type mismatch argument %d: got %s, expecting %s"):format(idx,type(val),t),2)
    end
end

M.define('Function',function(get,put)
    local name = get:upto '('
    local args,endt = get:list()
    args = args:strip_spaces()
    local argnames,names = {},{}
    for i,a in ipairs(args) do
        local name = a:pick(1)
        M.assert(a:pick(2) == ':')
        table.remove(a,1)
        table.remove(a,1)
        argnames[i] = {{'iden',name}}
        names[i] = name
    end
    get:expecting ':'
    local rtype, endt = get:upto '\n'
    put :keyword 'function' :space() :tokens(name) '(' :list(argnames) ')' :space '\n'
    put :space()
    for i,a in ipairs(args) do
        local tp = a:pick(1)
        put :iden('_assert_arg') '(' :iden(names[i]) ',' :number(i) ',' :string(tp) ')' ';'
    end
    return put
end)
