require_ 'macro.do'
-- unrolling a loop
y = 0
do_( i,1, 10,
  y = y + i
)
assert(y == 55)

-- do_ defined a _local_ macro 'i'
assert(i == nil)


-- tuples usually expand to A_1,A_2,A_3 and so forth
tuple(3) A,B
B = 10,20,30
print(B)

def_ do3(v,s) do_(v,1,3,s)

-- but inside a do_ statements with tuples work element-wise
-- debug_
do_(k,1,3,
    A = B/2
)
--[[
print(A)
--]]

