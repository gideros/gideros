-- Timer test
local timer = Timer.new(1000, 2)

timer.proxy = newproxy(true)
getmetatable(timer.proxy).__gc = function() print("timer collected.") end

timer:start()

timer:addEventListener(Event.TIMER_COMPLETE, function() print("timer complete.") end)


-- SoundChannel test
local sound = Sound.new("testaudio-stereo.wav")
local channel = sound:play()

channel.proxy = newproxy(true)
getmetatable(channel.proxy).__gc = function() print("channel collected.") end

channel:addEventListener(Event.COMPLETE, function() print("channel complete.") end)


-- UrlLoader test
local urlloader = UrlLoader.new("http://www.google.com")

urlloader.proxy = newproxy(true)
getmetatable(urlloader.proxy).__gc = function() print("urlloader collected.") end

urlloader:addEventListener(Event.COMPLETE, function() print("urlloader complete.") end)


-- collectgarbage
stage:addEventListener(Event.ENTER_FRAME, function() collectgarbage() end)

