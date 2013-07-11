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
	self.mesh = Mesh.new()
	self.mesh.ni = 0
	self.mesh.nv = 0
	self.mesh:setTexture(texture)
	self:addChild(self.mesh)
	self:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
end

function BulletLayer:addBullet(id, x, y, dx, dy, type, categoryBits, maskBits)
	local bullet = bullets[id]

	local tx = bullet[1]
	local ty = bullet[2]
	local w = bullet[3]
	local h = bullet[4]
	local radius = bullet[5]
	
	self.pool[radius] = self.pool[radius] or {}
	
	local pool = self.pool[radius]

	local sprite
	if #pool == 0 then
		local body = self.world:createBody({type = b2.KINEMATIC_BODY})
		local shape = b2.CircleShape.new(0, 0, radius)
		local fixture = body:createFixture({shape = shape, isSensor = true, filter = {categoryBits = categoryBits, maskBits = maskBits}})

		local nv = self.mesh.nv
		local ni = self.mesh.ni
		self.mesh:setIndices(ni + 1, nv + 1, ni + 2, nv + 2, ni + 3, nv + 3, ni + 4, nv + 1, ni + 5, nv + 3, ni + 6, nv + 4)
		self.mesh:setVertices(nv + 1, 0, 0, nv + 2, 0, 0, nv + 3, 0, 0, nv + 4, 0, 0)
		self.mesh.ni = ni + 6
		self.mesh.nv = nv + 4

		sprite = {nv = nv, ni = ni, body = body, type = type, radius = radius, x = x, y = y, w = w, h = h}
		
		body.sprite = sprite
	else
		sprite = pool[#pool]
		pool[#pool] = nil
		sprite.body:setActive(true)
	end

	local nv = sprite.nv
	self.mesh:setTextureCoordinates(nv + 1, tx, ty, nv + 2, tx + w, ty, nv + 3, tx + w, ty + h, nv + 4, tx, ty + h)

	sprite.body:setPosition(x, y)
	sprite.body:setLinearVelocity(dx, dy)

	self.bullets[sprite] = true
end

function BulletLayer:removeBullet(s)
	local pool = self.pool[s.radius]
	local nv = s.nv
	self.mesh:setVertices(nv + 1, 0, 0, nv + 2, 0, 0, nv + 3, 0, 0, nv + 4, 0, 0)
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
			local nv = s.nv
			local w = s.w / 2
			local h = s.h / 2
			self.mesh:setVertices(nv + 1, x - w, y - h, nv + 2, x + w, y - h, nv + 3, x + w, y + h, nv + 4, x - w, y + h)
			s.x = x
			s.y = y
		end
	end
end
