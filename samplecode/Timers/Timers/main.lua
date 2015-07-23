--[[

An example showing timer capabilities of Gideros Mobile
(C) 2010-2011 Gideros Mobile

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

timer = Timer.new(1000, 5)

print(timer)

function onTimer(e)
	print("timer", e:getTarget(), e:getType())
end

function onTimerComplete(e)
	print("timer complete", e:getTarget(), e:getType())
end

timer:addEventListener(Event.TIMER, onTimer)
timer:addEventListener(Event.TIMER_COMPLETE, onTimerComplete)
timer:start()

