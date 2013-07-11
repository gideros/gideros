Ball = gideros.class(Sprite)	
local physScale = 30

function Ball:init(world, x, y, type)
	self.world = world
	
	if (type == 2)	then 
		self.sprite = Bitmap.new(TextureRegion.new(Texture.new("ball40x40.png", true)))	
	else
		self.sprite = Bitmap.new(TextureRegion.new(Texture.new("ball_ice_02_40.png", true)))	
	end

	self:addChild(self.sprite)

	self.width = self.sprite:getWidth()
	self.height = self.sprite:getHeight()
	self.radius = self.width / 2

	local px = (x + self.width / 2) / physScale
	local py = (y + self.height / 2) / physScale
	local bodyDef = {
			type = b2.DYNAMIC_BODY,
			position = {x = px, y = py},
			bullet = true,
			}
	self.body = self.world:createBody(bodyDef)


	local bshape = b2.CircleShape.new( self.width/2/physScale,self.height/2/physScale, self.radius / physScale)
	self.fixtureDef = {shape = bshape, friction = 0, density = 1.0, restitution=1.0}

	self.fix = self.body:createFixture(self.fixtureDef)
	self.body:setLinearVelocity(2, 3)
	self.body.sprite = self.sprite


	self:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
--	self.world:addEventListener(b2ContactEvent.BEGIN_CONTACT, self.onBeginContact, self)	
--	self.world:addEventListener(b2ContactEvent.END_CONTACT, self.onEndContact, self)	
	
	self:createBoundaries()
end	


function Ball:createBoxBody(x, y, width, height)

	local bposition = {x=x / physScale, y=y / physScale}
	local bodyDef = {
		type = b2.STATIC_BODY,
		position = bposition
		}
	local body = self.world:createBody(bodyDef)
	local bShape = b2.PolygonShape.new()
	bShape:setAsBox(width / physScale / 2, height / physScale / 2)
	local fixtureDef = {shape = bShape}
	body:createFixture(fixtureDef)
	body.isBorder = true
	
	return body
end

function Ball:createBoundaries()
	self:createBoxBody(240, 0-25, 480+100, 50)

	self:createBoxBody(240, 320+25, 480+100, 50)	--
	
	self:createBoxBody(0-25, 160, 50, 320+100)
	self:createBoxBody(480+25, 160, 50, 320+100)
end

function Ball:onEnterFrame(event)
--	self.frame = self.frame + 1

	self.world:step(1/60, 10, 10)
	self.world:clearForces()
	
	if (self.body ~= nil) then 
		local x,y =  self.body:GetPosition()
	
		self.sprite:setX(x * physScale)
		self.sprite:setY(y * physScale)
	end		
	
end
