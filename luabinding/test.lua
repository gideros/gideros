--[[
-- it works!

ED = EventDispatcher()
ED_mt = getmetatable(ED)

A = {}
A.__index = ED_mt
setmetatable(A, ED_mt)

Aglue = {}
Aglue.__index =
   function (t, k)
	  return (A[k] or ED[k])
   end
setmetatable(Aglue, A)

function A:foo()
   print("A:foo()")
end

a = {}
setmetatable(a, Aglue)

print(a.__userdata)
a:dispatchEvent({type = "temp"})
a:foo()

--]]


function printTable(t)
   for a, b in pairs(t) do
	  print(a, b)
   end
end


--printTable(envTable())

a = Sprite.new()

a.graphics:beginFill(0.5, 0.5, 0.5)
a.graphics:moveTo(10, 10)
a.graphics:lineTo(100, 10)
a.graphics:lineTo(100, 100)
a.graphics:endFill()

function onEnterFrame(self, event)
   self:setVisible(not self:visible())
end

function onMouseDown(self, event)
   print("mouseDown", event.x, event.y)
end

function onMouseUp(self, event)
   print("mouseUp", event.x, event.y)
end

function onMouseMove(self, event)
   print("mouseMove", event.x, event.y)
end

print("false:", a:hasEventListener(Event.ENTER_FRAME))
print("false:", a:hasEventListener("gubar"))

a:addEventListener(Event.ENTER_FRAME, onEnterFrame, a)
a:addEventListener(MouseEvent.MOUSE_DOWN, onMouseDown, a)
a:addEventListener(MouseEvent.MOUSE_UP, onMouseUp, a)
a:addEventListener(MouseEvent.MOUSE_MOVE, onMouseMove, a)

stage:addChild(a)


print("true:", a:hasEventListener(Event.ENTER_FRAME))
print("false:", a:hasEventListener("gubar"))

a:removeEventListener(Event.ENTER_FRAME, onEnterFrame, a)
print("false:", a:hasEventListener(Event.ENTER_FRAME))




--[[
function onTimer(self, event)
   print("timer event")
end

function onTimerComplete(self, event)
   print("timerComplete event")
end

local t = Timer.new(1000, 3)
t:addEventListener(TimerEvent.TIMER, onTimer, t)
t:addEventListener(TimerEvent.TIMER_COMPLETE, onTimerComplete, t)
t:start()
--]]


--[[
print(t:running())

t:stop()

print(t:running())

print(Timer.running(t))
--]]

--[[
do
   function onTimer()
	  collectgarbage()
   end
   
   local t = Timer.new(1);
   t:addEventListener(TimerEvent.TIMER, onTimer, t)
   t:start()
end
--]]


Timer.__get = {
   running = Timer.getRunning,
   delay = Timer.getDelay
}

Timer.__set = {
   delay = Timer.setDelay
}

Timer.__oldindex = Timer.__index

Timer.__index = function (t, k)
				   local mt = getmetatable(t)

				   local f = mt.__get[k]
				   if f ~= nil then
					  return f(t)
				   end

				   mt.__index, mt.__oldindex = mt.__oldindex, mt.__index --swap indices
				   local result = t[k]
				   mt.__index, mt.__oldindex = mt.__oldindex, mt.__index --swap indices back
				   return result
				end

Timer.__newindex = function (t, k, v)
				   local mt = getmetatable(t)

				   local f = mt.__set[k]
				   if f ~= nil then
					  f(t, v)
					  return
				   end

				   local newindex = mt.__newindex
				   mt.__newindex = nil --disable __newindex
				   t[k] = v
				   mt.__newindex = newindex --enable __newindex back
				end

t = Timer.new(0)
--[[
print(t:getRunning())
print(t.running)
t:start()
print(t:getRunning())
print(t.running)


t:setDelay(1000)
print(t:getDelay())
print(t.delay)

--]]

t:setDelay(500)
t.delay = 1000
print(t:getDelay())
print(t.delay)

--setmetatable(t, nil)
--print(t.delay)

t.a = "a"
print(t.a)



