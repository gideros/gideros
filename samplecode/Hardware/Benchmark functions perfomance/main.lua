--[[
	https://blog.giderosmobile.com/2018/04/how-much-faster-is-gideros-rad-operator.html
]]

max=1000000
calls=10
local randomSeed,random=Core.randomSeed,Core.random
local rad=math.rad
randomSeed(0,1) -- just in case the first call takes longer

function test1()
	randomSeed(0,1)
	for loop=1,max do
		local n=random(0,1,360)
		n=math.rad(n)
	end
end

function test2()
	randomSeed(0,1)
	for loop=1,max do
		local n=random(0,1,360)
		n=rad(n)
	end
end

function test3()
	local rad2=rad
	randomSeed(0,1)
	for loop=1,max do
		local n=random(0,1,360)
		n=rad2(n)
	end
end

function test4()
	randomSeed(0,1)
	for loop=1,max do
		local n=random(0,1,360)
		n=^<n
	end
end

Core.profilerReset()
Core.profilerStart()
for loop=1,calls do
	test1()
	test2()
	test3()
	test4()
end
Core.profilerStop()

-- print the results of the profiler
result=Core.profilerReport()
print("Number of tests:",max*calls)
for k,v in pairs(result) do
	local found=false
	for k2,v2 in pairs(v) do
		if found and k2=="time" then print(v1,v2) end
		if k2=="name" and string.sub(v2,1,4)=="test" then v1=v2 found=true end
	end
end
