--[[
Sprite.parent = Sprite.getParent
Sprite.getVisible = Sprite.isVisible
Sprite.getNumChildren = Sprite.numChildren

MouseEvent = {}
MouseEvent.MOUSE_DOWN = Event.MOUSE_DOWN
MouseEvent.MOUSE_UP = Event.MOUSE_UP
MouseEvent.MOUSE_MOVE = Event.MOUSE_MOVE

TimerEvent = {}
TimerEvent.TIMER = Event.TIMER
TimerEvent.TIMER_COMPLETE = Event.TIMER_COMPLETE

TouchEvent = {}
TouchEvent.TOUCHES_BEGIN =Event.TOUCHES_BEGIN
TouchEvent.TOUCHES_MOVE = Event.TOUCHES_MOVE
TouchEvent.TOUCHES_END = Event.TOUCHES_END
TouchEvent.TOUCHES_CANCEL = Event.TOUCHES_CANCEL

BitmapData = TextureRegion
--]]


gideros = {}
gideros.class = Core.class

Timer.pauseAllTimers = Timer.pauseAll
Timer.resumeAllTimers = Timer.resumeAll

local function delayedCallWrapper(timer)
	timer.func()
end

local function delayedCallWrapperWithData(timer)
	timer.func(timer.data)
end

function Timer.delayedCall(delay, func, data)
	local t = type(func)
	if t ~= "function" then
		error("bad argument #2 to 'delayedCall' (function expected, got "..t..")")
	end

	local timer = Timer.new(delay, 1)
	timer.func = func

	if data ~= nil then
		timer.data = data
		timer:addEventListener(Event.TIMER, delayedCallWrapperWithData, timer)
	else
		timer:addEventListener(Event.TIMER, delayedCallWrapper, timer)
	end

	timer:start()

	return timer
end

if not package then
	package={ preload={}, loaded={} }
	require=function(module)
		if package.loaded[module] then return package.loaded[module] end
		assert(package.preload[module],"Module "..module.." not found")
		assert(type(package.preload[module])=="function","Module loader isn't a function")
		local m=package.preload[module](module)
		package.loaded[module]=m or true
		return m
	end
end

package.preload["accelerometer"] = function()
	accelerometer = Accelerometer.new()
	return accelerometer
end

package.preload["gyroscope"] = function()
	gyroscope = Gyroscope.new()
	return gyroscope
end

package.preload["geolocation"] = function()
	geolocation = Geolocation.new()
	return geolocation
end


local accelerometer = nil
function Application:getAccelerometer()
	if accelerometer == nil then
		accelerometer = Accelerometer.new()
		accelerometer:start()
	end
	return accelerometer:getAcceleration()
end
