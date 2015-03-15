
SelectScene = Core.class(ControllerLayer)

function SelectScene:init()
	local curPlayer = 1
	if controller then
		local p = controller:getPlayers()
		if next(p) then
			curPlayer = p[1]
		end
	end
	print(curPlayer)
	self:setPlayer(curPlayer)
	--here we'd probably want to set up a background picture
	local screen = Bitmap.new(Texture.new("images/gideros_mobile.png", conf.textureFilter))
	self:addChild(screen)
	screen:setPosition((conf.width-screen:getWidth())/2, (conf.height-screen:getHeight())/2)
	self:addEventListener(Event.KEY_DOWN, self.onKeyDown, self)
	if self:isControllerMode() then
		self:addEventListener(Event.KEY_DOWN, self.detectPlayer, self)
		self:addEventListener(Event.LEFT_JOYSTICK, self.detectPlayer, self)
		self:addEventListener("selected", self.onSelected, self)
	end

	self.players = {false}
	self.playerSelect = {}
	self.layers = {self}
	self.cars = {}
	self:createCar(1)
	self:createCar(2)
	self:createCar(3)
	self:createCar(4)
	
	if self:isControllerMode() then
		local text = TextField.new(conf.largeFont, "Press O to select vehicle")
		--add menu buttons
		self:addChild(text)
		--position menu in center
		text:setPosition((conf.width-text:getWidth())/2, conf.height-text:getHeight())
	end
	
end

function SelectScene:onSelected(e)
	self.playerSelect[e.playerId] = e.object.id
end

function SelectScene:createCar(id)
	local player = Sprite.new()
	local car = Car.new()
	car:setRotation(-90)
	car:setScale(0.5)
	player:addChild(car)
	self:addObject(player)
	player.id = id
	player:setPosition(100+(id-1)*200, 250)
	self:addChild(player)
	local level = TextField.new(conf.smallFont, "Car "..id)
	level:setPosition(-level:getWidth()/2, 70)
	player:addChild(level)
	if not controller then
		player:addEventListener(Event.MOUSE_UP, function(self, e)
			if self:hitTestPoint(e.x, e.y) then
				sceneManager:changeScene("game", 0.1, conf.transition, conf.easing, {userData = {self.id}})
			end
		end, player)
	end
	self.cars[id] = player
end

function SelectScene:onKeyDown(e)
	if e.keyCode == KeyCode.BUTTON_A then	
		local ignore = false
		if self.cars[self.playerSelect[e.playerId]].selected then
				ignore = true
		else
			self.cars[self.playerSelect[e.playerId]].selected = true
			self.cars[self.playerSelect[e.playerId]]:setColorTransform(0,1,0,1)
		end
		
		if not ignore then
			self.players[e.playerId] = self.playerSelect[e.playerId]
			self.layers[e.playerId]:setHoverActive(false)
			for i = 1, #self.cars do
				self.layers[e.playerId]:removeObject(self.cars[i])
			end
		
			local gotoGame = true
			for i = 1, #self.players do
				if self.players[i] == false then
					gotoGame = false
				end
			end
			if gotoGame then
				sceneManager:changeScene("game", 0.1, conf.transition, conf.easing, {userData = self.players})
			end
		end
	end
end

function SelectScene:detectPlayer(e)
	if self.players[e.playerId] == nil then
		self.players[e.playerId] = false
		local layer = ControllerLayer.new()
		layer:setPlayer(e.playerId)
		self:addChild(layer)
		self.layers[e.playerId] = layer
		for i = 1, #self.cars do
			layer:addObject(self.cars[i])
		end
	end
end

function SelectScene:back()
	sceneManager:changeScene("start", 0.1, conf.transition, conf.easing)
end