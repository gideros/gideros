GameScene = Core.class(ControllerLayer)

function GameScene:init(p)
	self.p = p
	self.layer = Sprite.new()
	self:addChild(self.layer)
	self.bg = Shape.new()
	self.worldW = conf.width + conf.dx
	self.worldH = conf.height + conf.dy
	local texture = Texture.new("images/tile1.png", true, {wrap = Texture.REPEAT})
	self.bg:setFillStyle(Shape.TEXTURE, texture)
	self.bg:beginPath()
	self.bg:moveTo(-conf.dx, -conf.dy)
	self.bg:lineTo(-conf.dx, self.worldH)
	self.bg:lineTo(self.worldW, self.worldH)
	self.bg:lineTo(self.worldW, -conf.dy)
	self.bg:closePath()
	self.bg:endPath()
	self.layer:addChild(self.bg)
	
	self.isPaused = false
	self.world = b2.World.new(0, 0, true)
	self.bodies = Sprite.new()
	self.layer:addChild(self.bodies)
	self.players = {}
	for i = 1, #p do
		self.players[i] = Player.new(self, 100 + i*200, 400)
	end
	
	local s = Sprite.new()
	self:addChild( s)
	if not self:isControllerMode() then
		--Add on screen joysticks here
	end
	self:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
	self:addEventListener("exitBegin", self.onExitBegin, self)
	if controller then
		self:addEventListener(Event.LEFT_JOYSTICK, self.onLeftJoystick, self)
		self:addEventListener(Event.RIGHT_JOYSTICK, self.onRightJoystick, self)
		self:addEventListener(Event.DISCONNECTED, self.onDisconnect, self)
		self:addEventListener(Event.KEY_DOWN, self.handleKeys, self)
	end
	
	local debugDraw = b2.DebugDraw.new()
	self.world:setDebugDraw(debugDraw)
	self.layer:addChild(debugDraw)
	
end

function GameScene:onDisconnect(e)
	if self.players[e.playerId] ~= nil then
		self:openMenu()
	end
end

function GameScene:handleKeys(e)
	if e.keyCode == KeyCode.BUTTON_MENU then
		self:openMenu()
	end
end

function GameScene:pauseGame()
	self.isPaused = true
	Timer.pauseAll();
end

function GameScene:unpauseGame()
	self.isPaused = false
	Timer.resumeAll()
end

function GameScene:openMenu()
	if not self.menuLayer then
		local puwidth = conf.width - 100
		local puheight = conf.height - 110
		--create layer for menu buttons
		local layer = Popup.new({
			easing = conf.easing, 
			radius = 0,
			width = puwidth,
			height = puheight,
			y = conf.height/2 + 30,
			duration = 0.5
		})
		layer:setFillStyle(Shape.SOLID, 0x000000, 0.6)
		layer:setLineStyle(2, 0x8b9d9b)
		layer:setPosition(conf.width + conf.dx + puwidth/2 + 50, conf.height/2 + 30)
	
		local text = TextField.new(conf.largeFont, "Level paused!")
		text:setPosition(-text:getWidth()/2, -200)
		layer:addChild(text)
	
		local text = TextField.new(conf.largeFont, "Resume Level")
		text:setTextColor(0xffffff)
		local button = Button.new(text, text)
		button:addEventListener("click", function()
			self.menuLayer:close()
		end)
		button:setPosition(-100 - button:getWidth(), 100)
		self:addObject(button)
		layer:addChild(button)
	
		local text = TextField.new(conf.largeFont, "To Main Menu")
		text:setTextColor(0xffffff)
		local button = Button.new(text, text)
		button:addEventListener("click", function()
			sceneManager:changeScene("start", 0.1, conf.transition, conf.easing)
		end)
		button:setPosition(100, 100)
		self:addObject(button)
		layer:addChild(button)
		
		layer:setScale(0)
		self.menuLayer = layer
		self.menuLayer.level = self
		
		function self.menuLayer:close()
			self:hide()
			self.level:unpauseGame()
			self.opened = false
			self:removeFromParent()
		end
		
		function self.menuLayer:open()
			self.level:pauseGame()
			self.level:addChild(self)
			self:show()
			self.opened = true
		end
		
		function self.menuLayer:isOpened()
			return self.opened
		end
	end
	if not self.menuLayer:isOpened() then
		self.menuLayer:open()
	end
end

function GameScene:onLeftJoystick(e)
	if self.players[e.playerId] then
		if e.x == 0 and e.y == 0 then
			self.players[e.playerId]:stopMovement()
		elseif not self.isPaused and math.finite(e.angle) and math.finite(e.strength) then
			self.players[e.playerId]:setMovement(e.angle, e.strength)
		end
	end
end

function GameScene:onRightJoystick(e)
	if self.players[e.playerId] then
		if e.x == 0 and e.y == 0 then
		elseif not self.isPaused and math.finite(e.angle) and math.finite(e.strength) then
			self.players[e.playerId]:setAngle(e.angle)
		end
	end
end

function GameScene:onEnterFrame(e)
	if not self.isPaused then
		self.world:step(1/60, 8, 3)
		--iterate through all child sprites
		local sprite, body
		for i = 1, self.bodies:getNumChildren() do
			--get specific body
			sprite = self.bodies:getChildAt(i)
			body = sprite.body
			--update object's position to match box2d world object's position
			--apply coordinates to sprite
			sprite:setPosition(body:getPosition())
			--apply rotation to sprite
			sprite:setRotation(math.deg(body:getAngle()))
		end
		for i = 1, #self.players do
			self.players[i]:step()
		end
	end
end

function GameScene:onExitBegin()
	self:removeEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
end

function GameScene:back()
	sceneManager:changeScene("select", 0.1, conf.transition, conf.easing)
end