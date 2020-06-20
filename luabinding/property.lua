do
	local c = Object
	c.__index = c

	c.__new = function(...)
		local s1 = {}

		setmetatable(s1, c)

		local init = rawget(c, "init")
		if type(init) == "function" then
			init(s1, ...)
		end

		return s1		
	end

	c.new = function(...)
		local s1 = c.__new(...)

		local postInit = s1.postInit
		if type(postInit) == "function" then
			postInit(s1, ...)
		end

		return s1
	end
end

local __Object = Object

Core = {}

local function callDestructors(s)
  k=getmetatable(s)
  while k and k.__gid_destructor do
	local g=k.__gid_destructor
    local gt=type(g)
    if gt=="function" then g(s)
    elseif gt=="string" then k[g](s)
    end
    k=getmetatable(k)
  end
end
Core.class = function (b,a,d)
  if a then
      assert(type(a)=="function","Second argument to Core.class() should be a function or null")
  end
  if d then
      assert((type(d)=="function") or (type(d)=="string"),"Third argument to Core.class() should be a function, a function name or null")
  end
	b = b or __Object

	local c = {}
	c.__index = c
	c.__gid_destructor = d
	if not d and b.__gid_destructor then c.__gid_destructor=function() end end 
	setmetatable(c, b)

	c.super = b

	c.__new = function(...)
		local b = getmetatable(c)
		local s1
    if a then
      s1 = b.__new(a(...))      
    else
		  s1 = b.__new(...)
		end

		setmetatable(s1, c)

		local init = rawget(c, "init")
		if type(init) == "function" then
			init(s1, ...)
		end

		return s1
	end

	c.new = function(...)
		local s1 = c.__new(...)

    if c.__gid_destructor then
      local prox = newproxy(true)
      getmetatable(prox).__gc = function() callDestructors(s1) end
      s1.__gid_destructor = prox
    end

		local postInit = s1.postInit
		if type(postInit) == "function" then
			postInit(s1, ...)
		end
 
		return s1
	end

	return c
end
