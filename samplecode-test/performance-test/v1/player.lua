local texture = Texture.new("gfx/tyrian.shp.007D3C.png", true, {transparentColor = 0xbfdcbf})

Player = Core.class(Sprite)

function Player:init(game)
	self.game = game
	self.world = game.world

	local body = self.world:createBody{type = b2.DYNAMIC_BODY, position = {x = 160, y = 400}}
	local shape = b2.CircleShape.new(0, 0, 4)
	body:createFixture({shape = shape, density = 1, filter = {categoryBits = 2, maskBits = 2}})
	
	self.body = body
	body.sprite = self
	
	self.type = "player"

	self.frames = {
		TextureRegion.new(texture, 24 * 0, 84, 24, 28),
		TextureRegion.new(texture, 24 * 1, 84, 24, 28),
		TextureRegion.new(texture, 24 * 2, 84, 24, 28),
		TextureRegion.new(texture, 24 * 3, 84, 24, 28),
		TextureRegion.new(texture, 24 * 4, 84, 24, 28),
	}
	
	self.bitmap = Bitmap.new(self.frames[3])
	self.bitmap:setAnchorPoint(0.48, 0.4)
	
	self:addChild(self.bitmap)
	
	self:addEventListener(Event.MOUSE_DOWN, self.onMouseDown, self)
	self:addEventListener(Event.MOUSE_MOVE, self.onMouseMove, self)
	self:addEventListener(Event.MOUSE_UP, self.onMouseUp, self)
	
	self:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
end

function Player:onMouseDown(event)
	self.mx = event.x
	self.my = event.y
end

function Player:onMouseMove(event)
	local x, y = self.body:getPosition()
	x = x - self.mx + event.x
	y = y - self.my + event.y
	self.body:setPosition(x, y)
	
	self.mx = event.x
	self.my = event.y
end

function Player:onMouseUp()
end

function Player:onEnterFrame(event)
	self:setPosition(self.body:getPosition())

	self.frame = self.frame or 0
	
	self.frame = self.frame + 1
	
	if self.frame == 3 then
		local x, y = self:getPosition()

		self.game:addPlayerBullet(1, x, y,  0, -20)
		self.game:addPlayerBullet(2, x, y, -8, -16)
		self.game:addPlayerBullet(3, x, y,  8, -16)
		self.game:addPlayerBullet(4, x, y, -14, -14)
		self.game:addPlayerBullet(5, x, y,  14, -14)
		
		self.frame = 0
	end
end
