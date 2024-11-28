local texture = Texture.new("gfx/explosions/explosions_ts.png", false, {transparentColor = 0xbfdcbf})

ExplosionLayer = Core.class(Sprite)

function ExplosionLayer:init()
	self.pool = {}
end

function ExplosionLayer:addExplosion(x, y)
	local explosion
	if #self.pool == 0 then
		local timeline = {}
		for i = 1, 13 do
			local bitmap = Bitmap.new(TextureRegion.new(texture, 73 + 12 * (i - 1), 85, 12, 12))
			bitmap:setAnchorPoint(0.5, 0.5)
			timeline[i] = {i, i, bitmap}
		end
		explosion = MovieClip.new(timeline)
		explosion:addEventListener(Event.COMPLETE, self.onExplosionComplete, self)
	else
		explosion = self.pool[#self.pool]
		self.pool[#self.pool] = nil
	end
	explosion:setPosition(x, y)
	explosion:gotoAndPlay(1)
	self:addChild(explosion)
end

function ExplosionLayer:onExplosionComplete(event)
	local explosion = event:getTarget()
	explosion:removeFromParent()
	self.pool[#self.pool+1] = explosion
end
