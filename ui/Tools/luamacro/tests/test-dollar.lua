require_ 'dollar'
print($PATH)
if $(ls) ~= 0 then
  print($(dir /B)) -- so there!
end


