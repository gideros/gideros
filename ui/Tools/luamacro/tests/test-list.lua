--- sugar for making pl.List work like Python lists.
-- The list macro is a factory which generates macros which 'shadow' the variable
-- and kick in when they are followed by [...].
require_ 'list'

-- the two forms of 'list' initialization
-- (altho it grabs values upto '\n', this only happens outside a () or {},
-- so multi-line initializations are possible
list ls,lo = {10,20,30},{'A','ay',
    'B','C'}
list two

-- the above statements created both the macros 'ls' and values 'ls', etc.
two:append(1)
two:append(2)

-- seen as plain table access
print(ls[2])

-- special treatment for slice notation
print(ls[1:2])

-- if we are on the LHS, then adjust accordingly
ls[1:2] = {11,21,22}

print(ls[2:])

print(ls, two, lo)

-- like in Python, this makes a copy of all of the list
print(ls[:])





