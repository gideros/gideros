local function bezier3(p1,p2,p3,mu)
   local mum1,mum12,mu2
   local p = {}
   mu2 = mu * mu
   mum1 = 1 - mu
   mum12 = mum1 * mum1
   p.x = p1.x * mum12 + 2 * p2.x * mum1 * mu + p3.x * mu2
   p.y = p1.y * mum12 + 2 * p2.y * mum1 * mu + p3.y * mu2
   return p
end

local function getProperties(elem)
	local animate = {}
	animate.alpha = elem:getAlpha()
	animate.scaleX = elem:getScaleX()
	animate.scaleY = elem:getScaleY()
	animate.rotation = elem:getRotation()
	animate.x = elem:getX()
	animate.y = elem:getY()
	return animate
end

Popup = Core.class(Shape)

function Popup:init(config)
	--default configuration
	self.conf = {
		scaleX = 0,
		scaleY = 0,
		x = application:getContentWidth()/2, 
		y = application:getContentHeight()/2,
		width = application:getContentWidth(),
		height = application:getContentHeight(),
		radius = 50,
		easing = nil,
		animate = true,
		duration = 1,
		reverse = true
	}
	self:addEventListener(Event.MOUSE_DOWN, function(e)
		if self:hitTestPoint(e.x, e.y) then
			e:stopPropagation()
		end
	end)
	self:addEventListener(Event.MOUSE_MOVE, function(e)
		if self:hitTestPoint(e.x, e.y) then
			e:stopPropagation()
		end
	end)
	self:addEventListener(Event.MOUSE_UP, function(e)
		if self:hitTestPoint(e.x, e.y) then
			e:stopPropagation()
		end
	end)
	self:addEventListener(Event.TOUCHES_BEGIN, function(e)
		if self:hitTestPoint(e.touch.x, e.touch.y) then
			e:stopPropagation()
		end
	end)
	self:addEventListener(Event.TOUCHES_MOVE, function(e)
		if self:hitTestPoint(e.touch.x, e.touch.y) then
			e:stopPropagation()
		end
	end)
	self:addEventListener(Event.TOUCHES_END, function(e)
		if self:hitTestPoint(e.touch.x, e.touch.y) then
			e:stopPropagation()
		end
	end)
	
	if config then
		--copying configuration
		for key,value in pairs(config) do
			self.conf[key]= value
		end
	end
	
	
	
	--some properties
	self.fill = nil
	self.line = nil
	
	self:setVisible(false)
	self:draw()
end

function Popup:draw()
	--shape
	self:clear()
	if self.fill then
		Shape.setFillStyle(self,unpack(self.fill))
	end
	if self.line then
		Shape.setLineStyle(self,unpack(self.line))
	end
	self:beginPath()
	if(self.conf.radius == 0)then
		self:moveTo(-self.conf.width/2, -self.conf.height/2)
		self:lineTo(-self.conf.width/2, self.conf.height/2)
		self:lineTo(self.conf.width/2, self.conf.height/2)
		self:lineTo(self.conf.width/2, -self.conf.height/2)
		self:lineTo(-self.conf.width/2, -self.conf.height/2)
	else
		self:moveTo(-self.conf.width/2, -self.conf.height/2 + self.conf.radius)
		self:lineTo(-self.conf.width/2, self.conf.height/2 - self.conf.radius)
		self:quadraticCurveTo(-self.conf.width/2, self.conf.height/2 - self.conf.radius,
			-self.conf.width/2, self.conf.height/2, 
			-self.conf.width/2 + self.conf.radius, self.conf.height/2)
		self:lineTo(self.conf.width/2 - self.conf.radius, self.conf.height/2)
		self:quadraticCurveTo(self.conf.width/2 - self.conf.radius, self.conf.height/2,
			self.conf.width/2, self.conf.height/2, 
			self.conf.width/2, self.conf.height/2 - self.conf.radius)
		self:lineTo(self.conf.width/2, -self.conf.height/2 + self.conf.radius)
		self:quadraticCurveTo(self.conf.width/2, -self.conf.height/2 + self.conf.radius,
			self.conf.width/2, -self.conf.height/2, 
			self.conf.width/2 - self.conf.radius, -self.conf.height/2)
		self:lineTo(-self.conf.width/2 + self.conf.radius, -self.conf.height/2)
		self:quadraticCurveTo(-self.conf.width/2 + self.conf.radius, -self.conf.height/2,
			-self.conf.width/2, -self.conf.height/2, 
			-self.conf.width/2, -self.conf.height/2 + self.conf.radius)
	end
	self:endPath()
end

function Popup:setFillStyle(...)
	self.fill = arg
	self:draw()
end

function Popup:setLineStyle(...)
	self.line = arg
	self:draw()
end

function Popup:hide()
	if self.conf.animate and (self.original)then
		local tween = GTween.new(self, self.conf.duration, self.original, {ease = self.conf.easing, dispatchEvents = true})
		tween:addEventListener("complete", function()
			self:setVisible(false)
		end)
	else
		self:setVisible(false)
	end
end

function Popup:show()
	self:setVisible(true)
	if self.conf.animate then
		--default properties
		local animate = {}
		animate.alpha = 1
		animate.scaleX = 1
		animate.scaleY = 1
		animate.rotation = 0
		animate.y = self.conf.y
		animate.x = self.conf.x
		
		--copy original properties
		self.original = getProperties(self)
		
		--animate
		GTween.new(self, self.conf.duration, animate, {ease = self.conf.easing})
	end	
end

function Popup:quadraticCurveTo(lastX, lastY, cpx, cpy, x, y, mu)
	local inc = mu or 0.1 -- need a better default
	for i = 0,1,inc do
	local p = bezier3(
		{ x=lastX, y=lastY },
		{ x=cpx, y=cpy },
		{ x=x, y=y },
		i)
	self:lineTo(p.x,p.y)
	end
end

function Popup:getWidth()
	return self.conf.width
end

function Popup:getHeight()
	return self.conf.height
end