require "CuteC2"
require "SceneManager"

sceneManager = SceneManager.new({
	Collisions = CollisionsScene,
	TOI = TOIScene,
	GJK = GJKScene,
	Ray = RayScene,
})

sceneManager:changeScene("Ray")
stage:addChild(sceneManager)
Vec = Core.class()

function Vec:init(x, y)
	self.x = x
	self.y = y
end
function Vec:__add(other)
	return Vec.new(self.x + other.x, self.y + other.y)
end
function Vec:__sub(other)
	return Vec.new(self.x - other.x, self.y - other.y)
end
function Vec:__mul(val)
	if (type(self) == 'number') then 
		self, val = val, self
	end
	return Vec.new(self.x * val, self.y * val)
end
function Vec:__tostring()
	return ("(%f; %f)"):format(self.x, self.y)
end

local min = Vec.new(-150, -150)
local max = Vec.new(-120, -110)
local p = Vec.new(0, 0)
local size = max - min
local hsize = 0.5 * size

local dst = p - hsize
local offset = dst - min
--print(dst, min + offset)


--min: (-414.000000; -310.425629) max: (0.000000; 0.000000) hSize: (207.000000; 155.212814)
--Dst: (926.000000; 535.787170) offset: (1340.000000; 846.212769) pos: (1133.000000; 691.000000)
