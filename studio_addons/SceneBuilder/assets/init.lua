
--LIBRAIRIES
_inspect = nil
local function requireInspect()
	pcall(function () 
		local inspect = require("inspect")
		if inspect and inspect.inspect then _inspect = function(t) return inspect.inspect(t) end end
	end)
end
requireInspect()


local fontfile="OpenSans-Bold.ttf"
UI=UI or {}
UI.Default= { TTF=fontfile }
