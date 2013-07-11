--[[
function Sound:playWithLoopCount(nbLoops)
    nbLoops = nbLoops or 1
    local soundLength = self:getLength()

    local channel = self:play(0, true) -- loop infinitely

    -- set looping as false after (loops - 0.5) count
    Timer.delayedCall((nbLoops - 0.5) * soundLength, function() channel:setLooping(false) end)

    return channel
end

local sound = Sound.new("1.wav")

sound:playWithLoopCount(3)

do
return
end

--]]


--local sound = Sound.new("bgmusicloop.wav")
local sound = Sound.new("bgmusicloop.mp3")

print(sound:getLength())

local channel = sound:play(26000, true, false)

--print(channel:getPosition())
--channel:setPosition(20000)
--print(channel:getPosition())
--channel:setPaused(false)

channel:addEventListener(Event.COMPLETE, function() print("complete") end)

local function onTimer()
	print(channel:getPosition(), sound:getLength())
	collectgarbage()
end

local t = Timer.new(100)
t:addEventListener(Event.TIMER, onTimer)
t:start()

stage:addEventListener(Event.MOUSE_DOWN, function() 
	channel:setPaused(not channel:isPaused())
	--channel:setPosition(10000)
	--channel:setPitch(1.5)
 end)
