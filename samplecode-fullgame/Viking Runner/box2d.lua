require "box2d"

Box2d = Core.class(Sprite)

function Box2d:init(debug)

	--create world instance
	self.world = b2.World.new(0, 10, true)
	
	local screenW = application:getContentWidth()
	local screenH = application:getContentHeight()

	
	--create ball
	--self.ball = self:ball(160, 20)
	
	--create empty box2d body for joint
	--since mouse cursor is not a body
	--we need dummy body to create joint
	local ground = self.world:createBody({})
	
	--joint with dummy body
	--local mouseJoint = nil
	--[[
	-- create a mouse joint on mouse down
	function self:onMouseDown(event)
		if self.ball:hitTestPoint(event.x, event.y) then
			local jointDef = b2.createMouseJointDef(ground, self.ball.body, event.x, event.y, 100000)
			mouseJoint = self.world:createJoint(jointDef)
		end
	end
	
	-- update the target of mouse joint on mouse move
	function self:onMouseMove(event)
		if mouseJoint ~= nil then
			mouseJoint:setTarget(event.x, event.y)
		end
	end
	
	-- destroy the mouse joint on mouse up
	function self:onMouseUp(event)
		if mouseJoint ~= nil then
			self.world:destroyJoint(mouseJoint)
			mouseJoint = nil
		end
	end
]]
	-- register for mouse events
	--self:addEventListener(Event.MOUSE_DOWN, self.onMouseDown, self)
	--self:addEventListener(Event.MOUSE_MOVE, self.onMouseMove, self)
	--self:addEventListener(Event.MOUSE_UP, self.onMouseUp, self)
	
	if debug then
		
		--set up debug drawing
		local debugDraw = b2.DebugDraw.new()
		
		self.world:setDebugDraw(debugDraw)
		self:addChild(debugDraw)
		
	end
	
	local function onBeginContact(event)
		-- you can get the fixtures and bodies in this contact like:
		local fixtureA = event.fixtureA
		local fixtureB = event.fixtureB
		local bodyA = fixtureA:getBody()
		local bodyB = fixtureB:getBody()
		
		print("begin contact: ", bodyA.name, "<->", bodyB.name)
		if bodyA.type == "Collectable" then print(bodyA.score, "POINTS!!!") end
		if bodyA.type == "Nasty" then print("OUCH!!!") end
		if bodyA.name == "bottom" then print("Rock Bottom!!!") end
	end

	local function onEndContact(event)
		-- you can get the fixtures and bodies in this contact like:
		local fixtureA = event.fixtureA
		local fixtureB = event.fixtureB
		local bodyA = fixtureA:getBody()
		local bodyB = fixtureB:getBody()
		
		print("end contact: ", bodyA.name, "<->", bodyB.name)
	end

	local function onPreSolve(event)
		-- you can get the fixtures and bodies in this contact like:
		local fixtureA = event.fixtureA
		local fixtureB = event.fixtureB
		local bodyA = fixtureA:getBody()
		local bodyB = fixtureB:getBody()
		
		print("pre solve: ", bodyA.name, "<->", bodyB.name)
	end

	local function onPostSolve(event)
		-- you can get the fixtures and bodies in this contact like:
		local fixtureA = event.fixtureA
		local fixtureB = event.fixtureB
		local bodyA = fixtureA:getBody()
		local bodyB = fixtureB:getBody()

		print("post solve: ", bodyA.name, "<->", bodyB.name)
	end

	-- register 4 physics events with the world object
	self.world:addEventListener(Event.BEGIN_CONTACT, onBeginContact)
	--self.world:addEventListener(Event.END_CONTACT, onEndContact)
	--self.world:addEventListener(Event.PRE_SOLVE, onPreSolve)
	--self.world:addEventListener(Event.POST_SOLVE, onPostSolve)
	
	--run world
	self:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
	--remove event on exiting scene
	self:addEventListener("exitBegin", self.onExitBegin, self)
	
end

-- for creating objects using shape
-- as example - bounding walls
function Box2d:object(x, y, width, height, name, type, score)
	
	local object = Shape.new()
	
	--define object shape
	object:beginPath()
	
	--we make use (0, 0) as center of shape,
	--thus we have half of width and half of height in each direction
	object:moveTo(-width/2, -height/2)
	object:lineTo(width/2, -height/2)
	object:lineTo(width/2, height/2)
	object:lineTo(-width/2, height/2)
	object:closePath()
	object:endPath()
	object:setPosition(x,y)
	
	--create box2d physical object
	local body = self.world:createBody{type = b2.STATIC_BODY}
	
	body:setPosition(object:getX(), object:getY())
	body:setAngle(object:getRotation() * math.pi/180)
	
	local poly = b2.PolygonShape.new()
	
	poly:setAsBox(object:getWidth()/2, object:getHeight()/2)
	
	local fixture = body:createFixture{shape = poly, density = 1.0, 
	friction = 0.1, restitution = 0.8}
	
	object.body = body
	object.body.name = name
	object.body.type = type
	object.body.score = score
	
	--add to scene
	self:addChild(object)
	
	--return created object
	return object
	
end

-- for creating objects using image
-- as example - ball
function Box2d:ball(x, y)
--[[
	--create ball bitmap object from ball graphic
	local ball = Bitmap.new(Texture.new("./ball.png"))
	
	--reference center of the ball for positioning
	ball:setAnchorPoint(0.5, 0.5)
	
	ball:setPosition(x,y)
	
	--get radius
	local radius = ball:getWidth()/2
	
	--create box2d physical object
	local body = self.world:createBody{type = b2.DYNAMIC_BODY}
	
	body:setPosition(ball:getX(), ball:getY())
	body:setAngle(ball:getRotation() * math.pi/180)
	
	local circle = b2.CircleShape.new(0, 0, radius)
	
	local fixture = body:createFixture{shape = circle, density = 1.0, 
	friction = 0.1, restitution = 0.8}
	
	ball.body = body
	ball.body.type = "ball"
	ball.body.name = "ball"
	
	--add to scene
	self:addChild(ball)
	
	
	--return created object
	return ball
	]]
end

--running the world
function Box2d:onEnterFrame() 

	-- edit the step values if required. These are good defaults!
	self.world:step(1/60, 8, 3)
	
	--iterate through all child sprites
	for i = 1, self:getNumChildren() do
	
		--get specific sprite
		local sprite = self:getChildAt(i)
		
		-- check if sprite HAS a body (ie, physical object reference we added)
		if sprite.body then
		
			--update position to match box2d world object's position
			--get physical body reference
			local body = sprite.body
			
			--get body coordinates
			local bodyX, bodyY = body:getPosition()
			
			--apply coordinates to sprite
			sprite:setPosition(bodyX, bodyY)
			
			--apply rotation to sprite
			sprite:setRotation(body:getAngle() * 180 / math.pi)
		end
		
	end
	
end

--removing event on exiting scene
--just in case you're using SceneManager
function Box2d:onExitBegin()

  self:removeEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
  
end
