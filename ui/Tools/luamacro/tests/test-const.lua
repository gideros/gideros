require_ 'const'
do
  const N,M = 10,20
  do
     const N = 5
     assert(N == 5)
  end
  assert(N == 10 and M == 20)
end
assert(N == nil and M == nil)
