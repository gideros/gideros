require "box2d"

local texture = Texture.new("gfx/newsh(.shp.000000.png", false, {transparentColor = 0xbfdcbf})

local bullets = {
	{13, 42, 10, 11, 3},	
	{120, 140, 6, 11, 3},
	{138, 140, 6, 11, 3},
	{120, 112, 11, 11, 3},
	{133, 112, 11, 11, 3},

	{207, 74, 7, 7, 3},
	{121, 172, 21, 21, 10}
}

BulletLayer = Core.class(Sprite)

function BulletLayer:init(game)
	self.game = game
	self.world = game.world
	self.pool = {}
	self.bullets = {}
	
	self:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
end

function BulletLayer:addBullet(id, x, y, dx, dy, type, categoryBits, maskBits)
	local bullet = bullets[id]

	local radius = bullet[5]
	
	self.pool[radius] = self.pool[radius] or {}
	
	local pool = self.pool[radius]

	local sprite
	if #pool == 0 then
		local textureRegion = TextureRegion.new(texture, bullet[1], bullet[2], bullet[3], bullet[4])
		sprite = Bitmap.new(textureRegion)
		sprite.textureRegion = textureRegion
		sprite:setAnchorPoint(0.5, 0.5)
		local body = self.world:createBody({type = b2.KINEMATIC_BODY})
		local shape = b2.CircleShape.new(0, 0, radius)
		local fixture = body:createFixture({shape = shape, isSensor = true, filter = {categoryBits = categoryBits, maskBits = maskBits}})
		sprite.body = body
		body.sprite = sprite
		sprite.type = type
		sprite.radius = radius
	else
		sprite = pool[#pool]
		sprite.textureRegion:setRegion(bullets[id][1], bullets[id][2], bullets[id][3], bullets[id][4])
		sprite:setTextureRegion(sprite.textureRegion)
		pool[#pool] = nil
		sprite.body:setActive(true)
	end

	sprite:setPosition(x, y)
	sprite.body:setPosition(x, y)
	sprite.body:setLinearVelocity(dx, dy)
	
	self.bullets[sprite] = true
	
	self:addChild(sprite)
end

function BulletLayer:removeBullet(s)
	local pool = self.pool[s.radius]
	s:removeFromParent()
	s.body:setActive(false)
	self.bullets[s] = nil
	pool[#pool + 1] = s
end

function BulletLayer:onEnterFrame()
	for s in pairs(self.bullets) do
		local x,y = s.body:getPosition()
		if y < 0 or x < 0 or x > 320 or y > 480 then
			self:removeBullet(s)
		else
			s:setPosition(x, y)
		end
	end
end
