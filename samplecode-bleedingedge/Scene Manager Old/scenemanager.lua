SceneManager = gideros.class(Sprite)

function SceneManager:init(scenes)
	self.scenes = scenes
	self.currentScene = 1

	self.scene1 = self.scenes[self.currentScene].new()
	self:addChild(self.scene1)
		
	self.tweening = false
	
	self:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
end

function SceneManager:nextScene()
	if self.tweening then
		return
	end
	
	if self.currentScene == #self.scenes then
		return
	end
	
	self.currentScene = self.currentScene + 1
	self.scene2 = self.scenes[self.currentScene].new()
	self:addChild(self.scene2)
		
	self.t = 0
	self.tweening = true
	self.direction = 1
end

function SceneManager:previousScene()
	if self.tweening then
		return
	end

	if self.currentScene == 1 then
		return
	end

	self.currentScene = self.currentScene - 1
	self.scene2 = self.scenes[self.currentScene].new()
	self:addChild(self.scene2)
		
	self.t = 0
	self.tweening = true
	self.direction = -1
end

function SceneManager:onEnterFrame()
	if not self.tweening then
		return
	end
	
	self.t = self.t + 0.03
	
	if self.t > 1 then
		self.t = 1
	end

	local t = self.t
	local t = 1 - (1 - t) * (1 - t)

	if self.direction == 1 then
		self.scene1:setAlpha(1 - t)
		self.scene1:setX(-t * 320)
		self.scene2:setX((1 - t) * 320)
	else
		self.scene1:setAlpha(1 - t)
		self.scene1:setX(t * 320)
		self.scene2:setX((t - 1) * 320)
	end
	
	if self.t == 1 then
		self:removeChild(self.scene1)
		self.scene1 = self.scene2
		self.scene2 = nil
		self.tweening = false
	end
end
