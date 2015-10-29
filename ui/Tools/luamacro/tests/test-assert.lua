require_ 'macro.try'

def_ ASSERT(condn,expr) if condn then else error(expr) end

try 
  ASSERT(2 == 1,"damn..".. 2 .." not equal to ".. 1)
except (e)
  print('threw:',e)
end

