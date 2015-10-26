require_ 'macro.try'
local check
try
    try
      if arg[1] then error('throw!') end
    except(err)
        print('shd be throw',err)
    end
    a.x = 0
except(e)
    check = e
end

-- the truly tricky case here is the plain return, since we need to distinguish
-- it from merely leaving the protected block.
function testtry(n)
    try
        if n == 0 then return 'cool'
        elseif n == 1 then return 'cool',42
        elseif n == 2 then return --aha
        elseif n == 3 then
            error('throw baby!')
        end
    except(msg)
        return nil,msg
    end
    return 42,'answer'
end

function match (x,y)
    local ok
    if type(x) == 'string' and type(y) == 'string' then
        ok = y:find(x)
    else
        ok = x == y
    end
    if not ok then print('mismatch',x,y) end
    return ok
end

function assert2 (x1,x2,y1,y2)
    assert(match(x1,y1) and match(x2,y2))
end

assert(match("attempt to index global 'a' %(a nil value%)$",check))
assert2('cool',nil,testtry(0))
assert2('cool',42,testtry(1))
assert2(42,'answer',testtry(2))
assert2(nil,'throw baby!$',testtry(3))
assert2(42,'answer',testtry(4))


