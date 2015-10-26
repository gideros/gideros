-- in this case, a simple statement macro can be used
-- `block ... end` expands to `(function() ... end)`

def_ block (function() _END_")"

function peval(fun)
    print(fun())
end

peval block
    return 10,'hello',54
end

