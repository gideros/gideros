-- Wrapping list comprehensions so that they return a pl.List
-- the trick here is that the L macro will pop its macro stack at the end,
-- if set. Here we want to wrap L{...} so it returns a list, so we have
-- List(L{...}).  By pushing ')', L will emit the close parens for us.
require 'pl'
require_ 'macro.forall'

def_ LL List( _PUSH_('L',')') L

print(LL{i|i=1,3}..LL{i|i=10,20,5})

print( LL{List{k,v} for k,v in pairs{A=1,B=2}} )
