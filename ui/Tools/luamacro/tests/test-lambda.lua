require_ 'macro.lambda'
ok = pcall(require,'pl')
if not ok then return print 'test-lambda needs penlight' end
ls = List{10,20,30}
assert(ls:map(\x(x+1)) == List{11,21,31})
assert((\(42))() == 42 )

F = \x,y(x - y)

G = \x(F(x,10))

assert(G(11) == 1)

ls = List { (\(10,20,30))() }
assert(ls == List{10,20,30})



