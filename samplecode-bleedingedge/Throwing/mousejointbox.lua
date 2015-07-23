MouseJointBox = gideros.class(Sprite)

function MouseJointBox:init(world, x, y)
	self.world = world
	
	self.body = self.world:createBody{type = b2.DYNAMIC_BODY, position = {x = x, y = y}}

	local shape = b2.PolygonShape.new()
	shape:setAsBox(40, 40)
	self.body:createFixture{shape = shape, density = 1, restitution = 0.2, friction = 0.3}

	local sprite = Bitmap.new(Texture.new("box.png"))
	sprite:setAnchorPoint(0.5, 0.5)
	self:addChild(sprite)

	self:addEventListener(Event.MOUSE_DOWN, self.onMouseDown, self)
	self:addEventListener(Event.MOUSE_MOVE, self.onMouseMove, self)
	self:addEventListener(Event.MOUSE_UP, self.onMouseUp, self)
end


function MouseJointBox:onMouseDown(event)
	if self:hitTestPoint(event.x, event.y) then
		local jd = b2.createMouseJointDef(self.world.ground, self.body, event.x, event.y, 10000)
		self.mouseJoint = self.world:createJoint(jd)
	end
end

function MouseJointBox:onMouseMove(event)
	if self.mouseJoint then
		self.mouseJoint:setTarget(event.x, event.y)
	end
end

function MouseJointBox:onMouseUp(event)
	if self.mouseJoint then
		self.world:destroyJoint(self.mouseJoint)
		self.mouseJoint = nil
	end
end
