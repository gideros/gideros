require_ 'macro.with'

aLongTableName = {}
with aLongTableName do
  .a = 1
  .b = {{x=1},{x=2}}
  .c = {f = 2}
  print(.a,.c.f,.b[1].x)
end

def_ @ return
def_ F function

F f(x) @ x+1 end

print(f(10))


