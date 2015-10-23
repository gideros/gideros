def_ OF_ def_ (of elseif _value ==)
def_ case(x) do OF_ local _value = x if false then _END_END_

function test(n)
    local res
    case(n)
    of 10 then
        res = 1
    of 20 then
        res = 2
    else
        res = 3
    end
    return res
end

assert(test(10)==1)
assert(test(20)==2)
assert(test(30)==3)


