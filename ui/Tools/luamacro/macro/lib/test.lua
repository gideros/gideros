--- `assert_` macro library support.
-- This module may of course be used on its own; `assert_` merely provides
-- some syntactical sugar for its functionality. It is based on Penlight's
-- `pl.test` module.
-- @module macro.libs.test

local test = {}

local _eq,_tostring

-- very much like tablex.deepcompare from Penlight
function _eq (v1,v2)
    if type(v1) ~= type(v2) then return false end
    -- if the value isn't a table, or it has defined the equality operator..
    local mt = getmetatable(v1)
    if (mt and mt.__eq) or type(v1) ~= 'table' then
        return v1 == v2
    end
    -- both values are plain tables
    if v1 == v2 then return true end -- they were the same table...
    for k1,x1 in pairs(v1) do
        local x2 = v2[k1]
        if x2 == nil or not _eq(x1,x2) then return false end
    end
    for k2,x2 in pairs(v2) do
        local x1 = v1[k2]
        if x1 == nil or not _eq(x1,x2) then return false end
    end
    return true
end

local function keyv (k)
    if type(k) ~= 'string' then
        k = '['..k..']'
    end
    return k
end

function _tostring (val)
    local mt = getmetatable(val)
    if (mt and mt.__tostring) or type(val) ~= 'table' then
        if type(val) == 'string' then
            return '"'..tostring(val)..'"'
        else
            return tostring(val)
        end
    end
    -- dump the table; doesn't need to be pretty!
    local res = {}
    local function put(s) res[#res+1] = s end
    put '{'
    for k,v in pairs(val) do
        put(keyv(k)..'=')
        put(_tostring(v))
        put ','
    end
    table.remove(res) -- remove last ','
    put '}'
    return table.concat(res)
end

local function _lt (v1,v2) return v1 < v2 end
local function _gt (v1,v2) return v1 > v2 end
local function _match (v1,v2) return v1:match(v2) end

local function _assert (v1,v2,cmp,msg)
    if not cmp(v1,v2) then
        print('first:',_tostring(v1))
        print(msg)
        print('second:',_tostring(v2))
        error('assertion failed',3)
    end
end

--- assert if parameters are not equal. If the values are tables,
-- they will be compared by value.
-- @param v1 given value
-- @param v2 test value
function test.assert_eq (v1,v2)
    _assert(v1,v2,_eq,"is not equal to");
end

--- assert if first parameter is not less than second.
-- @param v1 given value
-- @param v2 test value
function test.assert_lt (v1,v2)
    _assert(v1,v2,_lt,"is not less than")
end

--- assert if first parameter is not greater than second.
-- @param v1 given value
-- @param v2 test value
function test.assert_gt (v1,v2)
    _assert(v1,v2,_gt,"is not greater than")
end

--- assert if first parameter string does not match the second.
-- The condition is `v1:match(v2)`.
-- @param v1 given value
-- @param v2 test value
function test.assert_match (v1,v2)
    _assert(v1,v2,_match,"does not match")
end

-- return the error message from a function that raises an error.
-- Will raise an error if the function did not raise an error.
-- @param fun the function
-- @param ... any arguments to the function
-- @return the error message
function test.pcall_no(fun,...)
    local ok,err = pcall(fun,...)
    if ok then error('expression did not throw error',3) end
    return err
end

local tuple = {}

function tuple.__eq (a,b)
    if a.n ~= b.n then return false end
    for i=1, a.n do
        if not _eq(a[i],b[i]) then return false end
    end
    return true
end

function tuple.__tostring (self)
    local ts = {}
    for i = 1,self.n do
        ts[i] = _tostring(self[i])
    end
    return '('..table.concat(ts,',')..')'
end

--- create a tuple capturing multiple return values.
-- Equality between tuples means that all of their values are equal;
-- values may be `nil`
-- @param ... any values
-- @return a tuple object
function test.tuple(...)
    return setmetatable({n=select('#',...),...},tuple)
end

return test

