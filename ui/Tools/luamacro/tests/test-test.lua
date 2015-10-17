require_ 'assert'

-- can compare table values as well
assert_ 2 == 2
assert_ print == print
assert_ {one=1,two=2} == {two=2,one=1}

assert_ 'hello' matches '^hell'
assert_ 2 > 1

--assert_ 3 < 2 -- quite wrong!

-- if the first expression returns multiple values, then
-- the second can match this with parentheses

function two() return 40,2 end

assert_ two() == (40,2)

function three() return {1,2},nil,'three' end

assert_ three() == ({1,2},nil,'three')

-- 'throws' only succeeds if the expression did actually raise an error,
-- and the error string does match the error message.

function bad() error 'something bad!' end

assert_ bad() throws "something bad!"

a = nil

assert_ a.x throws "attempt to index global 'a'"

-- can of course redefine assert_...
def_ assert assert_

-- This is an experimental feature, which matches two numbers up to the
-- precision supplied in the second number. This only happens if the test
-- number has a fractional part, and the number must be explicitly
-- be in %f formaat.
assert 3.1412 == 3.14
assert 2302.24432 == 2302.2
assert 100 == 100
assert 1.1e-3 == 0.001





