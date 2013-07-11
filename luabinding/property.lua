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

Core.class = function (b)
	b = b or __Object

	local c = {}
	c.__index = c
	setmetatable(c, b)

	c.super = b

	c.__new = function(...)
		local b = getmetatable(c)

		local s1 = b.__new(...)

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

	return c
end
