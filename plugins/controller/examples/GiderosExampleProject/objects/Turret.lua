Turret = Core.class(Sprite)

function Turret:init(player)
	self.level = player.level
	self.player = player
	self.id = player.id
	self.type = player.type
	self.internalAngle = 0
	self.angle = self.player.body:getAngle()
	self.isShooting = false
	self.image = Bitmap.new(Texture.new("images/turret.png", true))
	self.image:setAnchorPoint(0.3, 0.5)
	self:addChild(self.image)
end

function Turret:setAngle(angle)
	self.internalAngle = (angle)
	self.angle = self.player.body:getAngle() + angle
	self:setRotation(math.deg(angle))
end

function Turret:getAngle()
	return self.internalAngle
end