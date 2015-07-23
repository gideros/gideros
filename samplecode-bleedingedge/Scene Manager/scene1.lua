Scene1 = gideros.class(Sprite)

function Scene1:init()
	self:addChild(Bitmap.new(Texture.new("gfx/scene1.jpg")))
	
	self:addEventListener("enterBegin", self.onTransitionInBegin, self)
	self:addEventListener("enterEnd", self.onTransitionInEnd, self)
	self:addEventListener("exitBegin", self.onTransitionOutBegin, self)
	self:addEventListener("exitEnd", self.onTransitionOutEnd, self)
end

function Scene1:onTransitionInBegin()
	print("scene1 - enter begin")
end

function Scene1:onTransitionInEnd()
	print("scene1 - enter end")
end

function Scene1:onTransitionOutBegin()
	print("scene1 - exit begin")
end

function Scene1:onTransitionOutEnd()
	print("scene1 - exit end")
end
