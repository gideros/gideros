require_ 'macro.forall'

def_ dump(t) print '---'; forall val in t do print(val) end

forall x in {10,20,30} do print(x) end

t = {'hello','dolly'}
print '---'
forall name in t do print(name) end
print '---'
forall x in t if x:match 'o$' do print(x) end

-- a wee bit tautological, but valid!
print '---'
forall x in L{x^2 | x in {10,20,30}} do print(x) end

t = L{s:upper() | s in {'one','two','three'} if s ~= 'two'}

dump(t)

forall i = 1,5 do print(i) end

t = L{2*i|i=1,10}

dump(t)

-- identity matrix using nested list comprehensions.
t = L{L{i==j and 1 or 0 | j=1,3} | i=1,3}

-- note the other form of LCs: using 'for' means that you explicitly want
-- the generic Lua for-statement.
ls = L{line for line in io.lines 'test-forall.lua'}
print('length of this file',#ls)
