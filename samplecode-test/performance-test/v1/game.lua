require "box2d"

Game = Core.class(Sprite)

function Game:init()
	self.world = b2.World.new(0, 0)
	
	self.player = Player.new(self)
	self.enemy = Enemy.new(self)
	
	self.playerBullets = BulletLayer.new(self)
	self.enemyBullets = BulletLayer.new(self)
	
	self.explosionLayer = ExplosionLayer.new()
	
	self:addChild(self.player)
	self:addChild(self.enemy)
	self:addChild(self.playerBullets)
	self:addChild(self.enemyBullets)
	self:addChild(self.explosionLayer)
	
	self:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)

	self.world:addEventListener(Event.BEGIN_CONTACT, self.onBeginContact, self)
	
	self.toBeRemoved = {}

--[[
	local debugDraw = b2.DebugDraw.new()
	self:addChild(debugDraw)
	self.world:setDebugDraw(debugDraw)
--]]
end

function Game:addPlayerBullet(id, x, y, dx, dy)
	self.playerBullets:addBullet(id, x, y, dx, dy, "playerBullet", 1, 1)
end

function Game:addEnemyBullet(id, x, y, dx, dy)
	self.enemyBullets:addBullet(id, x, y, dx, dy, "enemyBullet", 2, 2)
end

function Game:getPlayerPosition()
	return self.player:getPosition()
end

function Game:onEnterFrame()
	for i=#self.toBeRemoved, 1, -1 do
		local sprite = self.toBeRemoved[i]
		if sprite.type == "playerBullet" then
			self.playerBullets:removeBullet(sprite)
		elseif sprite.type == "enemyBullet" then
			self.enemyBullets:removeBullet(sprite)
		end
		self.toBeRemoved[i] = nil
	end

	self.world:step(1/60, 3, 6)
end

function Game:onBeginContact(event)
	local fixtureA = event.fixtureA
	local fixtureB = event.fixtureB
	local bodyA = fixtureA:getBody()
	local bodyB = fixtureB:getBody()
	local spriteA = bodyA.sprite
	local spriteB = bodyB.sprite

	local bullet

	if spriteA.type == "playerBullet" or spriteA.type == "enemyBullet" then
		bullet = spriteA
	elseif spriteB.type == "playerBullet" or spriteB.type == "enemyBullet" then
		bullet = spriteB
	end

	if bullet ~= nil then
		self.toBeRemoved[#self.toBeRemoved + 1] = bullet
		local x,y = bullet:getPosition()
		self.explosionLayer:addExplosion(x + math.random(0, 10) - 5, y + math.random(0, 10) - 5)
		soundManager:play("hit")
	end
end
