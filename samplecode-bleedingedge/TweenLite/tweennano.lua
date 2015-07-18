--[[
based on 1.05
]]

TweenNano = {}
TweenNano.__index = TweenNano

TweenNano._masterList = {}
TweenNano._shape = Shape.new()
TweenNano._reservedProps = {ease=1, delay=1, useFrames=1, overwrite=1, onComplete=1, onCompleteParams=1, runBackwards=1, immediateRender=1, onUpdate=1, onUpdateParams=1}

function TweenNano.new(...)
	local self = setmetatable({}, TweenNano)
	self:init(...)
	return self
end

function TweenNano:init(target, duration, vars)
	if not TweenNano._tnInitted then
		TweenNano._time = os.clock()
		TweenNano._frame = 0
		TweenNano._shape:addEventListener(Event.ENTER_FRAME, TweenNano.updateAll)
		TweenNano._tnInitted = true;
	end

	self.vars = vars
	self.duration = duration
	self.active = duration == 0 and self.vars.delay == 0 and self.vars.immediateRender ~= false
	self.target = target	
	if type(self.vars.ease) ~= "function" then
		self._ease = TweenNano.easeOut
	else
		self._ease = self.vars.ease
	end
	self._propTweens = {}
	self.useFrames = vars.useFrames == true
	local delay = self.vars.delay or 0
	self.startTime = self.useFrames and TweenNano._frame + delay or TweenNano._time + delay

	local a = TweenNano._masterList[target]
	if a == nil or self.vars.overwrite == true or self.vars.overwrite == nil then
		TweenNano._masterList[target] = {self};
	else
		a[#a + 1] = self;
	end

	if self.vars.immediateRender == true or self.active then
		self:renderTime(0)
	end
end

function TweenNano:init2()
	for p in pairs(self.vars) do
		if TweenNano._reservedProps[p] == nil then
			self._propTweens[#self._propTweens + 1] = {p, self.target:get(p), type(self.vars[p]) == "number" and self.vars[p] - self.target:get(p) or tonumber(self.vars[p])}
		end
	end
	
	if self.vars.runBackwards then
		for i = 1, #self._propTweens do
			local pt = self._propTweens[i]
			pt[2] = pt[2] + pt[3]
			pt[3] = -pt[3]
		end
	end
	
	self._initted = true
end

local function unpack2(t)
	if t then
		return unpack(t)
	end
end

function TweenNano:renderTime(time)
	if not self._initted then
		self:init2()
	end
	
	if time >= self.duration then
		time = self.duration
		self.ratio = 1
	elseif time <= 0 then
		self.ratio = 0
	else
		self.ratio = self._ease(time, 0, 1, self.duration)
	end

	for i=1,#self._propTweens do
		local pt = self._propTweens[i]
		self.target:set(pt[1], pt[2] + (self.ratio * pt[3]))
	end

	if self.vars.onUpdate then
		self.vars.onUpdate(unpack2(self.vars.onUpdateParams))
	end
	if time == self.duration then
		self:complete(true)
	end
end

function TweenNano:complete(skipRender)
	if not skipRender then
		self:renderTime(self.duration)
		return
	end
	self:kill()
	if self.vars.onComplete then
		self.vars.onComplete(unpack2(self.vars.onCompleteParams))
	end
end

function TweenNano:kill()
	self.gc = true
	self.active = false
end

function TweenNano.to(target, duration, vars)
	return TweenNano.new(target, duration, vars)
end


function TweenNano.from(target, duration, vars)
	vars.runBackwards = true;
	if vars.immediateRender == nil then
		vars.immediateRender = true
	end
	return TweenNano.new(target, duration, vars)
end

function TweenNano.delayedCall(delay, onComplete, onCompleteParams, useFrames)
	return TweenNano.new(onComplete, 0, {delay=delay, onComplete=onComplete, onCompleteParams=onCompleteParams, useFrames=useFrames, overwrite=false});
end

function TweenNano.updateAll()
	TweenNano._frame = TweenNano._frame + 1
	TweenNano._time = os.clock()

	local ml = TweenNano._masterList
	for tgt,a in pairs(ml) do
		for i=#a,1,-1 do
			local tween = a[i]
			local t = tween.useFrames and TweenNano._frame or TweenNano._time
			if tween.active or (not tween.gc and t >= tween.startTime) then
				tween:renderTime(t - tween.startTime)
			elseif tween.gc then
				table.remove(a, i)
			end
		end	
		
		if #a == 0 then
			ml[tgt] = nil
		end
	end
end

function TweenNano:killTweensOf(target, complete)
	local a = TweenNano._masterList[target]
	if a ~= nil then
		if complete then
			for i=1,#a do
				if not a[i].gc then
					a[i].complete(false)
				end
			end
		end
		
		TweenNano._masterList[target] = nil
	end
end

function TweenNano.easeOut(t, b, c, d)
	return -1 * (t/d) * ((t/d) - 2)	
end
