--!NEEDS:uiinit.lua

UI.Animation={_animated={} }
function UI.Animation:animate(sprite,channel,animation,duration,parameters)
	local spa=sprite.__UI_Animations or {}
	sprite.__UI_Animations=spa
	local anim=animation.new(sprite,duration,parameters)
	if spa[channel] then
		spa[channel]:stop()
	end
	spa[channel]=anim
	self._animated[sprite]=true
end

Core.asyncCall(function()
	while true do
		Core.yield(true)
		local sended={}
		for s,_ in pairs(UI.Animation._animated) do
			local ended={}
			for channel,anim in pairs(s.__UI_Animations) do
				if anim:tick() then
					anim:stop()
					ended[channel]=true
				end
			end
			for channel,_ in pairs(ended) do
				s.__UI_Animations[channel]=nil
			end
			if not next(s.__UI_Animations) then
				sended[s]=true
			end
		end
		for s,_ in pairs(sended) do
			UI.Animation._animated[s]=nil			
			s.__UI_Animations=nil
		end
	end
end)

--Base class for animations
UI.Animation.Animation=Core.class(Object)
function UI.Animation.Animation:init(sprite,duration,parameters)
	self.sprite=sprite
	self.duration=duration
	self.started=os:timer()
	self.onStop=parameters.onStop
	if parameters.easing then
		self.easing=parameters.easing
		assert(self.easing.type,"Easing type not specified")
	end
end
function UI.Animation.Animation:ease(ratio)
	if not self.easing then return ratio end --Linear
	return UI.Animation.Easing.ease(self.easing.type,self.easing.way,ratio)
end
function UI.Animation.Animation:tick()
	return true --Ended
end
function UI.Animation.Animation:stop()
	if self.onStop then self.onStop() end
end

local easing = {}
UI.Animation.Easing=easing

-- Move: Manipulate the anchor position to create sliding effects
-- ARGS: x,y: Target Position, Source is current one
UI.Animation.AnchorMove=Core.class(UI.Animation.Animation)
function UI.Animation.AnchorMove:init(sprite,duration,parameters)
	self.tx=parameters.x or 0
	self.ty=parameters.y or 0
	local cx,cy=sprite:getAnchorPosition()
	self.ox=cx
	self.oy=cy
	self.dx,self.dy=self.tx-cx,self.ty-cy
end

function UI.Animation.AnchorMove:tick()
	local ratio=(((os:timer()-self.started)/self.duration)<>0)><1
	local i=self:ease(ratio)
	local ancx,ancy=self.ox+self.dx*i,self.oy+self.dy*i
	self.sprite:setAnchorPosition(ancx,ancy)
	if ratio>=1 then
		return true
	end
end

-- Alpha: Set the alpha value of main color
UI.Animation.Alpha=Core.class(UI.Animation.Animation)
function UI.Animation.Alpha:init(sprite,duration,parameters)
end

function UI.Animation.Alpha:tick()
	local ratio=(((os:timer()-self.started)/self.duration)<>0)><1
	local i=self:ease(ratio)
	self.sprite:setColorTransform(1,1,1,i)
	if ratio>=1 then
		return true
	end
end

-- Alpha: Set the alpha value of main color
UI.Animation.AnchorScale=Core.class(UI.Animation.Animation)
function UI.Animation.AnchorScale:init(sprite,duration,parameters)
end

function UI.Animation.AnchorScale:tick()
	local w,h=self.sprite:getDimensions()
	local ratio=(((os:timer()-self.started)/self.duration)<>0)><1
	local i=self:ease(ratio)
	self.sprite:setAnchorPosition(-w*(1-i)*.5/i,-h*(1-i)*.5/i)
	self.sprite:setScale(i,i,i)
	if ratio>=1 then
		return true
	end
end


--Easing library
function easing.reverse(f, t, ...)
	return 1 - f(1 - t, ...)
end

function easing.inout(f, t, ...)
	if t < .5 then
		return .5 * f(t * 2, ...)
	else
		t = 1 - t
		return .5 * (1 - f(t * 2, ...)) + .5
	end
end

function easing.outin(f, t, ...)
	if t < .5 then
		return .5 * (1 - f(1 - t * 2, ...))
	else
		t = 1 - t
		return .5 * (1 - (1 - f(1 - t * 2, ...))) + .5
	end
end

--ease any interpolation function
function easing.ease(f, way, t, ...)
	f = easing[f] or f
	if way == 'out' then
		return easing.reverse(f, t, ...)
	elseif way == 'inout' then
		return easing.inout(f, t, ...)
	elseif way == 'outin' then
		return easing.outin(f, t, ...)
	else
		return f(t, ...)
	end
end

--auto-updating names table sorted in insert order (for listing)
easing.names = {}
function easing:__newindex(name, func)
	table.insert(self.names, name)
	rawset(self, name, func)
end
setmetatable(easing, easing)

--actual easing functions

function easing.linear(t) return t end
function easing.quad  (t) return t^2 end
function easing.cubic (t) return t^3 end
function easing.quart (t) return t^4 end
function easing.quint (t) return t^5 end
function easing.expo  (t) return 2^(10 * (t - 1)) end
function easing.sine  (t) return -math.cos(t * (math.pi * .5)) + 1 end
function easing.circ  (t) return -(math.sqrt(1 - t^2) - 1) end
function easing.back  (t) return t^2 * (2.7 * t - 1.7) end

function easing.steps (t, steps)
	steps = steps or 10
	return math.floor(t * steps) / (steps - 1)
end

-- a: amplitude, p: period
function easing.elastic(t, a, p)
	if t == 0 then return 0 end
	if t == 1 then return 1 end
	p = p or 0.3
	local s
	if not a or a < 1 then
		a = 1
		s = p / 4
	else
		s = p / (2 * math.pi) * math.asin(1 / a)
	end
	t = t - 1
	return -(a * 2^(10 * t) * math.sin((t - s) * (2 * math.pi) / p))
end

function easing.bounce(t)
	if t < 1 / 2.75 then
		return 7.5625 * t^2
	elseif t < 2 / 2.75 then
		t = t - 1.5 / 2.75
		return 7.5625 * t^2 + 0.75
	elseif t < 2.5 / 2.75 then
		t = t - 2.25 / 2.75
		return 7.5625 * t^2 + 0.9375
	else
		t = t - 2.625 / 2.75
		return 7.5625 * t^2 + 0.984375
	end
end

function easing.slowmo(t, power, ratio, yoyo)
	power = power or .8
	ratio = math.min(ratio or .7, 1)
	local p = ratio ~= 1 and power or 0
	local p1 = (1 - ratio) / 2
	local p2 = ratio
	local p3 = p1 + p2
	local r = t + (.5 - t) * p
	if t < p1 then
		local pt = 1 - (t / p1)
		return yoyo and 1 - pt^2 or r - pt^4 * r
	elseif t > p3 then
		local pt = (t - p3) / p1
		return yoyo and (t == 1 and 0 or 1 - pt^2) or r + ((t - r) * pt^4)
	else
		return yoyo and 1 or r
	end
end
