local mod = require 'mod'
-- def_ show(expr) print(_STR_(expr),expr)

assert(mod.one() == 42)

f = mod.Fred(22)

assert(f:get() == 22)
f:set2()
assert(f:get() == 0)

a = mod.Alice()
a:set2()
assert(a:get() == 1)
a:set(66)
assert(tostring(a) == "Alice 66")




