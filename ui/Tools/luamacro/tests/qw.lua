local macro = require 'macro'
macro.define('qw',function(get,put)
  local append = table.insert
  local t,v = get() 
  local res = {{'{','{'}}
  t,v = get:next()
  while t ~= ')' do
    if t ~= ',' then
      append(res,{'string','"'..v..'"'})
      append(res,{',',','})
    end    
    t,v = get:next()
  end
  append(res,{'}','}'})
  return res
end)