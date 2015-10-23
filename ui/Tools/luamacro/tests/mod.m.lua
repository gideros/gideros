require_ 'module'

local function dump(text)
  print (text)
end

function one ()
  return two()
end

class Fred

  function _init(self,x)
    @set(x or 1)
  end

  function set(self,x)
    @x = x
  end

  function get(self)
    return @x
  end

  function set2(self)
    @set(0)
  end

end

class Alice : Fred
  function __tostring(self)
    return "Alice "..tostring(@x)
  end

  function set2(self)
    @set(1)
  end
end


function two ()
  return 42
end




