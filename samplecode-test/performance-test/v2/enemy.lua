local texture = Texture.new("gfx/newshh.shp.000000.png", true, {transparentColor = 0xbfdcbf})

Enemy = Core.class(Sprite)

function Enemy:init(game)
	self.game = game
	self.world = game.world
		
	local body = self.world:createBody{type = b2.DYNAMIC_BODY, position = {x = 160, y = 100}}
	local shape = b2.PolygonShape.new()
	shape:setAsBox(67/2, 54/2)
	body:createFixture({shape = shape, density = 1, filter = {categoryBits = 1, maskBits = 1}})
	
	self.body = body
	body.sprite = self
	
	self.type = "enemy"

	local bitmap = Bitmap.new(TextureRegion.new(texture, 75, 113, 67, 54))
	bitmap:setAnchorPoint(0.5, 0.5)
	self:addChild(bitmap)
	
	self:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)

	self.attacks = {self.attack1, self.attack2, self.attack3}
end

function Enemy:onEnterFrame()
	self.angle = self.angle or 0
	self.body:setPosition(160 + 100 * math.sin(self.angle), 100 + 10 * math.cos(self.angle))
	self.angle = self.angle + 0.02

	self:setPosition(self.body:getPosition())

	if self.frame == nil then
		self.frame = 0
		self.attack = self.attacks[math.random(1, #self.attacks)]
	end
	
	self:attack()

	if self.frame ~= nil then
		self.frame = self.frame + 1
	end
end

function Enemy:attack1()
	if self.frame < 70 and self.frame % 10 == 2 then
		local x,y = self:getPosition()
		for i=0,350,10 do
			self.game:addEnemyBullet(6, x, y, 5 * math.cos(i * math.pi / 180), 5 * math.sin(i * math.pi / 180))
		end
	end

	if self.frame == 100 then
		self.frame = nil
	end
end

function Enemy:attack2()
	if self.frame >= 0 and self.frame <= 180 then
		local x,y = self:getPosition()
		self.game:addEnemyBullet(6, x, y, 5 * math.cos(self.frame * math.pi / 22.5), 5 * math.sin(self.frame  * math.pi / 22.5))
		self.game:addEnemyBullet(6, x, y, -5 * math.cos(self.frame * math.pi / 22.5), -5 * math.sin(self.frame  * math.pi / 22.5))
	end

	if self.frame == 300 then
		self.frame = nil
	end
end


function Enemy:attack3()
	local x1,y1 = self:getPosition()
	local x2,y2 = self.game:getPlayerPosition()
	
	if self.frame < 70 and self.frame % 10 == 5 then
		local x1,y1 = self:getPosition()
		local x2,y2 = self.game:getPlayerPosition()
		local dx = x2 - x1
		local dy = y2 - y1
		local len = math.sqrt(dx * dx + dy * dy)
		dx = (dx / len) * 15
		dy = (dy / len) * 15	
		self.game:addEnemyBullet(7, x1, y1, dx, dy)
	end

	if self.frame == 100 then
		self.frame = nil
	end
end
